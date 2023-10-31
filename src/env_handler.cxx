#include "env_handler.h"

env_handler &env_handler::get_instance() {
  static env_handler instance;
  return instance;
}
extern char **environ;
env_handler::env_handler() {
  for (auto s = environ; *s; s++) {
    std::string str{*s};
    auto pos = str.find('=');
    if (pos != std::string_view::npos) {
      envs[str.substr(0, pos)] = str.substr(pos + 1);
    }
  }
}

void env_handler::set_env(const std::string_view key,
                          const std::string_view value) {
  envs[std::string{key}] = value;
}

std::string env_handler::get_env(const std::string_view key) {
  auto it = envs.find(std::string{key});
  if (it != envs.end()) {
    return it->second;
  }
  return {};
}

// replace ${ENV} and $ENV with the value of ENV
std::string operator|(const std::string_view lhs, env_handler &rhs) {
  std::string ret{lhs};
  auto pos = ret.find("${");
  while (pos != std::string::npos) {
    auto end = ret.find('}', pos);
    if (end != std::string::npos) {
      auto key = ret.substr(pos + 2, end - pos - 2);
      auto value = rhs.get_env(key);
      if (!value.empty()) {
        ret.replace(pos, end - pos + 1, value);
      }
    }
    pos = ret.find("${", pos + 1);
  }
  pos = ret.find('$');
  while (pos != std::string::npos) {
    auto end = ret.find(' ', pos);
    if (end != std::string::npos) {
      auto key = ret.substr(pos + 1, end - pos - 1);
      auto value = rhs.get_env(key);
      if (!value.empty()) {
        ret.replace(pos, end - pos + 1, value);
      }
    }
    pos = ret.find('$', pos + 1);
  }
  return ret;
}