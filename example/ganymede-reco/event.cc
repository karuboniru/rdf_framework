#include <event.h>

void event::add_particle_in(int pdg, TLorentzVector p4) {
  particles_in.insert({pdg, p4});
  if (abs(pdg) < 100 && neutrino_pdg == 0) {
    neutrino_pdg = pdg;
    neutrino = p4;
  }
  if (abs(pdg) > 100 && nucleus_pdg == 0) {
    nucleus_pdg = pdg;
    nucleus = p4;
    if (nucleon.P() == 0)
      set_hit_nucleon(p4);
  }
}

void event::add_particle_out(int pdg, TLorentzVector p4) {
  particles_out.insert({pdg, p4});
  // now consider the case of NC events
  if ((abs(pdg) >= 11 || abs(pdg) <= 14) && primary_lepton.P() == 0)
    primary_lepton = p4;
}

void event::set_interaction(interaction_channel ic) { interaction = ic; }

TLorentzVector event::get_leading_out(int pdg) const {
  TLorentzVector leading{};
  auto range = particles_out.equal_range(pdg);
  for (auto it = range.first; it != range.second; ++it) {
    if (it->second.P() > leading.P()) {
      leading = it->second;
    }
  }
  return leading;
}

size_t event::count_particle_out(int pdg) const {
  return particles_out.count(pdg);
}