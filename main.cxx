#include <ROOT/RDF/InterfaceUtils.hxx>
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <TChain.h>
#include <TDirectory.h>
#include <THStack.h>
#include <TROOT.h>
#include <TSystem.h>
#include <common.h>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

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
