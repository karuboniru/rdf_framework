#include "wildcard_expand.h"
#include <TSystem.h>
#include <regex>

std::vector<std::string> expand_wildcard_path(std::string_view input) {
  std::vector<std::string> ret;
  auto pos_star = input.find("*");
  if (pos_star != std::string::npos) {
    auto slash_before_star = input.rfind("/", pos_star);
    auto slash_after_star = input.find("/", pos_star);
    auto dir = std::string{input.substr(0, slash_before_star)};
    auto pattern = std::string{input.substr(
        slash_before_star + 1, (slash_after_star - slash_before_star - 1))};
    // convert * to .*
    pattern = std::regex_replace(pattern, std::regex("\\*"), ".*");
    auto dir_entry = gSystem->OpenDirectory(dir.c_str());
    const char *ent;
    while ((ent = gSystem->GetDirEntry(dir_entry))) {
      std::string entry = ent;
      if (std::regex_match(entry, std::regex(pattern))) {
        entry = dir + "/" + entry;
        if (slash_after_star != std::string::npos) {
          auto following_path = std::string{input.substr(slash_after_star + 1)};
          auto thispath = entry + "/" + following_path;
          if (thispath.find("*") != std::string::npos) {
            auto subpaths = expand_wildcard_path(thispath);
            ret.insert(ret.end(), subpaths.begin(), subpaths.end());
          } else if (!gSystem->AccessPathName(thispath.c_str())) {
            ret.push_back(thispath);
          }
        } else {
          ret.push_back(entry);
        }
      }
    }
    gSystem->FreeDirectory(dir_entry);
  } else {
    ret.push_back(std::string{input});
  }
  return ret;
}