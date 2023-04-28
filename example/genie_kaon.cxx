#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TF1.h>
#include <TLorentzVector.h>
#include <TObjString.h>
#include <TSpline.h>

template <typename T>
std::unique_ptr<T> get_object(std::string file_path, std::string obj_path) {
  TFile root_file{file_path.c_str(), "READ"};
  auto objptr = static_cast<T *>(root_file.Get(obj_path.c_str())->Clone());
  assert(objptr);
  return std::unique_ptr<T>{objptr};
}

ROOT::RDF::RNode CC_selection(ROOT::RDF::RNode df) {
  return df
      .Filter([](TObjString &str) { return str.GetString().Contains("CC"); },
              {"EvtCode"}, "CC_cut")
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
              {"StdHepN", "StdHepP4", "StdHepPdg", "StdHepStatus"});
}

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

std::pair<double, double> get_xsec(TH1 *h_rate, TGraph *spline) {
  double fluxint{};
  // spline->SaveAs("wrong.root");
  TSpline3 sp("sp", spline);
  TF1 func(
      "spline", [&](double *x, double *) { return sp.Eval(*x); }, 0,
      h_rate->GetXaxis()->GetXmax(), 0);
  for (int ii = 1; ii <= h_rate->GetNbinsX(); ii++) {
    double bin_c = h_rate->GetBinContent(ii);
    double bin_up = h_rate->GetXaxis()->GetBinUpEdge(ii);
    double bin_low = h_rate->GetXaxis()->GetBinLowEdge(ii);
    double bin_width = bin_up - bin_low;
    if (bin_c < 1 || func.Integral(bin_low, bin_up) == 0) {
      continue;
    }
    fluxint += bin_c / func.Integral(bin_low, bin_up) * bin_width;
  }
  double event_rate = h_rate->Integral();
  return {event_rate, event_rate / fluxint};
}

double normalize_factor_CC(ROOT::RDF::RNode df,
                           std::vector<std::string> parameters) {
  auto h = df.Histo1D({"", "", 256, 0, 0}, "neutrinoE");
  auto filename = parameters[0];
  auto obj_path = parameters[1];
  auto Z = std::stoi(parameters[2]);
  auto spline_obj = get_object<TGraph>(filename, obj_path);
  auto [tot, xsec] = get_xsec(h.GetPtr(), spline_obj.get());
  xsec *= 1. / ((double)Z) * 1e-38;
  return xsec / tot;
}
