#include "get_callable.h"

std::unique_ptr<ProcessNodeI>
get_node_process_callable(nlohmann::json &conf) {
  std::string name;
  nlohmann::json config{};
  if (conf.is_string()) {
    name = conf.get<std::string>();
  } else {
    name = conf["name"].get<std::string>();
    config = conf.value("config", nlohmann::json());
  }
  auto ptr = ProcessNodeFactory::instance().create(name);
  ptr->configure(config);
  return ptr;
}

std::unique_ptr<NormalizeI> get_normalize_callable(nlohmann::json &conf) {
  std::string name;
  nlohmann::json config{};
  if (conf.is_string()) {
    name = conf.get<std::string>();
  } else {
    name = conf["name"].get<std::string>();
    config = conf.value("config", nlohmann::json());
  }
  auto ptr = NormalizeFactory::instance().create(name);
  ptr->configure(config);
  return ptr;
}



REGISTER_PROCESS_NODE(noop)

