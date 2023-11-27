#include "common.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <TLorentzVector.h>

class GENIESingleKin : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df
        .Define("particle_p4",
                [=, this](int StdHepN, ROOT::RVec<double> &StdHepP4,
                    ROOT::RVec<int> &StdHepPdg, ROOT::RVec<int> &StdHepStatus) {
                  ROOT::RVec<TLorentzVector> particle_p4;
                  for (int i = 0; i < StdHepN; i++) {
                    if (StdHepPdg[i] == pdgid &&
                        StdHepStatus[i] == 1) { // select all pi+
                      particle_p4.emplace_back(
                          StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                          StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]);
                    }
                  }
                  return particle_p4;
                },
                {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"})
        .Define("particleE",
                [](const ROOT::RVec<TLorentzVector> &particle_p4) {
                  ROOT::RVec<double> particleE;
                  for (auto &p : particle_p4) {
                    particleE.emplace_back(p.E() - p.M());
                  }
                  return particleE;
                },
                {"particle_p4"})
        .Define("p4_neutrino",
                [](ROOT::RVec<double> &StdHepP4) {
                  return TLorentzVector{StdHepP4[0], StdHepP4[1], StdHepP4[2],
                                        StdHepP4[3]};
                },
                {"StdHepP4"})
        .Define("theta",
                [](const ROOT::RVec<TLorentzVector> &particle_p4,
                   const TLorentzVector &p4_neutrino) {
                  ROOT::RVec<double> piontheta;
                  for (auto &p : particle_p4) {
                    piontheta.emplace_back(p.Angle(p4_neutrino.Vect()));
                  }
                  return piontheta;
                },
                {"particle_p4", "p4_neutrino"});
  }

  void configure(const nlohmann::json &conf) override {
    pdgid = conf.value("pdg", 2212);
  }

private:
  int pdgid{2212};
};

REGISTER_PROCESS_NODE(GENIESingleKin)