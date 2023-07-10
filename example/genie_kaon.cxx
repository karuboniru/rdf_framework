#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TF1.h>
#include <TLorentzVector.h>
#include <TObjString.h>
#include <TSpline.h>
#include <genie_general.h>

ROOT::RDF::RNode define_variable(ROOT::RDF::RNode df) {
  return df
      .Filter([](ROOT::RVec<TLorentzVector> kaons) { return !kaons.empty(); },
              {"kaons"}, "kaon_cut")
      .Define("kaonE",
              [](ROOT::RVec<TLorentzVector> &kaons) {
                ROOT::RVec<double> kaonE;
                for (auto &p : kaons) {
                  kaonE.emplace_back(p.E() - p.M());
                }
                return kaonE;
              },
              {"kaons"})
      .Define("leading_kaon_e",
              [](ROOT::RVec<double> &kaonE) {
                return *std::max_element(kaonE.begin(), kaonE.end());
              },
              {"kaonE"})
      .Define("subleading_kaon_e",
              [](ROOT::RVec<double> &kaonE) {
                ROOT::RVec<double> ret{};
                if (kaonE.size() > 1) {
                  std::sort(kaonE.begin(), kaonE.end(), std::greater<double>());
                  for (size_t i = 1; i < kaonE.size(); i++) {
                    ret.emplace_back(kaonE[i]);
                  }
                }
                return ret;
              },
              {"kaonE"});
}

ROOT::RDF::RNode kaon_kin(ROOT::RDF::RNode df) {
  return define_variable(df.Define(
      "kaons",
      [](int StdHepN, ROOT::RVec<double> &StdHepP4, ROOT::RVec<int> &StdHepPdg,
         ROOT::RVec<int> &StdHepStatus) {
        ROOT::RVec<TLorentzVector> kaons;
        for (int i = 0; i < StdHepN; i++) {
          if (StdHepPdg[i] == 321 && StdHepStatus[i] == 1) {
            kaons.emplace_back(StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                               StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]);
          }
        }
        return kaons;
      },
      {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"}));
}

ROOT::RDF::RNode kaon_kin_beforefsi(ROOT::RDF::RNode df) {
  return define_variable(df.Define(
      "kaons",
      [](int StdHepN, ROOT::RVec<double> &StdHepP4, ROOT::RVec<int> &StdHepPdg,
         ROOT::RVec<int> &StdHepStatus) {
        ROOT::RVec<TLorentzVector> kaons;
        for (int i = 0; i < StdHepN; i++) {
          if (StdHepPdg[i] == 321 && StdHepStatus[i] == 14) {
            kaons.emplace_back(StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                               StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]);
          }
        }
        return kaons;
      },
      {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"}));
}

ROOT::RDF::RNode pions(ROOT::RDF::RNode df) { // name maybe confusing
  return define_variable(df.Define(
      "kaons",
      [](int StdHepN, ROOT::RVec<double> &StdHepP4, ROOT::RVec<int> &StdHepPdg,
         ROOT::RVec<int> &StdHepStatus) {
        ROOT::RVec<TLorentzVector> kaons;
        for (int i = 0; i < StdHepN; i++) {
          if (StdHepPdg[i] == 211 && StdHepStatus[i] == 1) {
            kaons.emplace_back(StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                               StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]);
          }
        }
        return kaons;
      },
      {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"}));
}

ROOT::RDF::RNode muons(ROOT::RDF::RNode df) { // name maybe confusing
  return define_variable(df.Define(
      "kaons",
      [](int StdHepN, ROOT::RVec<double> &StdHepP4, ROOT::RVec<int> &StdHepPdg,
         ROOT::RVec<int> &StdHepStatus) {
        ROOT::RVec<TLorentzVector> kaons;
        for (int i = 0; i < StdHepN; i++) {
          if (StdHepPdg[i] == 13 && StdHepStatus[i] == 1) {
            kaons.emplace_back(StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                               StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]);
          }
        }
        return kaons;
      },
      {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"}));
}

ROOT::RDF::RNode protons(ROOT::RDF::RNode df) { // name maybe confusing
  return define_variable(df.Define(
      "kaons",
      [](int StdHepN, ROOT::RVec<double> &StdHepP4, ROOT::RVec<int> &StdHepPdg,
         ROOT::RVec<int> &StdHepStatus) {
        ROOT::RVec<TLorentzVector> kaons;
        for (int i = 0; i < StdHepN; i++) {
          if (StdHepPdg[i] == 2212 && StdHepStatus[i] == 1) {
            kaons.emplace_back(StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                               StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]);
          }
        }
        return kaons;
      },
      {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"}));
}
