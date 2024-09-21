#include "env_handler.h"
#include <array>

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

// replace ${ENV} with the value of ENV
std::string operator|(const std::string_view lhs, env_handler &rhs) {
  std::string ret{lhs};
  for (auto pos = ret.find("${"); pos != std::string::npos;
       pos = ret.find("${", pos + 1)) {
    auto end = ret.find('}', pos);
    if (end != std::string::npos) {
      auto key = ret.substr(pos + 2, end - pos - 2);
      auto colonpos = key.find(':');
      if (colonpos == std::string::npos) {
        auto value = rhs.get_env(key);
        if (!value.empty()) {
          ret.replace(pos, end - pos + 1, value);
        }
      } else {
        auto realkey = key.substr(0, colonpos);
        auto value = rhs.get_env(realkey);
        if (!value.empty()) {
          ret.replace(pos, end - pos + 1, value);
        } else {
          ret.replace(pos, end - pos + 1, key.substr(colonpos + 1));
        }
      }
    }
  }
  return ret;
}

size_t guess_nproc_from_env() {
  constexpr auto keys =
      std::to_array({"NPROC", "OMP_NUM_THREADS", "GSL_NUM_THREADS",
                     "MKL_NUM_THREADS", "JULIA_NUM_THREADS", "TF_NUM_THREADS",
                     "GOMAXPROCS", "SLURM_NTASKS", "SLURM_NPROCS"});
  for (auto &&key : keys) {
    auto value = env_handler::get_instance().get_env(key);
    if (!value.empty()) {
      return std::stoul(value);
    }
  }
  // 0 means let ROOT decide
  return 0;
}