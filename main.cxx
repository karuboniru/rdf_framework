#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TChain.h>
#include <TDirectory.h>
#include <THStack.h>
#include <TROOT.h>
#include <common.h>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

std::unique_ptr<ProcessNodeI>
get_node_process_callable(const nlohmann::json &conf) {
  std::string name;
  nlohmann::json config{};
  if (conf.is_string()) {
    name = conf.get<std::string>();
  } else {
    name = conf["name"].get<std::string>();
    config = conf.value("config", nlohmann::json());
  }
  auto ptr = ProcessNodeFactory::instance().create(name);
  ptr->configure(config);
  return ptr;
}

std::unique_ptr<NormalizeI> get_normalize_callable(const nlohmann::json &conf) {
  std::string name;
  nlohmann::json config{};
  if (conf.is_string()) {
    name = conf.get<std::string>();
  } else {
    name = conf["name"].get<std::string>();
    config = conf.value("config", nlohmann::json());
  }
  auto ptr = NormalizeFactory::instance().create(name);
  ptr->configure(config);
  return ptr;
}

class noop : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override { return df; }
};

REGISTER_PROCESS_NODE(noop)

TH1 *draw_hists(ROOT::RDF::RNode node, std::string var, std::string name,
                double xmin, double xmax, int nbins, double norm,
                nlohmann::json &cut, std::string wname = "") {
  auto cut_func =
      cut.empty() ? std::make_unique<noop>() : get_node_process_callable(cut);
  node = (*cut_func)(node);
  auto hist = wname.empty()
                  ? (TH1D *)node
                        .Histo1D(ROOT::RDF::TH1DModel{name.c_str(), var.c_str(),
                                                      nbins, xmin, xmax},
                                 var)
                        .GetPtr()
                        ->Clone()
                  : (TH1D *)node
                        .Histo1D(ROOT::RDF::TH1DModel{name.c_str(), var.c_str(),
                                                      nbins, xmin, xmax},
                                 var, wname)
                        .GetPtr()
                        ->Clone();
  auto hist_int = hist->Integral();
  if (norm && hist_int) { // if we have a normalization factor and hist
                          // is not empty
    hist->Scale(norm, "WIDTH");
  }
  return (TH1 *)hist;
}

TH1 *draw_hists_2d(ROOT::RDF::RNode node, std::string varx, std::string vary,
                   std::string name, double xmin, double xmax, double ymin,
                   double ymax, int nbinsx, int nbinsy, double norm,
                   nlohmann::json &cut, std::string wname = "") {
  auto cut_func =
      cut.empty() ? std::make_unique<noop>() : get_node_process_callable(cut);
  node = (*cut_func)(node);
  auto hist =
      wname.empty()
          ? (TH2D *)node
                .Histo2D(ROOT::RDF::TH2DModel{name.c_str(), varx.c_str(),
                                              nbinsx, xmin, xmax, nbinsy, ymin,
                                              ymax},
                         varx, vary)
                .GetPtr()
                ->Clone()
          : (TH2D *)node
                .Histo2D(ROOT::RDF::TH2DModel{name.c_str(), varx.c_str(),
                                              nbinsx, xmin, xmax, nbinsy, ymin,
                                              ymax},
                         varx, vary, wname)
                .GetPtr()
                ->Clone();
  auto hist_int = hist->Integral();
  if (norm && hist_int) { // if we have a normalization factor and hist
                          // is not empty
    hist->Scale(norm, "WIDTH");
  }
  return (TH2 *)hist;
}

std::vector<std::string> expand_wildcard_path(std::string input) {
  std::vector<std::string> ret;
  auto pos_star = input.find("*");
  if (pos_star != std::string::npos) {
    auto slash_before_star = input.rfind("/", pos_star);
    auto slash_after_star = input.find("/", pos_star);
    auto dir = input.substr(0, slash_before_star);
    auto pattern = input.substr(slash_before_star + 1,
                                (slash_after_star - slash_before_star - 1));
    // convert * to .*
    pattern = std::regex_replace(pattern, std::regex("\\*"), ".*");
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
      // std::cout << "matching " << entry.path().filename().string() << " to "
      //           << pattern << std::endl;
      if (std::regex_match(entry.path().filename().string(),
                           std::regex(pattern))) {
        if (slash_after_star != std::string::npos) {
          if (!entry.is_directory()) {
            continue;
          }
          auto following_path = input.substr(slash_after_star + 1);
          auto thispath = entry.path().string() + "/" + following_path;
          if (thispath.find("*") != std::string::npos) {
            auto subpaths = expand_wildcard_path(thispath);
            ret.insert(ret.end(), subpaths.begin(), subpaths.end());
          } else if (std::filesystem::exists(thispath)) {
            ret.push_back(thispath);
          }
        } else {
          ret.push_back(entry.path().string());
        }
      }
    }
  } else {
    ret.push_back(input);
  }
  return ret;
}

int main(int argc, char **argv) {
  ROOT::EnableImplicitMT();
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <json_file>" << std::endl;
    return 1;
  }

  nlohmann::json j;
  {
    std::ifstream json_file(argv[1]);
    json_file >> j;
  }

  std::vector<std::string> file_paths{};
  for (auto &file : j["files"]) {
    auto expanded_paths = expand_wildcard_path(file);
    file_paths.insert(file_paths.end(), expanded_paths.begin(),
                      expanded_paths.end());
  }
  // for (auto &file : file_paths) {
  //   std::cout << "Adding file " << file << std::endl;
  // }
  TChain filechain;
  for (auto &file : file_paths) {
    filechain.AddFile((file + "?#" + j["treename"].get<std::string>()).c_str());
  }
  std::vector<std::unique_ptr<TChain>> friend_chains{};
  for (auto &friend_tree : j["friend_trees"]) {
    auto &chain = friend_chains.emplace_back(std::make_unique<TChain>());
    for (auto &file : file_paths) {
      chain->AddFile(
          (file + "?#" + friend_tree["treename"].get<std::string>()).c_str());
    }
    filechain.AddFriend(chain.get(), friend_tree.value("alias", "").c_str());
  }
  // ROOT::RDataFrame df(j["treename"].get<std::string>(), file_paths);
  ROOT::RDataFrame df(filechain);
  auto rootnode = ROOT::RDF::AsRNode(df);

  for (auto &entry : j["plugins"]) {
    std::cout << "Processing " << entry["name"] << std::endl;

    std::string so_name = entry["plugin"];
    auto handle = dlopen(so_name.c_str(), RTLD_NOW);
    if (!handle) {
      std::cerr << "Cannot open library: " << dlerror() << std::endl;
      continue;
    }

    // analysis_func_t pre = no_op;
    // {
    //   auto funcname = entry.value("preprocess", "");
    //   if (!funcname.empty())
    //     pre =
    //         (analysis_func_t)(dlsym(handle,
    //         funcname_mangle(funcname).c_str()));
    // }
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
      auto plots_file = std::make_unique<TFile>(
          analysis_entry["plot_file"].get<std::string>().c_str(), "RECREATE");
      // std::vector<TObject *> objs_list{};
      auto func = get_node_process_callable(analysis_entry["func"]);
      // if (!func) {
      //   std::cerr << "Cannot load symbol '" << analysis_entry["func"]
      //             << "': " << dlerror() << std::endl;
      //   continue;
      // }
      auto result_node = (*func)(preprocessed_node);
      result_node.Snapshot(
          analysis_entry["treename"].get<std::string>(),
          analysis_entry["filename"].get<std::string>(),
          analysis_entry["output"].get<std::vector<std::string>>());
      // std::cout << "Snapshot done" << std::endl;
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
        draw_hists_2d(result_node, varx, vary, name, xmin, xmax, ymin, ymax,
                      nbinsx, nbinsy, norm_factor, cut, wname)
            ->SetDirectory(plots_file.get());
      }
      plots_file->Write();
      plots_file->Close();
    }
    dlclose(handle);
  }

  return 0;
}
