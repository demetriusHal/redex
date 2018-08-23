/**
 * Copyright (c) 2017-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "Pass.h"

class SurgeonDevirtPass : public Pass {
  public:
    SurgeonDevirtPass() : Pass("SurgeonDevirtPass"){}

    virtual void run_pass(DexStoresVector&, ConfigFiles&, PassManager&) override;

  //private:

};

class MethodDevirtualizationPass : public Pass {
 public:
  MethodDevirtualizationPass() : Pass("MethodDevirtualizationPass") {}

  virtual void configure_pass(const PassConfig& pc) override {
    pc.get("staticize_vmethods_not_using_this",
           true,
           m_staticize_vmethods_not_using_this);
    pc.get("staticize_vmethods_using_this",
           false,
           m_staticize_vmethods_using_this);
    pc.get("staticize_dmethods_not_using_this",
           true,
           m_staticize_dmethods_not_using_this);
    pc.get("staticize_dmethods_using_this",
           false,
           m_staticize_dmethods_using_this);
    pc.get("ignore_keep", false, m_ignore_keep);
  }

  virtual void run_pass(DexStoresVector&, ConfigFiles&, PassManager&) override;

 private:
  bool m_staticize_vmethods_not_using_this;
  bool m_staticize_vmethods_using_this;
  bool m_staticize_dmethods_not_using_this;
  bool m_staticize_dmethods_using_this;
  bool m_ignore_keep;
};

// additions ...

class SrgMethodProto {

	public:
	std::string name;
	std::string rtype;
  std::string cls;
	std::vector<std::string> *args;

	SrgMethodProto(std::string name, std::vector<std::string> * vptr, std::string rtype):
	rtype(rtype), args(vptr) { }

	SrgMethodProto()
	{}
  SrgMethodProto(const DexMethodRef *dm);

  bool compare(const DexMethodRef* );

	~SrgMethodProto() {
  delete args;
	}
};
//expect undefined behavior if the input isn't corret
class SrgInvocSite {

	std::string cls;   //class name
	SrgMethodProto method;	//method prototype
	std::string callee;  //callee name
	int n;    	//nth function call whatsoever


	public:
	std::string& gcls(void) {return cls;}
	SrgMethodProto& gmethod(void) {return method;}
	std::string& gcallee(void) {return callee;}
	int gn(void) {return n;}


	SrgInvocSite(std::string str) {
    int sptr = 0;
    get_cls_name(str, sptr);
    get_method_p(str, sptr);
    get_callee(str, sptr);
    this-> n = std::stoi(str.substr(sptr, str.length()-sptr));
    //std::cout << "===============" << std::endl;
  }

  private:
  void get_cls_name(std::string str, int& ptr);
  void get_method_p(std::string str, int& ptr);
  void get_callee(std::string str, int& ptr);

  void disc_empty(std::string str, int& ptr) {
    while (isspace(str[ptr]))
      ptr++;
  }

};

class SrgFunction
{
	std::string cls; 		//class name
	SrgMethodProto method;	//method prototype


	public:
	std::string& gcls(void) {return cls;}
	SrgMethodProto& gmethod(void) {return method;}

	SrgFunction(std::string str) {

		int sptr = 0;
		get_cls_name(str, sptr);
		get_method_p(str, sptr);

	}

  private:
  void get_cls_name(std::string str, int& ptr);
  void get_method_p(std::string str, int& ptr);

  void disc_empty(std::string str, int& ptr) {
    while (isspace(str[ptr]))
      ptr++;
  }

};
