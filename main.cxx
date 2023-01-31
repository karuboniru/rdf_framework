#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TDirectory.h>
#include <THStack.h>
#include <TROOT.h>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
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
                std::string cut = "") {
  if (!cut.empty()) {
    analysis_func_t func =
        (analysis_func_t)(dlsym(handle, funcname_mangle(cut).c_str()));
    if (func) {
      node = func(node);
    } else {
      node = node.Filter(cut);
    }
  }
  auto hist = (TH1D *)node
                  .Histo1D(ROOT::RDF::TH1DModel{name.c_str(), var.c_str(),
                                                nbins, xmin, xmax},
                           var)
                  .GetPtr()
                  ->Clone();
  auto hist_int = hist->Integral();
  if (norm && hist_int) { // if we have a normalization factor and hist
                          // is not empty
    hist->Scale(norm, "WIDTH");
  }
  return (TH1 *)hist;
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

  ROOT::RDataFrame df(j["treename"].get<std::string>(), j["files"]);
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
      norm_factor = func(preprocessed_node, normalize_conf["parameters"]);
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
        double xmin = plotentry.value("xmin", 0);
        double xmax = plotentry.value("xmax", 0);
        int nbins = plotentry.value("nbins", 128);
        std::string cut = plotentry.value("cut", "");
        draw_hists(result_node, var, name, xmin, xmax, nbins, handle,
                   norm_factor, cut)
            ->SetDirectory(plots_file.get());
      }
      plots_file->cd();
      for (auto &plotentry : analysis_entry["stack_plots"]) {
        std::string var = plotentry["var"];
        std::string name = plotentry["name"];
        double xmin = plotentry.value("xmin", 0);
        double xmax = plotentry.value("xmax", 0);
        int nbins = plotentry.value("nbins", 128);
        std::string cut = plotentry.value("cut", "");
        THStack *stack = new THStack((name + "hs").c_str(), var.c_str());
        for (auto &cutentry : plotentry["cuts"]) {
          // objs_list.emplace_back();
          auto hist = draw_hists(result_node, var, name, xmin, xmax, nbins,
                                 handle, norm_factor, cutentry);
          stack->Add(hist);
          // delete hist;
          // objs_list.emplace_back(stack);
        }
        stack->Write();
      }
      // for (auto &obj : objs_list) {
      //   // obj->SetDirectory(plots_file.get());
      //   // plots_file->Append(obj);
      //   obj->Write();
      // }
      plots_file->Write();
      plots_file->Close();
    }
    dlclose(handle);
  }

  return 0;
}