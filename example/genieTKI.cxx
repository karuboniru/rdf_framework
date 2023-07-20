#include <TObjString.h>
#include <array>
#include "TF1.h"
#include "TRandom3.h"
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>
#include <TSpline.h>
#include <array>
#include <iostream>
#include <map>
#include <set>
#include <type_traits>
#include <vector>
#include <tki_general.h>

event::channel get_mode_genie(const TObjString &code) {
  if (code.GetString().Contains("QES")) {
    return event::channel::QE;
  } else if (code.GetString().Contains("RES")) {
    return event::channel::RES;
  } else if (code.GetString().Contains("DIS")) {
    return event::channel::DIS;
  } else if (code.GetString().Contains("MEC")) {
    return event::channel::MEC;
  }
  return event::channel::Other;
}

ROOT::RDF::RNode GENIE_RDF_setup_event(ROOT::RDF::RNode df) {
  return df
      .Filter(
          [](TObjString &EvtCode) {
            auto str = EvtCode.GetString();
            return str.Contains("CC");
          },
          {"EvtCode"})
      .Define("neutrinoE",
              [](int StdHepN, ROOT::RVec<double> &StdHepP4,
                 ROOT::RVec<int> &StdHepPdg, ROOT::RVec<int> &StdHepStatus) {
                for (int i = 0; i < StdHepN; i++) {
                  if ((abs(StdHepPdg[i]) == 12 || abs(StdHepPdg[i]) == 14 ||
                       abs(StdHepPdg[i]) == 16) &&
                      StdHepStatus[i] == 0) {
                    return StdHepP4[4 * i + 3];
                  }
                }
                return 0.;
              },
              {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"})
      .Define("event",
              [](int StdHepN, ROOT::RVec<int> &StdHepPdg,
                 ROOT::RVec<int> &StdHepStatus, ROOT::RVec<double> &StdHepP4_,
                 TObjString &EvtCode) {
                double(*StdHepP4)[4] = (double(*)[4]) & StdHepP4_[0];
                event e{};
                e.set_mode(get_mode_genie(EvtCode));
                for (int i = 0; i < StdHepN; ++i) {
                  auto pdg = StdHepPdg[i];
                  if (StdHepPdg[i] == 1000000010) {
                    pdg = 2112;
                  }
                  switch (StdHepStatus[i]) {
                  case 0:
                    e.add_particle_in(
                        pdg, TLorentzVector(StdHepP4[i][0], StdHepP4[i][1],
                                            StdHepP4[i][2], StdHepP4[i][3]));
                    break;
                  case 1:
                    e.add_particle_out(
                        pdg, TLorentzVector(StdHepP4[i][0], StdHepP4[i][1],
                                            StdHepP4[i][2], StdHepP4[i][3]));
                    break;
                  case 2: {
                    e.add_particle_nofsi(
                        pdg, TLorentzVector(StdHepP4[i][0], StdHepP4[i][1],
                                            StdHepP4[i][2], StdHepP4[i][3]));
                  } break;
                  default:
                    break;
                  }
                }
                return e;
              },
              {"StdHepN", "StdHepPdg", "StdHepStatus", "StdHepP4", "EvtCode"})
      .Filter([](event &e) { return e.get_mode() != event::channel::Other; },
              {"event"});
}


ROOT::RDF::RNode pre(ROOT::RDF::RNode df) { return GENIE_RDF_setup_event(df); }