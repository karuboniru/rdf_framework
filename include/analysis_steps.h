#pragma once
#include "common.h"
#include <memory>
#include <nlohmann/json.hpp>

std::unique_ptr<TChain> prepare_chain(const nlohmann::json &conf);
void plugin_handle(ROOT::RDF::RNode node, const nlohmann::json &j);
void analysis_entry_handle(ROOT::RDF::RNode preprocessed_node,
                           const nlohmann::json &analysis_entry,
                           double norm_factor = 0);