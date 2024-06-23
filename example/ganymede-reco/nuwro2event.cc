#include "common.h"
#include <ROOT/RDF/HistoModels.hxx>
#include <TF1.h>
#include <TObjString.h>
#include <TSpline.h>
#include <common.h>
#include <event.h>

class NuWro2Event : public ProcessNodeI {
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df
        .Define(
            "GenEvent",
            [](int StdHepN, const ROOT::RVec<double> &StdHepP4,
               const ROOT::RVec<int> &StdHepPdg,
               const ROOT::RVec<int> &StdHepStatus, const TObjString &EvtCode) {
              event e{};
              int code = std::stoi(EvtCode.GetString().Data());
              switch (code) {
              case 1: // ccqel
                e.set_interaction(interaction_channel::qel);
                e.set_is_cc(true);
                break;
              case 51: // ncqel
                e.set_interaction(interaction_channel::qel);
                e.set_is_cc(false);
                break;
              case 11:
                e.set_interaction(interaction_channel::res);
                e.set_is_cc(true);
                break;
              case 31:
                e.set_interaction(interaction_channel::res);
                e.set_is_cc(false);
                break;
              case 26:
                e.set_interaction(interaction_channel::dis);
                e.set_is_cc(true);
                break;
              case 46:
                e.set_interaction(interaction_channel::dis);
                e.set_is_cc(false);
                break;
              case 2:
                e.set_interaction(interaction_channel::mec);
                e.set_is_cc(true);
                break;
              case 16:
                e.set_interaction(interaction_channel::coh);
                e.set_is_cc(true);
                break;
              case 36:
                e.set_interaction(interaction_channel::coh);
                e.set_is_cc(false);
                break;
              default:
                e.set_interaction(interaction_channel::unknown);
                e.set_is_cc(false);
                break;
              }

              for (int i = 0; i < StdHepN; i++) {
                bool nucleon_found = false;
                switch (StdHepStatus[i]) {
                case 0:
                  e.add_particle_in(
                      StdHepPdg[i],
                      TLorentzVector{StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                                     StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]});
                  break;
                case 1:
                  e.add_particle_out(
                      StdHepPdg[i],
                      TLorentzVector{StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                                     StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]});
                  if (abs(StdHepPdg[i]) > 100 && !nucleon_found) {
                    e.set_hit_nucleon(TLorentzVector{
                        StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                        StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]});
                    nucleon_found = true;
                  }
                  break;
                default:
                  break;
                }
              }

              return e;
            },
            {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus", "EvtCode"})
        .Filter(
            [](const event &GenEvent) {
              return GenEvent.get_interaction() !=
                         interaction_channel::unknown &&
                     GenEvent.get_interaction() != interaction_channel::coh;
            },
            {"GenEvent"})
        .Define("cc_flag", [](const event &e) { return e.get_is_cc(); },
                {"GenEvent"});
    ;
  }
};

REGISTER_PROCESS_NODE(NuWro2Event)