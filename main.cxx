#include <ROOT/RDataFrame.hxx>
#include <TChain.h>
#include <common.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

#include "analysis_steps.h"
#include "env_handler.h"
#include "get_callable.h"
#include "hist_draw.h"
#include "wildcard_expand.h"

int main(int argc, char **argv) {
  ROOT::EnableImplicitMT();
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <json_file>" << std::endl;
    return 1;
  }

  if (argc > 2) {
    for (int i = 2; i < argc; i++) {
      std::string arg{argv[i]};
      auto pos = arg.find('=');
      if (pos != std::string_view::npos) {
        env_handler::get_instance().set_env(arg.substr(0, pos),
                                            arg.substr(pos + 1));
      }
    }
  }

  nlohmann::json j;
  {
    std::ifstream json_file(argv[1]);
    std::stringstream buffer;
    buffer << json_file.rdbuf();
    std::stringstream{buffer.str() | env_handler::get_instance()} >> j;
  }
  auto filechain = prepare_chain(j);
  ROOT::RDataFrame df(*filechain);
  auto rootnode = ROOT::RDF::AsRNode(df);

  for (auto &entry : j["plugins"]) {
    plugin_handle(rootnode, entry);
  }

  return 0;
}
