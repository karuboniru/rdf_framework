#pragma once
#include <TLorentzVector.h>
#include <unordered_map>

enum class interaction_channel { qel, res, dis, mec, coh, unknown };

class event {
public:
  void add_particle_in(int, TLorentzVector);
  void add_particle_out(int, TLorentzVector);
  void set_interaction(interaction_channel);
  void set_hit_nucleon(TLorentzVector p4) { nucleon = p4; }

  TLorentzVector get_leading_out(int) const;
  size_t count_particle_out(int) const;
  int get_neutrino_pdg() const { return neutrino_pdg; }
  int get_nucleus_pdg() const { return nucleus_pdg; }
  TLorentzVector get_neutrino() const { return neutrino; }
  TLorentzVector get_nucleus() const { return nucleus; }
  TLorentzVector get_nucleon() const { return nucleon; }
  TLorentzVector get_primary_lepton() const { return primary_lepton; }
  interaction_channel get_interaction() const { return interaction; }

private:
  std::unordered_multimap<int, TLorentzVector> particles_in{};
  std::unordered_multimap<int, TLorentzVector> particles_out{};
  interaction_channel interaction;
  int neutrino_pdg{}, nucleus_pdg{};
  TLorentzVector primary_lepton{}, neutrino{}, nucleus{}, nucleon{};
};