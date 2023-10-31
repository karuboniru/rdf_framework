#pragma once

#include "common.h"
#include <memory>
#include <nlohmann/json.hpp>
std::unique_ptr<ProcessNodeI>
get_node_process_callable(const nlohmann::json &conf);

std::unique_ptr<NormalizeI> get_normalize_callable(const nlohmann::json &conf);

class noop : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override { return df; }
};