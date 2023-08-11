#pragma once

#include "ROOT/RDF/InterfaceUtils.hxx"
#include <functional>
#include <string>
#include <unordered_map>
#ifndef _COMMON_HEADER_
#define _COMMON_HEADER_
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <nlohmann/json.hpp>

template <typename T> class ProcessNodeT {
public:
  virtual T operator()(ROOT::RDF::RNode) = 0;
  virtual void configure(const nlohmann::json &){};
};

template <typename T> class FactoryT {
public:
  std::unique_ptr<ProcessNodeT<T>> create(const std::string &name) const {
    auto it = creator.find(name);
    if (it == creator.end()) {
      throw std::runtime_error("No creator for type " + name);
    }
    return it->second();
  }

  bool
  registerCreator(const std::string &name,
                  std::function<std::unique_ptr<ProcessNodeT<T>>()> m_creator) {
    this->creator[name] = m_creator;
    return true;
  }

  static FactoryT<T> &instance() {
    static FactoryT<T> instance;
    return instance;
  }

private:
  std::unordered_map<std::string, std::function<std::unique_ptr<ProcessNodeT<T>>()>> creator;
  FactoryT() = default;
  FactoryT(const FactoryT &) = delete;
  FactoryT(FactoryT &&) = delete;
};

using ProcessNodeI = ProcessNodeT<ROOT::RDF::RNode>;
using ProcessNodeFactory = FactoryT<ROOT::RDF::RNode>;
using NormalizeI = ProcessNodeT<double>;
using NormalizeFactory = FactoryT<double>;

#define REGISTER_PROCESS_NODE(type)                                            \
  namespace {                                                                  \
  static bool type##registered =                                               \
      ProcessNodeFactory::instance().registerCreator(                          \
          #type, [] { return std::make_unique<type>(); });                     \
  }

#define REGISTER_NORMALIZE(type)                                               \
  namespace {                                                                  \
  static bool type##registered = NormalizeFactory::instance().registerCreator( \
      #type, [] { return std::make_unique<type>(); });                         \
  }

#endif