#include "Event/CdLpmtElecEvt.h"
#include "Event/CdLpmtElecTruthEvt.h"
#include "Event/ElecChannel.h"
#include "Event/EventObject.h"
#include "Event/LpmtElecTruth.h"
#include "Event/SimEvt.h"
#include "Event/SimHeader.h"
#include "Event/SimPMTHit.h"
#include "Event/SimTrack.h"
#include "Event/TrackElecTruth.h"
#include "Event/TrackElecTruthEvt.h"
#include "EvtNavigator/EvtNavHelper.h"
#include "common.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TCanvas.h>
#include <TClass.h>
#include <TError.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TH3.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TView.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <tuple>
#include <unordered_map>

// Get PMT location from csv file
// Provided as a singleton
class JUNOPMTLocation {
public:
  static JUNOPMTLocation &get_instance() {
    static JUNOPMTLocation instance;
    return instance;
  }
  JUNOPMTLocation(
      std::string input =
          "/cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc1120/Pre-Release/"
          "J22.2.0-rc0/data/Detector/Geometry/PMTPos_CD_LPMT.csv") {
    std::ifstream file(input);
    for (std::string line; std::getline(file, line);) {
      std::stringstream ss(line);
      std::string token;
      std::vector<std::string> tokens;
      while (std::getline(ss, token, ' ')) {
        tokens.push_back(token);
      }
      // an assumption that the csv file is well formatted
      // and the first column is pmtid
      // and one by one in order
      m_pmt_pos.emplace_back(std::stod(tokens[1]), std::stod(tokens[2]),
                             std::stod(tokens[3]));
      max_pmt_id++;
    }
  }

  double GetPMTTheta(int pmtid) const { return m_pmt_pos[pmtid].Theta(); }
  double GetPMTPhi(int pmtid) const { return m_pmt_pos[pmtid].Phi(); }
  size_t GetMaxPMTID() const { return max_pmt_id; }

private:
  std::vector<TVector3> m_pmt_pos;
  size_t max_pmt_id{};
};

class process : public ProcessNodeI {
public:
  virtual ROOT::RDF::RNode operator()(ROOT::RDF::RNode rdf) override {
    return rdf
        .Define(
            "maintrack",
            [](const std::vector<JM::SimTrack *> &track) { return track[0]; },
            {"SimEvt.m_tracks"})
        .Define("initP",
                [](JM::SimTrack *track) {
                  return TLorentzVector{
                      track->getInitPx(), track->getInitPy(),
                      track->getInitPz(),
                      std::sqrt(std::pow(track->getInitPx(), 2) +
                                std::pow(track->getInitPy(), 2) +
                                std::pow(track->getInitPz(), 2) +
                                std::pow(track->getInitMass(), 2))};
                },
                {"maintrack"})
        .Define("initpoint",
                [](JM::SimTrack *track) {
                  return TLorentzVector{track->getInitX(), track->getInitY(),
                                        track->getInitZ(), track->getInitT()};
                },
                {"maintrack"})
        .Define("exitpoint",
                [](JM::SimTrack *track) {
                  return TLorentzVector{track->getExitX(), track->getExitY(),
                                        track->getExitZ(), track->getExitT()};
                },
                {"maintrack"})
        .Define(
            "init_theta",
            [](const TLorentzVector &initpoint) { return initpoint.Theta(); },
            {"initpoint"})
        .Define("init_phi",
                [](const TLorentzVector &initpoint) { return initpoint.Phi(); },
                {"initpoint"})
        .Define(
            "exit_theta",
            [](const TLorentzVector &exitpoint) { return exitpoint.Theta(); },
            {"exitpoint"})
        .Define("exit_phi",
                [](const TLorentzVector &exitpoint) { return exitpoint.Phi(); },
                {"exitpoint"})
        .Define(
            "electruth",
            [](JM::CdLpmtElecTruthEvt &elecevent) {
              auto &&truth = elecevent.truths();
              std::unordered_map<size_t, double> hitTime_c, npe_c, amplitude_c;
              std::set<size_t> pmtid_c;
              // std::vector<size_t> pmtid;
              std::vector<double> npe;
              std::vector<double> hitTime;
              std::vector<double> amplitude;
              std::vector<double> theta;
              std::vector<double> phi;
              for (auto &&elec : truth) {
                pmtid_c.insert(elec.pmtId());
                auto &hittime_ref = hitTime_c[elec.pmtId()];
                hittime_ref = std::min(
                    hittime_ref == 0 ? INFINITY : hittime_ref, elec.hitTime());
                npe_c[elec.pmtId()] += elec.npe();
                amplitude_c[elec.pmtId()] =
                    std::max(amplitude_c[elec.pmtId()], elec.amplitude());
              }
              for (auto &&pmtid : pmtid_c) {
                npe.push_back(npe_c[pmtid]);
                hitTime.push_back(hitTime_c[pmtid]);
                theta.push_back(
                    JUNOPMTLocation::get_instance().GetPMTTheta(pmtid));
                phi.push_back(JUNOPMTLocation::get_instance().GetPMTPhi(pmtid));
                amplitude.push_back(amplitude_c[pmtid]);
              }
              return std::make_tuple(npe, hitTime, theta, phi, amplitude);
            },
            {"CdLpmtElecTruthEvt"})
        .Define(
            "plotnpe",
            [&](const std::tuple<std::vector<double>, std::vector<double>,
                                 std::vector<double>, std::vector<double>,
                                 std::vector<double>> &electruth,
                const TLorentzVector &initpoint,
                const TLorentzVector &exitpoint, unsigned int slot,
                ULong64_t entry) {
              auto &&[npe, hitTime, theta, phi, amplitude] = electruth;
              TGraph2D graph{
                  static_cast<int>(npe.size()),
                  const_cast<double *>(&theta[0]),
                  const_cast<double *>(&phi[0]),
                  const_cast<double *>(&npe[0]),
              };
              if (!save_path_prefix.empty())
                plot_graph(graph, initpoint.Vect(), exitpoint.Vect(), slot,
                           entry, "plotnpe");
              return graph;
            },
            {"electruth", "initpoint", "exitpoint", "rdfslot_", "rdfentry_"})
        .Define(
            "plothittime",
            [&](const std::tuple<std::vector<double>, std::vector<double>,
                                 std::vector<double>, std::vector<double>,
                                 std::vector<double>> &electruth,
                const TLorentzVector &initpoint,
                const TLorentzVector &exitpoint, unsigned int slot,
                ULong64_t entry) {
              auto &&[npe, hitTime, theta, phi, amplitude] = electruth;
              TGraph2D graph{
                  static_cast<int>(hitTime.size()),
                  const_cast<double *>(&theta[0]),
                  const_cast<double *>(&phi[0]),
                  const_cast<double *>(&hitTime[0]),
              };
              if (!save_path_prefix.empty())
                plot_graph(graph, initpoint.Vect(), exitpoint.Vect(), slot,
                           entry, "plothittime");
              return graph;
            },
            {"electruth", "initpoint", "exitpoint", "rdfslot_", "rdfentry_"})
        .Define(
            "plotamplitude",
            [&](const std::tuple<std::vector<double>, std::vector<double>,
                                 std::vector<double>, std::vector<double>,
                                 std::vector<double>> &electruth,
                const TLorentzVector &initpoint,
                const TLorentzVector &exitpoint, unsigned int slot,
                ULong64_t entry) {
              auto &&[npe, hitTime, theta, phi, amplitude] = electruth;
              TGraph2D graph{
                  static_cast<int>(amplitude.size()),
                  const_cast<double *>(&theta[0]),
                  const_cast<double *>(&phi[0]),
                  const_cast<double *>(&amplitude[0]),
              };
              if (!save_path_prefix.empty())
                plot_graph(graph, initpoint.Vect(), exitpoint.Vect(), slot,
                           entry, "plotamplitude");
              return graph;
            },
            {"electruth", "initpoint", "exitpoint", "rdfslot_", "rdfentry_"});
  }

  virtual void configure(const nlohmann::json &config) override {
    save_path_prefix = config["save_path_prefix"].get<std::string>();
    std::cout << "save_path_prefix: " << save_path_prefix << std::endl;
    if (!save_path_prefix.empty()) {
      std::filesystem::create_directories(save_path_prefix);
    }
    if (save_path_prefix.back() != '/') {
      save_path_prefix += '/';
    }
  }

private:
  std::string save_path_prefix{};
  void plot_graph(TGraph2D &gh, TVector3 init, TVector3 exit, unsigned int slot,
                  ULong64_t entry, std::string name = "") {
    auto c1 = std::make_unique<TCanvas>();
    gh.Draw("PCOLZ");
    gh.SetMarkerStyle(21);
    gh.SetMarkerSize(0.8);
    // auto *min = new TGraph2D();
    auto mark = std::make_unique<TGraph2D>();
    mark->SetPoint(0, init.Theta(), init.Phi(), 1000);
    mark->SetPoint(1, exit.Theta(), exit.Phi(), 1000);
    mark->SetMarkerStyle(29);
    mark->SetMarkerColor(kRed);
    mark->Draw("Psame");
    c1->Draw();
    c1->SetTitle("plothittime");
    gPad->GetView()->TopView();
    gPad->Update();
    // auto path = (save_path_prefix + name + std::to_string(slot) + "_" +
    //              std::to_string(entry) + ".png");
    // std::cout << "saving to " << path << std::endl;
    c1->SaveAs((save_path_prefix + name + std::to_string(slot) + "_" +
                std::to_string(entry) + ".png")
                   .c_str());
    c1->SaveAs((save_path_prefix + name + std::to_string(slot) + "_" +
                std::to_string(entry) + ".pdf")
                   .c_str());
    // c1->SaveAs("plot.pdf");
  };
};

REGISTER_PROCESS_NODE(process);