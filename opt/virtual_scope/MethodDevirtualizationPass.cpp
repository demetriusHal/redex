/**
 * Copyright (c) 2017-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "MethodDevirtualizationPass.h"
#include "DexUtil.h"
#include "MethodDevirtualizer.h"

#include "IRCode.h"

#include "Walkers.h"


void MethodDevirtualizationPass::run_pass(DexStoresVector& stores,
                                          ConfigFiles&,
                                          PassManager& manager) {
  MethodDevirtualizer devirt(m_staticize_vmethods_not_using_this,
                             m_staticize_vmethods_using_this,
                             m_staticize_dmethods_not_using_this,
                             m_staticize_dmethods_using_this,
                             m_ignore_keep);
  const auto scope = build_class_scope(stores);
  const auto metrics = devirt.devirtualize_methods(scope);
  manager.incr_metric("num_staticized_methods_drop_this",
                      metrics.num_methods_not_using_this);
  manager.incr_metric("num_staticized_methods_keep_this",
                      metrics.num_methods_using_this);
  manager.incr_metric("num_virtual_calls_converted", metrics.num_virtual_calls);
  manager.incr_metric("num_direct_calls_converted", metrics.num_direct_calls);
  manager.incr_metric("num_super_calls_converted", metrics.num_super_calls);
}

//static MethodDevirtualizationPass s_pass;

#include <map>
#include <vector>

//format delimeter ,
// <invocations site> % <target function>
// assuming % should appear nowhere

static const char __srgfmtdel = '%';

struct MethodInsInfo {
  std::pair <std::string, int> info;
};

struct MethodScopeInfo {
  std::map<std::string, std::vector< std::pair<int, SrgFunction*> >* > info;
};

struct ClassScopeInfo {
  std::map <SrgMethodProto, MethodScopeInfo*> info;
};

bool operator<(const SrgMethodProto& l, const SrgMethodProto& r) {
  if (l.name.compare(r.name) != 0)
    return l.name.compare(r.name) < 0;
  if (l.rtype.compare(r.rtype) != 0)
    return l.rtype.compare(r.rtype) < 0;
  if (l.args->size() != r.args->size())

    return l.args->size() < r.args->size() ;
  for (size_t i=0; i < l.args->size(); i++)
    if (l.args->at(i).compare(r.args->at(i)) != 0)
      return l.args->at(i).compare(r.args->at(i)) < 0;

  return false;
}


bool operator==(const SrgMethodProto& l, const SrgMethodProto& r) {
  if (l.name.compare(r.name) != 0)
    return false;
  if (l.rtype.compare(r.rtype))
    return false;

  if (l.args->size() != r.args->size())
    return false;
  for (size_t i=0; i < l.args->size(); i++)
    if (l.args->at(i).compare(r.args->at(i)))
      return false;

  return true;
}



static void read_instr(std::map <std::string, ClassScopeInfo*> *devi,
  std::string fileName) {

  std::ifstream file;
  std::string line;
  file.open(fileName);
  /*parse the files contents */
  if (!file.good()) {
    std::cout << "Srg:File not found: " << fileName << std::endl;
    return;
  }
  int lineId = -1;
  int status = 0; //ok
  while (file.good()) {
    lineId++;
    std::getline(file, line, '\n');
    size_t idx;

    idx = line.find(__srgfmtdel);
    std::string invsite = line.substr(0,idx);
    std::string tgmethod = line.substr(idx, line.length() - idx);

    SrgInvocSite* is = new SrgInvocSite(invsite);
    SrgFunction* fun = new SrgFunction(tgmethod);




    //debug
    /*
    std::cout << "-------->" << std::endl;
    std::cout << "`" << cls << "`" << std::endl;
    std::cout << "`" << method << "`" << std::endl;
    std::cout << "`" << callee << "`" << std::endl;
    std::cout << "`" << ith << "`" << std::endl;
    callId = std::stoi(ith);
    */
    // insert our stuff in our data structure

    //check if the map doesnt already exists , tedious but necessary

    std::string cls = is->gcls();
    SrgMethodProto& method = is->gmethod();
    std::string callee = is->gcallee();
    int n = is->gn();


    if (devi->find(cls) == devi->end())
      devi->operator[](cls) = new ClassScopeInfo;
    else
      std::cout << "Found " <<  std::endl;
    if (devi->operator[](cls)->info.find(method) == devi->operator[](cls)->info.end())
      devi->operator[](cls)->info.operator[](method) = new MethodScopeInfo;
    else
      std::cout << "Found " << std::endl;

    MethodScopeInfo* msi = devi->operator[](cls)->info.at(method);
    if (msi->info.find(callee) == msi->info.end())
      msi->info.operator[](callee) = new std::vector<std::pair<int, SrgFunction*> >() ;
    else
      std::cout << "Adding " << n << std::endl;
    msi->info.operator[](callee)->push_back(std::pair<int, SrgFunction* >(n, fun));

    std::cout << "Ok" << std::endl;

  } // end while()

  if (status == -1) {
    std::cerr << "Srg:Parse Error at line :" << lineId << std::endl;
  }


}


void devirt_targets(std::vector<DexClass*>& classes, std::map <std::string, ClassScopeInfo*> devi) {

  int i = 0;
  for (auto const &cls : classes) {
    std::string clsname = cls->get_name()->str();

    if (devi.find(clsname) == devi.end())
      continue;

    auto ccls = devi[clsname]->info;

    //have to check every type of method
    std::vector<DexMethod*>& dmeths= cls->get_dmethods();
    for (auto const& dmeth: dmeths) {
      std::cout << dmeth->get_name()->c_str() << std::endl;

    }


    std::vector<DexMethod*>& vmeths= cls->get_vmethods();
    for (auto const& vmeth: vmeths) {
      std::cout << vmeth->get_name()->c_str() << std::endl;
      SrgMethodProto srgmt(vmeth);
      if (ccls.find(srgmt) == ccls.end())
        continue;
      else {
        auto lfm = ccls[srgmt]->info;
        auto irc = vmeth->get_code();
        auto ircit = InstructionIterable(irc);
        for (auto it1 = ircit.begin(); it1 != ircit.end(); it1++) {
          IRInstruction* insn = it1->insn;
          if (!insn->has_method()) {
            continue;
          }
          else {
            std::cout << "Found invocation!" << std::endl;
            DexMethodRef *m = insn->get_method();
            std::cout << "--> " <<type_class(m->get_class())->get_super_class()->get_name()->str() << std::endl;;
            std::cout << m->get_name()->str() << std::endl;
            if (lfm.find(m->get_name()->str()) != lfm.end()) {
              auto v = lfm[(m->get_name()->str())];
              std::cout << "Found target!" << std::endl;
              for (size_t i=0; i < v->size(); i++) {
                if (v->at(i).first == 0) {
                  insn->set_opcode(OPCODE_INVOKE_STATIC);
                  IRInstruction* check_cast = new IRInstruction(OPCODE_CHECK_CAST);
                  check_cast->set_type(type_class(m->get_class())->get_super_class())->set_src(0, 1);

                  MethodItemEntry *newinstr = new MethodItemEntry(check_cast);
                  //TODO possibly makes this more efficient
                  // Find connection between irc iterator
                  // and IRCODE.h iterators ...
                  auto it2 = irc->iterator_to(*it1);
                  irc->insert_before(it2, *newinstr);
                  irc->insert_before(it2,
                    *(new MethodItemEntry((new IRInstruction(IOPCODE_MOVE_RESULT_PSEUDO_OBJECT))->set_dest(1))));
                  delete v->at(i).second;
                  v->erase(v->begin()+i);
                }
                  else
                    v->at(i).first = v->at(i).first - 1;
              }
            }
          }
        }
      }

    }

  }
}

SrgMethodProto::SrgMethodProto(const DexMethodRef *dm) {

  this->name = dm->get_name()->str();
  this->rtype = dm->get_proto()->get_rtype()->get_name()->str();
  this->args = new std::vector<std::string> ();

  for (size_t i=0; i < dm->get_proto()->get_args()->get_type_list().size(); i++) {
    this->args->push_back(dm->get_proto()->get_args()->get_type_list()[i]->get_name()->str());
  }
}

bool SrgMethodProto::compare(const DexMethodRef* dm) {

  if (this->name.compare(dm->get_name()->str()))
    return false;
  if (this->rtype.compare(dm->get_proto()->get_rtype()->get_name()->str()))
    return false;
  if (this->args->size() != dm->get_proto()->get_args()->get_type_list().size())
    return false;

  for (size_t i=0; i < this->args->size();i++) {

    if (this->args->at(i) !=
    dm->get_proto()->get_args()->get_type_list()[i]->get_name()->str())

      return false;
  }
  return true;

}

void SrgInvocSite::get_cls_name(std::string str, int& ptr) {

  ptr++;
  disc_empty(str, ptr);
  int i =0;
  while (str[ptr+i] != ':' && !isspace(str[ptr+i]))
  	i++;

  this->cls = str.substr(ptr, i);
  ptr += i;
  disc_empty(str, ptr);
  if (str[ptr] != ':')
  	std::cerr << "Problem." << std::endl;
  ptr++;

  std::cout << cls << std::endl;
}

void SrgFunction::get_cls_name(std::string str, int& ptr) {

  ptr++;
  disc_empty(str, ptr);
  int i =0;
  while (str[ptr+i] != ':' && !isspace(str[ptr+i]))
  	i++;

  this->cls = str.substr(ptr, i);
  ptr += i;
  disc_empty(str, ptr);
  if (str[ptr] != ':')
  	std::cerr << "Problem." << std::endl;
  ptr++;

  std::cout << cls << std::endl;
}

void SrgInvocSite::get_method_p(std::string str, int& ptr) {

  disc_empty(str, ptr);
  int i = 0;

  while (!isspace(str[ptr+i]))
  	i++;
  std::string rtype = str.substr(ptr, i);

  ptr += i;
  disc_empty(str, ptr);
  i = 0;

  while (!isspace(str[ptr+i]) && str[ptr+i] != '(')
  	i++;
  std::string name = str.substr(ptr, i);
  ptr += i;

  disc_empty(str, ptr);

  if (str[ptr] != '(')
  	std::cerr << "Problem." << std::endl;

  ptr++;

  std::vector<std::string> *vptr = new std::vector<std::string>();

  //now read the arguments
  disc_empty(str, ptr);

  // FIX THIS !
  int first = 0;
  while (str[ptr] != ')') {

  	if (first) {
    ptr++;
    disc_empty(str,ptr);
  	}
  	else
    first = 1;

  	i = 0;

  	while (!isspace(str[ptr+i]) && str[ptr+i] != ',' && str[ptr+i] != ')')
    i++;
  	vptr->push_back(std::string(str.substr(ptr, i)));
  	ptr += i;
  	disc_empty(str, ptr);

  }
  ptr++;

  this->method.args = vptr;
  this->method.rtype = rtype;
  this->method.name = name;//this->cls + '.' + name;
  this->method.cls = this->cls;

  std::cout << "`" << rtype << "`" << std::endl;
  std::cout << "`" << this->method.name << "`" << std::endl;
  for (size_t i=0; i < vptr->size(); i++)
    std::cout << "`" << vptr->at(i) << "`" << std::endl;


  if (str[ptr] != '>')
  	std::cerr << "Problem." << std::endl;
  ptr++;

}

void SrgFunction::get_method_p(std::string str, int& ptr) {

  disc_empty(str, ptr);
  int i = 0;

  while (!isspace(str[ptr+i]))
  	i++;
  std::string rtype = str.substr(ptr, i);

  ptr += i;
  disc_empty(str, ptr);
  i = 0;

  while (!isspace(str[ptr+i]) && str[ptr+i] != '(')
  	i++;
  std::string name = str.substr(ptr, i);
  ptr += i;

  disc_empty(str, ptr);

  if (str[ptr] != '(')
  	std::cerr << "Problem." << std::endl;

  ptr++;

  std::vector<std::string> *vptr = new std::vector<std::string>();

  //now read the arguments
  disc_empty(str, ptr);

  // FIX THIS !
  int first = 0;
  while (str[ptr] != ')') {

  	if (first) {
    ptr++;
    disc_empty(str,ptr);
  	}
  	else
    first = 1;

  	i = 0;

  	while (!isspace(str[ptr+i]) && str[ptr+i] != ',' && str[ptr+i] != ')')
    i++;
  	vptr->push_back(std::string(str.substr(ptr, i)));
  	ptr += i;
  	disc_empty(str, ptr);

  }
  ptr++;

  this->method.args = vptr;
  this->method.rtype = rtype;
  this->method.name = name;//this->cls + '.' + name;
  this->method.cls = this->cls;

  std::cout << "`" << rtype << "`" << std::endl;
  std::cout << "`" << this->method.name << "`" << std::endl;
  for (size_t i=0; i < vptr->size(); i++)
  std::cout << "`" << vptr->at(i) << "`" << std::endl;


  if (str[ptr] != '>')
  	std::cerr << "Problem." << std::endl;
  ptr++;

}

void SrgInvocSite::get_callee(std::string str, int& ptr) {
  if (str[ptr] != '/')
    std::cerr << "Problem." << std::endl;

  ptr++;
  disc_empty(str, ptr);
  int i =0;
  while (str[ptr+i] != '/' && !isspace(str[ptr+i]))
    i++;

  this->callee = str.substr(ptr, i);
  ptr += i;
  disc_empty(str, ptr);
  if (str[ptr] != '/')
    std::cerr << "Problem." << std::endl;
  ptr++;

  std::cout << this->callee << std::endl;

}



void SurgeonDevirtPass::run_pass(DexStoresVector& stores, ConfigFiles& ConfigFiles,
                                PassManager& manager) {

  /* Devirtualise invokations written from a file*/
  /* filename should be declared at the config file*/

  std::cout << "SurgeonDevirtPass started..." << std::endl;
  //TODO file shouldn't be hard coded ?
  std::string fileName("inputfuns.csv");
  std::map <std::string, ClassScopeInfo*> devirt_instrs;
  /*the struct above are the input files data */

  read_instr(&devirt_instrs, fileName);

  Scope scope = build_class_scope(stores);

  devirt_targets(scope, devirt_instrs);
  //TODO clean up
  return ;

}





static SurgeonDevirtPass s_pass;
