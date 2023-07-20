#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TDirectory.h>
#include <THStack.h>
#include <TROOT.h>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
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

typedef ROOT::RDF::RNode (*analysis_func_t)(ROOT::RDF::RNode);
typedef double (*normalize_func_t)(ROOT::RDF::RNode, std::vector<std::string>);

ROOT::RDF::RNode no_op(ROOT::RDF::RNode node) { return node; }

// mangle name for
// ROOT::RDF::RNode funcname(ROOT::RDF::RNode)
std::string funcname_mangle(std::string funcname) {
  auto len = funcname.length();
  std::string mangled_name =
      "_Z" + std::to_string(len) + funcname +
      "N4ROOT3RDF10RInterfaceINS_6Detail3RDF9RNodeBaseEvEE";
  return mangled_name;
}

std::string funcname_mangle_with_parameter(std::string funcname) {
  auto len = funcname.length();
  std::string mangled_name =
      "_Z" + std::to_string(len) + funcname +
      "N4ROOT3RDF10RInterfaceINS_6Detail3RDF9RNodeBaseEvEESt6vectorINSt7__"
      "cxx1112basic_stringIcSt11char_traitsIcESaIcEEESaISC_EE";
  return mangled_name;
}

TH1 *draw_hists(ROOT::RDF::RNode node, std::string var, std::string name,
                double xmin, double xmax, int nbins, void *handle, double norm,
                std::string cut = "", std::string wname = "") {
  if (!cut.empty()) {
    analysis_func_t func =
        (analysis_func_t)(dlsym(handle, funcname_mangle(cut).c_str()));
    if (func) {
      node = func(node);
    } else {
      node = node.Filter(cut);
    }
  }
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
  ROOT::RDataFrame df(j["treename"].get<std::string>(), file_paths);
  auto rootnode = ROOT::RDF::AsRNode(df);

  for (auto &entry : j["plugins"]) {
    std::cout << "Processing " << entry["name"] << std::endl;

    std::string so_name = entry["plugin"];
    auto handle = dlopen(so_name.c_str(), RTLD_NOW);
    if (!handle) {
      std::cerr << "Cannot open library: " << dlerror() << std::endl;
      continue;
    }

    analysis_func_t pre = no_op;
    {
      auto funcname = entry.value("preprocess", "");
      if (!funcname.empty())
        pre =
            (analysis_func_t)(dlsym(handle, funcname_mangle(funcname).c_str()));
    }
    auto preprocessed_node = pre(rootnode);

    double norm_factor{};
    if (entry.find("normalize") != entry.end()) {
      auto normalize_conf = entry["normalize"];
      auto func = (normalize_func_t)(dlsym(
          handle,
          funcname_mangle_with_parameter(normalize_conf["func"]).c_str()));
      if (!func) {
        std::cerr << "Cannot load symbol '" << normalize_conf
                  << "': " << dlerror() << std::endl;
        continue;
      }
      norm_factor = func(rootnode, normalize_conf["parameters"]);
      std::cout << "Normalization: " << norm_factor << std::endl;
    }

    for (auto &analysis_entry : entry["analysis"]) {
      auto plots_file = std::make_unique<TFile>(
          analysis_entry["plot_file"].get<std::string>().c_str(), "RECREATE");
      // std::vector<TObject *> objs_list{};
      analysis_func_t func = (analysis_func_t)(dlsym(
          handle,
          funcname_mangle(analysis_entry["func"].get<std::string>()).c_str()));
      if (!func) {
        std::cerr << "Cannot load symbol '" << analysis_entry["func"]
                  << "': " << dlerror() << std::endl;
        continue;
      }
      auto result_node = func(preprocessed_node);
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
        std::string cut = plotentry.value("cut", "");
        std::string wname = plotentry.value("wname", "");
        draw_hists(result_node, var, name, xmin, xmax, nbins, handle,
                   norm_factor, cut, wname)
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
                                 handle, norm_factor, cutentry, wname);
          stack->Add(hist);
        }
        stack->Write();
      }
      plots_file->Write();
      plots_file->Close();
    }
    dlclose(handle);
  }

  return 0;
}
