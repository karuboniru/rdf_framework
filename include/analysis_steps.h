#pragma once
#include "common.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <variant>

using general_hist_result_t =
    std::variant<ROOT::RDF::RResultPtr<TH1D>, ROOT::RDF::RResultPtr<TH2D>,
                 ROOT::RDF::RResultPtr<TH3D>>;

std::tuple<std::unique_ptr<TChain>, std::vector<std::unique_ptr<TChain>>>
prepare_chain(nlohmann::json &conf);
void plugin_handle(TChain &ch, nlohmann::json &j);
// void analysis_entry_handle(ROOT::RDF::RNode preprocessed_node,
//                            nlohmann::json &analysis_entry);
