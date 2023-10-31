#include "common.h"

class xsecplot : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return 
    df.Define ("E", [](ROOT::RVec<double> StdHepP4){
      return StdHepP4[3];
    }, {"StdHepP4"});
  }
};

REGISTER_PROCESS_NODE(xsecplot);

