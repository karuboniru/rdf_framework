#pragma once

#ifndef _COMMON_HEADER_
#define _COMMON_HEADER_
#include "ROOT/RDF/InterfaceUtils.hxx"
#include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

template <typename T> class ProcessNodeT {
public:
  virtual T operator()(ROOT::RDF::RNode) = 0;
  virtual void configure(const nlohmann::json &) {}
  ProcessNodeT() = default;
  ProcessNodeT(const ProcessNodeT &) = default;
  ProcessNodeT(ProcessNodeT &&) = default;
  ProcessNodeT &operator=(const ProcessNodeT &) = default;
  ProcessNodeT &operator=(ProcessNodeT &&) = default;
  virtual ~ProcessNodeT() = default;
};

template <typename T> class FactoryT {
public:
  std::unique_ptr<ProcessNodeT<T>> create(const std::string &name) const {
    auto it = creator.find(name);
    if (it == creator.end()) {
      return nullptr;
    }
    return it->second();
  }

  bool
  registerCreator(const std::string &name,
                  std::function<std::unique_ptr<ProcessNodeT<T>>()> m_creator) {
    this->creator[name] = m_creator;
    return true;
  }

  void unregisterCreator(const std::string &name) { creator.erase(name); }

  static FactoryT<T> &instance() {
    static FactoryT<T> instance;
    return instance;
  }

  FactoryT(const FactoryT &) = delete;
  FactoryT(FactoryT &&) = delete;
  FactoryT &operator=(const FactoryT &) = delete;
  FactoryT &operator=(FactoryT &&) = delete;
  ~FactoryT() = default;

private:
  std::unordered_map<std::string,
                     std::function<std::unique_ptr<ProcessNodeT<T>>()>>
      creator;
  FactoryT() = default;
};

using ProcessNodeI = ProcessNodeT<ROOT::RDF::RNode>;
using ProcessNodeFactory = FactoryT<ROOT::RDF::RNode>;
using NormalizeI = ProcessNodeT<double>;
using NormalizeFactory = FactoryT<double>;

template <typename TFactory, typename Handler> class RegistrationGuard {
public:
  RegistrationGuard(const std::string &name) : name(name) {
    TFactory::instance().registerCreator(
        name, []() { return std::make_unique<Handler>(); });
  }
  ~RegistrationGuard() { TFactory::instance().unregisterCreator(name); }

  RegistrationGuard(const RegistrationGuard &) = delete;
  RegistrationGuard(RegistrationGuard &&) = delete;
  RegistrationGuard &operator=(const RegistrationGuard &) = delete;
  RegistrationGuard &operator=(RegistrationGuard &&) = delete;

private:
  std::string name;
};

#define REGISTER_PROCESS_NODE(type)                                            \
  namespace {                                                                  \
  const static RegistrationGuard<ProcessNodeFactory, type> type##registered{   \
      #type};                                                                  \
  }

#define REGISTER_NORMALIZE(type)                                               \
  namespace {                                                                  \
  const static RegistrationGuard<NormalizeFactory, type> type##registered{     \
      #type};                                                                  \
  }

#endif