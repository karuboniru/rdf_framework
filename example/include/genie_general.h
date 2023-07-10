#pragma once

#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>

ROOT::RDF::RNode CC_selection(ROOT::RDF::RNode df);
std::pair<double, double> get_xsec(TH1 *h_rate, TGraph *spline);

double normalize_factor_CC(ROOT::RDF::RNode df,
                           std::vector<std::string> parameters);