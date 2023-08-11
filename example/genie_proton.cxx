#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TF1.h>
#include <TLorentzVector.h>
#include <TObjString.h>
#include <TSpline.h>
#include <common.h>
#include <genie_general.h>

ROOT::RDF::RNode define_variable(ROOT::RDF::RNode df) {
  return df
      .Filter(
          [](ROOT::RVec<std::pair<TLorentzVector, int>> &protons) {
            return !protons.empty();
          },
          {"protons"}, "proton_cut")
      .Define("anyid",
              [](ROOT::RVec<std::pair<TLorentzVector, int>> &protons,
                 ROOT::RVec<int> &StdHepStatus, ROOT::RVec<int> &StdHepFm) {
                ROOT::RVec<int> id{};
                for (auto &p : protons) {
                  if (StdHepFm[StdHepFm[p.second]] != -1)
                    id.emplace_back(StdHepStatus[StdHepFm[StdHepFm[p.second]]]);
                }
                return id;
              },
              {"protons", "StdHepStatus", "StdHepFm"})
      .Define("Fm_PDG",
              [](ROOT::RVec<std::pair<TLorentzVector, int>> &protons,
                 ROOT::RVec<int> &StdHepFm, ROOT::RVec<int> &StdHepPdg) {
                ROOT::RVec<int> id{};
                for (auto &p : protons) {
                  id.emplace_back(StdHepPdg[StdHepFm[p.second]]);
                }
                return id;
              },
              {"protons", "StdHepFm", "StdHepPdg"})
      .Define("proton_P",
              [](ROOT::RVec<std::pair<TLorentzVector, int>> &protons) {
                ROOT::RVec<double> proton_P;
                for (auto &p : protons) {
                  proton_P.emplace_back(p.first.P());
                }
                return proton_P;
              },
              {"protons"})
      .Define("leading_proton_p",
              [](ROOT::RVec<double> &proton_P) {
                return *std::max_element(proton_P.begin(), proton_P.end());
              },
              {"proton_P"})
      .Define("nonleading_proton_e",
              [](ROOT::RVec<double> &proton_P) {
                ROOT::RVec<double> ret{};
                if (proton_P.size() > 1) {
                  std::sort(proton_P.begin(), proton_P.end(),
                            std::greater<double>());
                  for (size_t i = 1; i < proton_P.size(); i++) {
                    ret.emplace_back(proton_P[i]);
                  }
                }
                return ret;
              },
              {"proton_P"});
}

class proton_kin : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return define_variable(df.Define(
        "protons",
        [](int StdHepN, ROOT::RVec<double> &StdHepP4,
           ROOT::RVec<int> &StdHepPdg, ROOT::RVec<int> &StdHepStatus) {
          ROOT::RVec<std::pair<TLorentzVector, int>> protons;
          for (int i = 0; i < StdHepN; i++) {
            if (StdHepPdg[i] == 2212 && StdHepStatus[i] == 1) {
              protons.emplace_back(
                  TLorentzVector{StdHepP4[4 * i + 0], StdHepP4[4 * i + 1],
                                 StdHepP4[4 * i + 2], StdHepP4[4 * i + 3]},
                  i);
            }
          }
          return protons;
        },
        {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"}));
  }
};

REGISTER_PROCESS_NODE(proton_kin);
