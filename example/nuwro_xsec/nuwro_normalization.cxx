#include "common.h"

// This is different from a normal normalization
// for NuWro, but only for plotting \sigma(E)
class normalize_factor_nuwro : public NormalizeI {
public:
  double operator()(ROOT::RDF::RNode df) override {
    auto mean = df.Mean("EvtWght");
    auto num = df.Count();
    return mean.GetValue() / num.GetValue() / ((Emax - Emin) / bin_count);
  }
  void configure(nlohmann::json const &p) override {
    bin_count = p["bin_count"].get<size_t>();
    Emin = p["Emin"].get<double>();
    Emax = p["Emax"].get<double>();
  }

private:
  size_t bin_count{};
  double Emin{};
  double Emax{};
};

REGISTER_NORMALIZE(normalize_factor_nuwro);