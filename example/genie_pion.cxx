#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TF1.h>
#include <TLorentzVector.h>
#include <TObjString.h>
#include <TSpline.h>
#include <genie_general.h>
#include <common.h>

class pion_kin : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override{
   return df
      .Define("pions",
              [](int StdHepN, ROOT::RVec<double> &StdHepP4,
                 ROOT::RVec<int> &StdHepPdg, ROOT::RVec<int> &StdHepStatus) {
                ROOT::RVec<TLorentzVector> pions;
                for (int i = 0; i < StdHepN; i++) {
                  if (StdHepPdg[i] == 211 &&
                      StdHepStatus[i] == 1) { // select all pi+
                    pions.emplace_back(StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                                       StdHepP4[4 * i + 2],
                                       StdHepP4[4 * i + 3]);
                  }
                }
                return pions;
              },
              {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"})
      .Define("pionE",
              [](const ROOT::RVec<TLorentzVector> &pions) {
                ROOT::RVec<double> pionE;
                for (auto &p : pions) {
                  pionE.emplace_back(p.E() - p.M());
                }
                return pionE;
              },
              {"pions"})
      .Define("p4_neutrino",
              [](ROOT::RVec<double> &StdHepP4) {
                return TLorentzVector{StdHepP4[0], StdHepP4[1], StdHepP4[2],
                                      StdHepP4[3]};
              },
              {"StdHepP4"})
      .Define("piontheta",
              [](const ROOT::RVec<TLorentzVector> &pions,
                 const TLorentzVector &p4_neutrino) {
                ROOT::RVec<double> piontheta;
                for (auto &p : pions) {
                  piontheta.emplace_back(p.Angle(p4_neutrino.Vect()));
                }
                return piontheta;
              },
              {"pions", "p4_neutrino"});
  }
};

REGISTER_PROCESS_NODE(pion_kin);