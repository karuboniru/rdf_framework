#include "dlopen_wrap.h"
#include <iostream>

dlopen_wrap::dlopen_wrap(std::string_view filename, int flag) {
  handle = dlopen(filename.data(), flag);
  if (!handle) {
    std::cerr << "Cannot open library: " << dlerror() << '\n';
    throw std::runtime_error("Cannot open library");
  }
}

dlopen_wrap::~dlopen_wrap() { dlclose(handle); }

void *dlopen_wrap::get_handle() { return handle; }

void *dlopen_wrap::get_symbol(const char *symbol) {
  return dlsym(handle, symbol);
}