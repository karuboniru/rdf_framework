#include "Event/EventObject.h"
#include "Event/SimEvt.h"
#include "Event/SimHeader.h"
#include "Event/SimTrack.h"
#include "EvtNavigator/EvtNavHelper.h"
#include "common.h"
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>

class process : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode rdf) override {
    return rdf
        .Define("NNVTHits",
                [](JM::SimEvt &event) { return event.getNNVTHits(); },
                {"SimEvt"})
        .Define("trackPDG",
                [](JM::SimEvt &event) {
                  ROOT::RVec<int> pdglist;
                  auto &&tracklist = event.getTracksVec();
                  for (auto &&track : tracklist) {
                    pdglist.emplace_back(track->getPDGID());
                  }
                  return pdglist;
                },
                {"SimEvt"});
  }
};

REGISTER_PROCESS_NODE(process);