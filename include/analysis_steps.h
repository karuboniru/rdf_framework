#pragma once
#include "common.h"
#include <memory>
#include <nlohmann/json.hpp>

std::tuple<std::unique_ptr<TChain>, std::vector<std::unique_ptr<TChain>>>
prepare_chain(nlohmann::json &conf);
void plugin_handle(TChain &ch, nlohmann::json &j);
// void analysis_entry_handle(ROOT::RDF::RNode preprocessed_node,
//                            nlohmann::json &analysis_entry);
