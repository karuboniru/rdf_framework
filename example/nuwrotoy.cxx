#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>

ROOT::RDF::RNode pre(ROOT::RDF::RNode df) {
  return df.Filter("StdHepN>3", "111")
      .Define("Y", [](int StdHepN) { return StdHepN + 1; }, {"StdHepN"});
}

ROOT::RDF::RNode anal(ROOT::RDF::RNode df) {
  return df.Define("X", [](int StdHepN, int Y) { return StdHepN - Y; },
                   {"StdHepN", "Y"});
}
