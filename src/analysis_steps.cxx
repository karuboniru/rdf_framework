#include "analysis_steps.h"
#include "THStack.h"
#include "dlopen_wrap.h"
#include "get_callable.h"
#include "hist_draw.h"
#include "wildcard_expand.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <TChain.h>
#include <dlfcn.h>
#include <memory>
#include <tuple>

std::tuple<std::unique_ptr<TChain>, std::vector<std::unique_ptr<TChain>>>
prepare_chain(nlohmann::json &j) {
  std::tuple<std::unique_ptr<TChain>, std::vector<std::unique_ptr<TChain>>> ret{};
  std::vector<std::string> file_paths{};
  for (auto &file : j["files"]) {
    auto expanded_paths = expand_wildcard_path(file.get<std::string>());
    file_paths.insert(file_paths.end(), expanded_paths.begin(),
                      expanded_paths.end());
  }
  auto &&[filechain, friend_chains] = ret;
  filechain = std::make_unique<TChain>();
  for (auto &file : file_paths) {
    filechain->AddFile(
        (file + "?#" + j["treename"].get<std::string>()).c_str());
  }
  for (auto &friend_tree : j["friend_trees"]) {
    auto &chain = friend_chains.emplace_back(std::make_unique<TChain>());
    for (auto &file : file_paths) {
      chain->AddFile(
          (file + "?#" + friend_tree["treename"].get<std::string>()).c_str());
    }
    filechain->AddFriend(chain.get(), friend_tree.value("alias", "").c_str());
  }
  return ret;
}

void plugin_handle(ROOT::RDF::RNode rootnode, nlohmann::json &entry) {
  std::cout << "Processing " << entry["name"] << std::endl;

  std::string so_name = entry["plugin"];
  dlopen_wrap so(so_name);

  std::unique_ptr<ProcessNodeI> preprocess =
      entry.find("preprocess") != entry.end()
          ? get_node_process_callable(entry["preprocess"])
          : std::make_unique<noop>();

  auto preprocessed_node = (*preprocess)(rootnode);

  double norm_factor{};
  if (entry.find("normalize") != entry.end()) {
    auto normalize_conf = entry["normalize"];
    auto normalize_func = get_normalize_callable(normalize_conf);
    norm_factor = (*normalize_func)(rootnode);
    std::cout << "Normalization: " << norm_factor << std::endl;
  }

  for (auto &analysis_entry : entry["analysis"]) {
    analysis_entry_handle(preprocessed_node, analysis_entry, norm_factor);
  }
}

void analysis_entry_handle(ROOT::RDF::RNode preprocessed_node,
                           nlohmann::json &analysis_entry, double norm_factor) {

  auto plots_file = std::make_unique<TFile>(
      analysis_entry["plot_file"].get<std::string>().c_str(), "RECREATE");
  // std::vector<TObject *> objs_list{};
  auto func = get_node_process_callable(analysis_entry["func"]);
  auto result_node = (*func)(preprocessed_node);

  {
    auto &&collist = analysis_entry["output"].get<std::vector<std::string>>();
    if (!collist.empty())
      result_node.Snapshot(analysis_entry["treename"].get<std::string>(),
                           analysis_entry["filename"].get<std::string>(),
                           collist);
  }
  for (auto &plotentry : analysis_entry["plots"]) {
    std::string var = plotentry["var"];
    std::string name = plotentry["name"];
    double xmin = plotentry.value("xmin", 0.0);
    double xmax = plotentry.value("xmax", 0.0);
    int nbins = plotentry.value("nbins", 128);
    auto cut = plotentry.value("cut", nlohmann::json{});
    std::string wname = plotentry.value("wname", "");
    draw_hists(result_node, var, name, xmin, xmax, nbins, norm_factor, cut,
               wname)
        ->SetDirectory(plots_file.get());
  }
  plots_file->cd();
  for (auto &plotentry : analysis_entry["stack_plots"]) {
    std::string var = plotentry["var"];
    std::string name = plotentry["name"];
    double xmin = plotentry.value("xmin", 0);
    double xmax = plotentry.value("xmax", 0);
    int nbins = plotentry.value("nbins", 128);
    std::string wname = plotentry.value("wname", "");
    // std::string cut = plotentry.value("cut", "");
    THStack *stack = new THStack((name + "hs").c_str(), var.c_str());
    for (auto &cutentry : plotentry["cuts"]) {
      // objs_list.emplace_back();
      auto hist = draw_hists(result_node, var, name, xmin, xmax, nbins,
                             norm_factor, cutentry, wname);
      stack->Add(hist);
    }
    stack->Write();
  }
  for (auto &plot_2d_entry : analysis_entry["plot_2d"]) {
    std::string varx = plot_2d_entry["varx"];
    std::string vary = plot_2d_entry["vary"];
    std::string name = plot_2d_entry["name"];
    double xmin = plot_2d_entry.value("xmin", 0.0);
    double xmax = plot_2d_entry.value("xmax", 0.0);
    double ymin = plot_2d_entry.value("ymin", 0.0);
    double ymax = plot_2d_entry.value("ymax", 0.0);
    int nbinsx = plot_2d_entry.value("nbinsx", 128);
    int nbinsy = plot_2d_entry.value("nbinsy", 128);
    auto cut = plot_2d_entry.value("cut", nlohmann::json{});
    std::string wname = plot_2d_entry.value("wname", "");
    draw_hists_2d(result_node, varx, vary, name, xmin, xmax, ymin, ymax, nbinsx,
                  nbinsy, norm_factor, cut, wname)
        ->SetDirectory(plots_file.get());
  }
  plots_file->Write();
  plots_file->Close();
}