#include "common.h"

class normalize_factor_nuwro : public NormalizeI {
public:
  double operator()(ROOT::RDF::RNode df) override {
    auto mean = df.Mean("EvtWght");
    auto num = df.Count();
    return mean.GetValue() / num.GetValue();
  }
};

REGISTER_NORMALIZE(normalize_factor_nuwro);