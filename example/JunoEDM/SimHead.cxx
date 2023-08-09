#include "Event/EventObject.h"
#include "Event/SimHeader.h"
#include "Event/SimTrack.h"
#include "EvtNavigator/EvtNavHelper.h"
#include <Event/SimEvt.h>
#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>

ROOT::RDF::RNode process(ROOT::RDF::RNode rdf) {
  return rdf
      .Define("NNVTHits", [](JM::SimEvt &event) { return event.getNNVTHits(); },
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