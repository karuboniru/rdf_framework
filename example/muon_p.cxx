#include "common.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>

class analysis : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df
        .Define("muon_p4",
                [](int Npars, ROOT::RVec<int> &pdg, ROOT::RVec<double> &px,
                   ROOT::RVec<double> &py, ROOT::RVec<double> &pz,
                   ROOT::RVec<double> &energy) {
                  ROOT::RVec<TLorentzVector> muon_p4;
                  for (int i = 0; i < Npars; i++) {
                    if (pdg[i] == 13) {
                      muon_p4.emplace_back(px[i], py[i], pz[i], energy[i]);
                    }
                  }
                  return muon_p4;
                },
                {"Npars", "pdg", "px", "py", "pz", "energy"})
        .Filter([](ROOT::RVec<TLorentzVector> &p4) { return p4.size() != 0; },
                {"muon_p4"})
        .Define("muon_p",
                [](ROOT::RVec<TLorentzVector> &p4) {
                  ROOT::RVec<double> muon_p;
                  for (auto &p : p4) {
                    muon_p.emplace_back(p.P());
                  }
                  return muon_p;
                },
                {"muon_p4"})
        .Define("leading_muon_p",
                [](ROOT::RVec<double> &muon_p) {
                  return *std::max_element(muon_p.begin(), muon_p.end());
                },
                {"muon_p"})
        .Define("muon_e",
                [](ROOT::RVec<TLorentzVector> &p4) {
                  ROOT::RVec<double> muon_e;
                  for (auto &p : p4) {
                    muon_e.emplace_back(p.E());
                  }
                  return muon_e;
                },
                {"muon_p4"})
        .Define("leading_muon_e",
                [](ROOT::RVec<double> &muon_e) {
                  return *std::max_element(muon_e.begin(), muon_e.end());
                },
                {"muon_e"});
  }
};

REGISTER_PROCESS_NODE(analysis)


class dummy_cut : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Filter("interType == 1");
  }
};

REGISTER_PROCESS_NODE(dummy_cut)