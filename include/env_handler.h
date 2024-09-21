#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
class env_handler {
public:
  static env_handler &get_instance();
  std::string get_env(const std::string_view key);
  void set_env(const std::string_view key, const std::string_view value);

private:
  env_handler();
  std::unordered_map<std::string, std::string> envs{};
  env_handler(const env_handler &other) = delete;
  env_handler &operator=(const env_handler &other) = delete;
};

std::string operator|(const std::string_view lhs, env_handler &rhs);

size_t guess_nproc_from_env();
