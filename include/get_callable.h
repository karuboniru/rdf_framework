#pragma once

#include "common.h"
#include <memory>
#include <nlohmann/json.hpp>
std::unique_ptr<ProcessNodeI>
get_node_process_callable(nlohmann::json &conf);

std::unique_ptr<NormalizeI> get_normalize_callable(nlohmann::json &conf);

class noop : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override { return df; }
};

class StrCut : ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Filter(cut);
  }
  void configure(const nlohmann::json &conf) override {
    cut = conf["cut"].get<std::string>();
  };

private:
  std::string cut{};
};