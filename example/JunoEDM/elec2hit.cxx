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
#include "TH3.h"
#include "common.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TCanvas.h>
#include <TClass.h>
#include <TError.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TVector3.h>
#include <TView.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
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
        .Define("plotnpe",
                [](const std::tuple<std::vector<double>, std::vector<double>,
                                    std::vector<double>, std::vector<double>,
                                    std::vector<double>> &electruth) {
                  auto &&[npe, hitTime, theta, phi, amplitude] = electruth;
                  TGraph2D graph{
                      static_cast<int>(npe.size()),
                      const_cast<double *>(&theta[0]),
                      const_cast<double *>(&phi[0]),
                      const_cast<double *>(&npe[0]),
                  };
                  graph.SetMarkerStyle(21);
                  graph.SetMarkerSize(1.0);
                  return graph;
                },
                {"electruth"})
        .Define("plothittime",
                [](const std::tuple<std::vector<double>, std::vector<double>,
                                    std::vector<double>, std::vector<double>,
                                    std::vector<double>> &electruth) {
                  auto &&[npe, hitTime, theta, phi, amplitude] = electruth;
                  TGraph2D graph{
                      static_cast<int>(hitTime.size()),
                      const_cast<double *>(&theta[0]),
                      const_cast<double *>(&phi[0]),
                      const_cast<double *>(&hitTime[0]),
                  };
                  graph.SetMarkerStyle(21);
                  graph.SetMarkerSize(1.0);
                  return graph;
                },
                {"electruth"})
        .Define("plotamplitude",
                [](const std::tuple<std::vector<double>, std::vector<double>,
                                    std::vector<double>, std::vector<double>,
                                    std::vector<double>> &electruth) {
                  auto &&[npe, hitTime, theta, phi, amplitude] = electruth;
                  TGraph2D graph{
                      static_cast<int>(amplitude.size()),
                      const_cast<double *>(&theta[0]),
                      const_cast<double *>(&phi[0]),
                      const_cast<double *>(&amplitude[0]),
                  };
                  graph.SetMarkerStyle(21);
                  graph.SetMarkerSize(1.0);
                  return graph;
                },
                {"electruth"})
        .Define(
            "maintrack",
            [](const std::vector<JM::SimTrack *> &track) { return track[0]; },
            {"SimEvt.m_tracks"})
        .Define("initpoint",
                [](JM::SimTrack *track) {
                  return TVector3{track->getInitX(), track->getInitY(),
                                  track->getInitZ()};
                },
                {"maintrack"})
        .Define("exitpoint",
                [](JM::SimTrack *track) {
                  return TVector3{track->getExitX(), track->getExitY(),
                                  track->getExitZ()};
                },
                {"maintrack"})
        .Define("init_theta",
                [](const TVector3 &initpoint) { return initpoint.Theta(); },
                {"initpoint"})
        .Define("init_phi",
                [](const TVector3 &initpoint) { return initpoint.Phi(); },
                {"initpoint"})
        .Define("exit_theta",
                [](const TVector3 &exitpoint) { return exitpoint.Theta(); },
                {"exitpoint"})
        .Define("exit_phi",
                [](const TVector3 &exitpoint) { return exitpoint.Phi(); },
                {"exitpoint"});
    ;
  }
};

REGISTER_PROCESS_NODE(process);