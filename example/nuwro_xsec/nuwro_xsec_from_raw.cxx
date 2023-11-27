#include "common.h"

#include <event1.h>

class xsecplot : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Define("E", [](event &e) { return e.in[0].t / 1000.; }, {"e"});
  }
};

REGISTER_PROCESS_NODE(xsecplot);

class ccqel : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Filter([](event &e) { return e.flag.cc && e.flag.qel; }, {"e"});
  }
};

REGISTER_PROCESS_NODE(ccqel);

class ccres : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Filter([](event &e) { return e.flag.cc && e.flag.res; }, {"e"});
  }
};

REGISTER_PROCESS_NODE(ccres);

class ccmec : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Filter([](event &e) { return e.flag.cc && e.flag.mec; }, {"e"});
  }
};

REGISTER_PROCESS_NODE(ccmec);

class ccdis : public ProcessNodeI {
public:
  ROOT::RDF::RNode operator()(ROOT::RDF::RNode df) override {
    return df.Filter([](event &e) { return e.flag.cc && e.flag.dis; }, {"e"});
  }
};

REGISTER_PROCESS_NODE(ccdis);