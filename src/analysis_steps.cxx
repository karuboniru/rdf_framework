#include "analysis_steps.h"
#include "ROOT/RDF/RResultMap.hxx"
#include "THStack.h"
#include "dlopen_wrap.h"
#include "get_callable.h"
#include "hist_draw.h"
#include "wildcard_expand.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TChain.h>
#include <dlfcn.h>
#include <memory>
// #include <optional>
#include <ranges>
#include <tuple>
#include <variant>
// #include <unordered_map>

std::tuple<std::unique_ptr<TChain>, std::vector<std::unique_ptr<TChain>>>
prepare_chain(nlohmann::json &j) {
  std::tuple<std::unique_ptr<TChain>, std::vector<std::unique_ptr<TChain>>>
      ret{};
  std::vector<std::string> file_paths{};
  // Process each file entry in the JSON configuration
  for (auto &file : j["files"]) {
    // Check if the file entry starts with '@' indicating a file list
    if (file.get<std::string>()[0] == '@') {
      // Open the file list (removing the '@' prefix)
      std::ifstream file_list(file.get<std::string>().substr(1));
      std::string line;
      while (std::getline(file_list, line)) {
        file_paths.emplace_back(line);
      }
    } else {
      // Handle wildcard paths by expanding them to actual file paths
      auto expanded_paths =
          ROOT::Internal::TreeUtils::ExpandGlob(file.get<std::string>());
      file_paths.insert(file_paths.end(), expanded_paths.begin(),
                        expanded_paths.end());
    }
  }
  auto &&[filechain, friend_chains] = ret;
  filechain = std::make_unique<TChain>();
  for (auto &file : file_paths) {
    filechain->AddFile(
        (file + "?#" + j["treename"].get<std::string>()).c_str());
  }

  for (auto &friend_tree : j["friend_trees"]) {
    std::vector<std::string> file_paths_friend{};
    if (friend_tree.find("files_friend") != friend_tree.end()) {
      for (auto &file : friend_tree["files_friend"]) {
        if (file.get<std::string>()[0] == '@') {
          std::ifstream file_list(file.get<std::string>().substr(1));
          std::string line;
          while (std::getline(file_list, line)) {
            file_paths_friend.emplace_back(line);
          }
        } else {
          auto expanded_paths =
              ROOT::Internal::TreeUtils::ExpandGlob(file.get<std::string>());
          file_paths_friend.insert(file_paths_friend.end(),
                                   expanded_paths.begin(),
                                   expanded_paths.end());
        }
      }
      if (file_paths_friend.size() != file_paths.size()) {
        throw std::runtime_error("Number of files in 'files_friend' must match "
                                 "'files', quitting for safety.");
      }
    } else {
      std::cerr
          << "No 'files_friend' specified, using 'files' for friend trees."
          << std::endl;
    }
    auto &chain = friend_chains.emplace_back(std::make_unique<TChain>());
    for (auto &file :
         file_paths_friend.empty() ? file_paths : file_paths_friend) {
      chain->AddFile(
          (file + "?#" + friend_tree["treename"].get<std::string>()).c_str());
    }
    filechain->AddFriend(
        chain.get(),
        friend_tree.value("alias", friend_tree["treename"].get<std::string>())
            .c_str());
  }
  return ret;
}

std::vector<axis> get_bins(nlohmann::json &j) {
  std::vector<axis> ret{};
  auto construct_and_emplace_bin_obj = [&ret](nlohmann::json &bin_obj) {
    if (bin_obj.contains("bin_edges")) {
      auto bin_edges = bin_obj["bin_edges"].get<std::vector<double>>();
      ret.emplace_back(
          axis_non_uniform{.var = bin_obj["var"], .bin_edges = bin_edges});
    } else {
      ret.emplace_back(axis_uniform{.var = bin_obj["var"],
                                    .min = bin_obj["min"],
                                    .max = bin_obj["max"],
                                    .bins = bin_obj["bins"]});
    }
  };
  if (j.find("bins_list") != j.end()) {
    for (auto &&bin_obj : j["bins_list"]) {
      construct_and_emplace_bin_obj(bin_obj);
    }
    return ret;
  }
  if (j.find("var") != j.end() && (j.find("bins") != j.end() ||
                                   j.find("bin_edges") != j.end())) { // 1D case
    construct_and_emplace_bin_obj(j);
    return ret;
  }
  // 2-3D case
  ret.emplace_back(axis_uniform{.var = j["varx"],
                                .min = j["xmin"],
                                .max = j["xmax"],
                                .bins = j["binsx"]});
  if (j.find("vary") != j.end()) {
    ret.emplace_back(axis_uniform{.var = j["vary"],
                                  .min = j["ymin"],
                                  .max = j["ymax"],
                                  .bins = j["binsy"]});
  }
  if (j.find("varz") != j.end()) {
    ret.emplace_back(axis_uniform{.var = j["varz"],
                                  .min = j["zmin"],
                                  .max = j["zmax"],
                                  .bins = j["binsz"]});
  }
  return ret;
}

auto analysis_entry_handle(ROOT::RDF::RNode preprocessed_node,
                           nlohmann::json &analysis_entry) {
  auto func = get_node_process_callable(analysis_entry["func"]);
  if (!func)
    throw std::runtime_error("Failed to get analysis function");
  auto result_node = (*func)(preprocessed_node);
  std::tuple<
      std::string, std::vector<general_hist_result_t>,
      std::vector<std::pair<THStack, std::vector<ROOT::RDF::RResultPtr<TH1D>>>>,
      // std::vector<ROOT::RDF::RResultPtr<TH2D>>,
      ROOT::RDF::RResultPtr<
          ROOT::RDF::RInterface<ROOT::Detail::RDF::RLoopManager>>>
      ret{};
  auto &&[plot_file, hist1ds, plots, snapshot_action] = ret;
  plot_file = analysis_entry["plot_file"].get<std::string>();

  auto cutset = analysis_entry.value("cutset", nlohmann::json{});
  std::vector<std::tuple<std::string, ROOT::RDF::RNode>> cut_nodes{};
  for (auto &&cuts : cutset) {
    std::vector<std::tuple<std::string, ROOT::RDF::RNode>>
        cut_nodes_next_layer{};
    for (auto &&cut : cuts) {
      std::cout << "cut: " << cut["cut"] << std::endl;

      cut_nodes_next_layer.emplace_back(cut["name"],
                                        do_cut(result_node, cut["cut"]));
      for (auto &&[name, node] : cut_nodes) {
        cut_nodes_next_layer.emplace_back(name + "_" +
                                              cut["name"].get<std::string>(),
                                          do_cut(node, cut["cut"]));
      }
    }
    cut_nodes.insert(cut_nodes.end(), cut_nodes_next_layer.begin(),
                     cut_nodes_next_layer.end());
  }

  // plot 1D hists
  for (auto &plotentry : analysis_entry["plots"]) {
    std::string name = plotentry["name"];
    auto bin_obj = get_bins(plotentry);
    auto cut = plotentry.value("cut", nlohmann::json{});
    std::string wname = plotentry.value("wname", "");
    hist1ds.emplace_back(draw_hists_nd(result_node, name, bin_obj, cut, wname));
    for (auto &[cutname, cutnode] : cut_nodes) {
      hist1ds.emplace_back(
          draw_hists_nd(cutnode, name + "_" + cutname, bin_obj, cut, wname));
    }
  }

  // Plot Hist2D
  for (auto &plotentry : analysis_entry["plot_2d"]) {
    std::string name = plotentry["name"];
    auto bin_obj = get_bins(plotentry);
    auto cut = plotentry.value("cut", nlohmann::json{});
    std::string wname = plotentry.value("wname", "");
    hist1ds.emplace_back(draw_hists_nd(result_node, name, bin_obj, cut, wname));
    for (auto &[cutname, cutnode] : cut_nodes) {
      hist1ds.emplace_back(
          draw_hists_nd(cutnode, name + "_" + cutname, bin_obj, cut, wname));
    }
  }

  // plot Stacks
  for (auto &plotentry : analysis_entry["stack_plots"]) {
    std::string var = plotentry["var"];
    std::string name = plotentry["name"];
    std::string wname = plotentry.value("wname", "");
    auto &&[hs, histvec] = plots.emplace_back(
        std::make_pair(THStack{(name + "hs").c_str(), var.c_str()},
                       std::vector<ROOT::RDF::RResultPtr<TH1D>>{}));
    // axis x{axis_uniform{var, xmin, xmax, nbins}};
    auto bin_obj = get_bins(plotentry);
    for (auto &cutentry : plotentry["cuts"]) {
      histvec.emplace_back(
          draw_hists(result_node, name, get_axis(bin_obj[0]), cutentry, wname));
    }
  }

  if (analysis_entry.contains("output")) {
    auto &&collist = analysis_entry["output"].get<std::vector<std::string>>();
    if (!collist.empty()) {
      ROOT::RDF::RSnapshotOptions opts;
      opts.fLazy = true;
      opts.fOverwriteIfExists = true;
      snapshot_action = result_node.Snapshot(
          analysis_entry["treename"].get<std::string>(),
          analysis_entry["filename"].get<std::string>(), collist, opts);
    }
  }
  return ret;
}

ROOT::RDF::RNode add_variation(ROOT::RDF::RNode df_in,
                               nlohmann::json &configuration) {
  return df_in.Define("__weight", []() { return 1.; })
      .Vary(
          "__weight",
          [](const TArrayD &weights) {
            ROOT::RVecD ret{};
            for (int i = 0; i < weights.GetSize(); ++i) {
              ret.push_back(weights[i]);
            }
            return ret;
          },
          {configuration["weight"]}, configuration["count"].get<int>());
}

void plugin_handle(TChain &ch, nlohmann::json &entry) {
  std::string so_name = entry["plugin"];
  auto skip_empty = entry.value("skip_empty", true);
  dlopen_wrap so(so_name);
  // auto rootnode = ROOT::RDF::AsRNode(ROOT::RDataFrame(ch));
  ROOT::RDataFrame rootnode(ch);
  std::cout << "Processing " << entry["name"] << std::endl;
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 30, 0)
  ROOT::RDF::Experimental::AddProgressBar(rootnode);
#endif

  std::unique_ptr<ProcessNodeI> preprocess =
      entry.find("preprocess") != entry.end()
          ? get_node_process_callable(entry["preprocess"])
          : std::make_unique<noop>();
  if (!preprocess)
    throw std::runtime_error("Failed to get preprocess function");

  auto variation_iter = entry.find("variation");
  auto with_variation = variation_iter != entry.end();

  auto preprocessed_node = (*preprocess)(
      with_variation ? add_variation(rootnode, *variation_iter) : rootnode);

  if (with_variation) {
    // modify plot configuration that the weight is applied
    for (auto &analysis_obj : entry["analysis"]) {
      for (auto &plot_obj : analysis_obj["plots"]) {
        plot_obj["wname"] = "__weight";
      }
    }
  }

  // double norm_factor{};
  std::unique_ptr<NormalizeI> normalize_func{};
  if (entry.find("normalize") != entry.end()) {
    auto normalize_conf = entry["normalize"];
    normalize_func = get_normalize_callable(normalize_conf);
    if (!normalize_func)
      throw std::runtime_error("Failed to get normalize function");
  }

  std::vector<std::tuple<
      std::string, std::vector<general_hist_result_t>,
      std::vector<std::pair<THStack, std::vector<ROOT::RDF::RResultPtr<TH1D>>>>,
      // std::vector<ROOT::RDF::RResultPtr<TH2D>>,
      ROOT::RDF::RResultPtr<
          ROOT::RDF::RInterface<ROOT::Detail::RDF::RLoopManager>>>>
      analysis_result_handles{};
  for (auto &analysis_entry : entry["analysis"]) {
    analysis_result_handles.emplace_back(
        analysis_entry_handle(preprocessed_node, analysis_entry));
  }

  // event loop should be triggered here if normalize_func is defined
  if (!with_variation) {
    auto normalize_factor =
        normalize_func ? (*normalize_func)(preprocessed_node) : 1.;
    if (normalize_factor != 1.) {
      for (auto &&[_, hist1ds, stacks, __] : analysis_result_handles) {
        for (auto &&hist : hist1ds) {
          std::visit(
              [&](auto &&hist_) { hist_->Scale(normalize_factor, "WIDTH"); },
              hist);
        }
        for (auto &&[_, hists] : stacks) {
          for (auto &&hist : hists) {
            hist->Scale(normalize_factor, "WIDTH");
          }
        }
      }
    }
  }

  if (!with_variation) {
    std::cerr << "Normal plot mode" << std::endl;
    for (auto &&[filename, hist1ds, stacks, snapshot_action] :
         analysis_result_handles) {
      snapshot_action.GetPtr();
      if (!hist1ds.empty() || !stacks.empty()) {
        std::cerr << "Writing to " << filename << std::endl;
        auto file = std::make_unique<TFile>(filename.c_str(), "RECREATE");
        file->cd();
        for (auto &&hist : hist1ds) {
          if (skip_empty &&
              std::visit([&](auto &&hist_) { return hist_->GetEntries(); },
                         hist) == 0)
            continue;
          std::visit([&](auto &&hist_) { hist_->Write(); }, hist);
        }
        for (auto &&[hs, hists] : stacks) {
          for (auto &&hist : hists) {
            if (skip_empty && hist->GetEntries() == 0)
              continue;
            hs.Add(hist.GetPtr());
          }
          hs.Write();
        }
        file->Write();
        file->Close();
      }
    }
  } else {
    std::cerr << "Variations plot mode" << std::endl;
    using general_var_result_t =
        std::variant<ROOT::RDF::Experimental::RResultMap<TH1D>,
                     ROOT::RDF::Experimental::RResultMap<TH2D>,
                     ROOT::RDF::Experimental::RResultMap<TH3D>>;

    for (auto &&[filename, hist1ds, variations] :
         analysis_result_handles | std::views::transform([](auto &&tup) {
           auto &&[filename, hist1ds, stacks, snapshot_action] = tup;
           auto range_vars =
               hist1ds | std::views::transform([](auto &&hist) {
                 return std::visit(
                     [](auto &&hist_) -> general_var_result_t {
                       return ROOT::RDF::Experimental::VariationsFor(hist_);
                     },
                     hist);
               });
           std::vector<general_var_result_t> variations{};
           for (auto &&var : range_vars) {
             variations.emplace_back(var);
           }
           return std::make_tuple(filename, hist1ds, variations);
         })) {
      auto file = std::make_unique<TFile>(filename.c_str(), "RECREATE");
      file->cd();
      // Oh I want std::views::zip
      for (size_t index{}; index < hist1ds.size(); index++) {
        auto &hist = hist1ds[index];
        // auto name = hist->GetName();
        auto name =
            std::visit([](auto &&hist_) { return hist_->GetName(); }, hist);
        auto dir = file->mkdir(Form("d_%s", name));
        dir->cd();
        auto &variation = variations[index];
        std::visit(
            [&](auto variation_) {
              for (auto &plots : variation_ | std::views::values) {
                plots->Divide(std::visit(
                    [](auto &&hist_) -> TH1 * { return hist_.GetPtr(); },
                    hist));
                plots->Write();
              }
            },
            variation);
        dir->Write();
        file->cd();
      }
    }
  }
}
