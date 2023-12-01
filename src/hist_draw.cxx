#include "hist_draw.h"
#include "get_callable.h"
#include <string>
#include <string_view>

ROOT::RDF::RNode do_cut(ROOT::RDF::RNode node, nlohmann::json &cut) {
  auto cut_func =
      cut.empty() ? std::make_unique<noop>() : get_node_process_callable(cut);
  if (cut_func)
    return (*cut_func)(node);
  else {
    auto cutstring = cut.get<std::string>();
    std::cout << "Failed to parse cut: " << cutstring << " as process node"
              << " passing it as string cut instead" << std::endl;
    return node.Filter(cut.get<std::string>());
  }
}

ROOT::RDF::RResultPtr<TH1D>
draw_hists(ROOT::RDF::RNode noderaw, std::string_view var,
           std::string_view name, double xmin, double xmax, int nbins,
           nlohmann::json &cut, std::string_view wname) {
  auto node = do_cut(noderaw, cut);
  auto hist = wname.empty()
                  ? node.Histo1D(ROOT::RDF::TH1DModel{name.data(), var.data(),
                                                      nbins, xmin, xmax},
                                 var)
                  : node.Histo1D(ROOT::RDF::TH1DModel{name.data(), var.data(),
                                                      nbins, xmin, xmax},
                                 var, wname);
  return hist;
}

ROOT::RDF::RResultPtr<TH2D>
draw_hists_2d(ROOT::RDF::RNode rawnode, std::string_view varx,
              std::string_view vary, std::string_view name, double xmin,
              double xmax, double ymin, double ymax, int nbinsx, int nbinsy,
              nlohmann::json &cut, std::string_view wname) {
  auto node = do_cut(rawnode, cut);
  auto hist =
      wname.empty()
          ? node.Histo2D(ROOT::RDF::TH2DModel{name.data(), varx.data(), nbinsx,
                                              xmin, xmax, nbinsy, ymin, ymax},
                         varx, vary)

          : node.Histo2D(ROOT::RDF::TH2DModel{name.data(), varx.data(), nbinsx,
                                              xmin, xmax, nbinsy, ymin, ymax},
                         varx, vary, wname);
  return hist;
}