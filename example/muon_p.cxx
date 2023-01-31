#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>

// There are 3 types of exposed funcs:
// 1. Preprocessing: this is called before any analysis funcs. It is used to
//   define common variables that are used in multiple analysis funcs.
//   The func signature always be void pre(ROOT::RDF::RNode &)
// 2. Analysis each of them is called once for each analysis entry in the json
//   func name should be given in the json file.
// 3. Cut funcs, used during plots.

// This is an example of a analysis func. It grab all muon, select events with
// at least 1 muon and calculate momentum and energy for all muons and the
// leading muon.
ROOT::RDF::RNode analysis(ROOT::RDF::RNode df) {
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

// This is a placeholder cut, it select events with interType == 1
// You can implement a more complexed cut:
ROOT::RDF::RNode dummy_cut(ROOT::RDF::RNode df) {
  return df.Filter("interType == 1");
}
