#include <common.h>
#include <event.h>

class ANALYSIS : public ProcessNodeI {
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df
        .Define("Q2",
                [](const event &GenEvent) {
                  auto lvq =
                      GenEvent.get_neutrino() - GenEvent.get_primary_lepton();
                  return -lvq.M2();
                },
                {"GenEvent"})
        .Define("W",
                [](const event &GenEvent) {
                  auto p4_hadronic_system = GenEvent.get_nucleon() +
                                            GenEvent.get_neutrino() -
                                            GenEvent.get_primary_lepton();
                  return p4_hadronic_system.M();
                },
                {"GenEvent"})
        .Define("xBj",
                [](const event &GenEvent, double Q2) {
                  auto lvq =
                      GenEvent.get_neutrino() - GenEvent.get_primary_lepton();
                  auto initN = GenEvent.get_nucleon();
                  return Q2 / (2 * initN.Dot(lvq));
                },
                {"GenEvent", "Q2"})
        .Define("y",
                [](const event &GenEvent) {
                  auto enu = GenEvent.get_neutrino().E();
                  auto el = GenEvent.get_primary_lepton().E();
                  return el / enu;
                },
                {"GenEvent"})
        .Define("npi0",
                [](const event &GenEvent) {
                  return GenEvent.count_particle_out(111);
                },
                {"GenEvent"})
        .Define("npi_charged",
                [](const event &GenEvent) {
                  return GenEvent.count_particle_out(211) +
                         GenEvent.count_particle_out(-211);
                },
                {"GenEvent"})
        .Define("n_neutron",
                [](const event &GenEvent) {
                  return GenEvent.count_particle_out(2112);
                },
                {"GenEvent"});
  }

  void configure(const nlohmann::json &) override {};
};

REGISTER_PROCESS_NODE(ANALYSIS)
