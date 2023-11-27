#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <RVersion.h>
#include <TChain.h>
#include <common.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

#include "analysis_steps.h"
#include "env_handler.h"

int main(int argc, char **argv) {
  TH1::AddDirectory(false);
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
  // friend_chains is not actually used
  // but we need to keep it alive
  auto &&[filechain, friend_chains] = prepare_chain(j);
  ROOT::RDataFrame df(*filechain);
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 30, 0)
  ROOT::RDF::Experimental::AddProgressBar(df);
#endif

  auto rootnode = ROOT::RDF::AsRNode(df);

  plugin_handle(rootnode, j["plugins"]);

  return 0;
}
