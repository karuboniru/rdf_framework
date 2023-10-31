#pragma once
#include "TH1.h"
#include "TH2.h"
#include <nlohmann/json.hpp>
#include <ROOT/RDataFrame.hxx>
#include <string>

TH1 *draw_hists(ROOT::RDF::RNode node, std::string_view var, std::string_view name,
                double xmin, double xmax, int nbins, double norm,
                nlohmann::json &cut, std::string_view wname = "");

TH1 *draw_hists_2d(ROOT::RDF::RNode node, std::string_view varx, std::string_view vary,
                   std::string_view name, double xmin, double xmax, double ymin,
                   double ymax, int nbinsx, int nbinsy, double norm,
                   nlohmann::json &cut, std::string_view wname = "");