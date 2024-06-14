#pragma once
#include "TH1.h"
#include "TH2.h"
#include <ROOT/RDataFrame.hxx>
#include <nlohmann/json.hpp>

ROOT::RDF::RResultPtr<TH1D>
draw_hists(ROOT::RDF::RNode node, std::string_view var, std::string_view name,
           double xmin, double xmax, int nbins, nlohmann::json &cut,
           std::string_view wname = "");

ROOT::RDF::RResultPtr<TH2D>
draw_hists_2d(ROOT::RDF::RNode node, std::string_view varx,
              std::string_view vary, std::string_view name, double xmin,
              double xmax, double ymin, double ymax, int nbinsx, int nbinsy,
              nlohmann::json &cut, std::string_view wname = "");

ROOT::RDF::RNode do_cut(ROOT::RDF::RNode node, nlohmann::json &cut);