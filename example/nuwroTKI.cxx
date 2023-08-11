#include "ROOT/RVec.hxx"
#include "TF1.h"
#include "TLorentzVector.h"
#include "TObjString.h"
#include "TRandom3.h"
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>
#include <array>
#include <cassert>
#include <common.h>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <tki_general.h>
#include <type_traits>
#include <vector>

event::channel getmode_nuwro(TObjString &code) {
  const int mode = code.GetString().Atoi();
  // cout << "mode = " << mode << endl;
  switch (mode) {
  case 1:
    return event::channel::QE;
    break;
  case 11:
    return event::channel::RES;
    break;
  case 26:
    return event::channel::DIS;
    break;
  case 2:
    return event::channel::MEC;
    break;
  case 16:
  default:
    return event::channel::Other;
    break;
  }
}

ROOT::RDF::RNode NuWro_RDF_setup_event(ROOT::RDF::RNode df) {
  return df
      .Define("event",
              [](int StdHepN, ROOT::RVec<int> &StdHepPdg,
                 ROOT::RVec<int> &StdHepStatus, ROOT::RVec<double> &StdHepP4_,
                 TObjString &EvtCode) {
                double(*StdHepP4)[4] = (double(*)[4]) & StdHepP4_[0];
                event e{};
                if (getmode_nuwro(EvtCode) == event::channel::Other) {
                  return e;
                }
                e.set_mode(getmode_nuwro(EvtCode));
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

class pre : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return MINERvAGFS_general(NuWro_RDF_setup_event(df));
  }
};

REGISTER_PROCESS_NODE(pre);

// double normalize_factor_CC(ROOT::RDF::RNode df, std::vector<std::string>) {
//   auto mean = df.Mean("EvtWght");
//   auto num = df.Count();
//   return mean.GetValue() / num.GetValue();
// }

class normalize_factor_CC : NormalizeI {
public:
  double operator()(ROOT::RDF::RNode df) override {
    auto mean = df.Mean("EvtWght");
    auto num = df.Count();
    return mean.GetValue() / num.GetValue();
  }
};