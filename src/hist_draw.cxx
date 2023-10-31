#include "hist_draw.h"
#include "get_callable.h"

TH1 *draw_hists(ROOT::RDF::RNode node, std::string_view var,
                std::string_view name, double xmin, double xmax, int nbins,
                double norm, const nlohmann::json &cut,
                std::string_view wname) {
  auto cut_func =
      cut.empty() ? std::make_unique<noop>() : get_node_process_callable(cut);
  node = (*cut_func)(node);
  auto hist = wname.empty()
                  ? (TH1D *)node
                        .Histo1D(ROOT::RDF::TH1DModel{name.data(), var.data(),
                                                      nbins, xmin, xmax},
                                 var)
                        .GetPtr()
                        ->Clone()
                  : (TH1D *)node
                        .Histo1D(ROOT::RDF::TH1DModel{name.data(), var.data(),
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

TH1 *draw_hists_2d(ROOT::RDF::RNode node, std::string_view varx,
                   std::string_view vary, std::string_view name, double xmin,
                   double xmax, double ymin, double ymax, int nbinsx,
                   int nbinsy, double norm, const nlohmann::json &cut,
                   std::string_view wname) {
  auto cut_func =
      cut.empty() ? std::make_unique<noop>() : get_node_process_callable(cut);
  node = (*cut_func)(node);
  auto hist =
      wname.empty()
          ? (TH2D *)node
                .Histo2D(ROOT::RDF::TH2DModel{name.data(), varx.data(), nbinsx,
                                              xmin, xmax, nbinsy, ymin, ymax},
                         varx, vary)
                .GetPtr()
                ->Clone()
          : (TH2D *)node
                .Histo2D(ROOT::RDF::TH2DModel{name.data(), varx.data(), nbinsx,
                                              xmin, xmax, nbinsy, ymin, ymax},
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