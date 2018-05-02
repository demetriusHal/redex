/* Reads a list of method names from a file and removes them from
   their containing classes. The input file is assumed to contain one
   JVM-style method descriptor per line (as in names contained in
   .smali files). Compared to opt/simpleinline/Deleter, Remover does
   not run checks (like is_concrete() or can_delete()). */

#include <fstream>
#include "Remover.h"
#include "DexUtil.h"

typedef std::set<std::pair<DexClass*, DexMethod*> > CMethods;
typedef std::set<std::pair<std::string, std::string>> CMethodStrs;

// The name of the methods file is currently hardcoded.
std::string fName = "methods_to_remove.txt";

/* Given a DexMethod, generates its JVM descriptor string. */
std::string gen_JVM_descriptor(DexMethod* m) {
  // Build result by concatenating method name and signature.
  std::stringstream ss;
  ss << m->get_name()->c_str();
  ss << show(m->get_proto());
  return ss.str();
}

std::string gen_method_desc(const std::string clsQName, DexMethod* m) {
  std::stringstream ss;
  ss << clsQName;
  ss << gen_JVM_descriptor(m);
  return ss.str();
}

void check_method(DexClass* cls, std::string& clsQName, DexMethod* method,
                  const CMethodStrs cMethodStrs, CMethods* methods,
                  int64_t* counter) {
  std::string descriptor = gen_JVM_descriptor(method);
  auto const m = std::make_pair(clsQName, descriptor);

  if (cMethodStrs.find(m) != cMethodStrs.end()) {
    auto const p = std::make_pair(cls, method);
    std::string meth_desc = gen_method_desc(clsQName, method);
    if (methods->find(p) == methods->end()) {
      methods->insert(p);
      (*counter)++;
    }
  }
}

/* Given a list of class/method pairs, finds all candidates for removal,
   then proceeds to delete them. */
void remove_methods(std::vector<DexClass*>& classes, CMethodStrs cMethodStrs) {
  CMethods methods;
  std::cout << "Collecting methods..." << std::endl;
  int64_t markedForRemoval = 0;
  for (auto const& cls : classes) {
    std::string clsQName = cls->get_name()->c_str();
    for (auto const& dm : cls->get_dmethods()) {
      check_method(cls, clsQName, dm, cMethodStrs, &methods, &markedForRemoval);
    }
    for (auto const& vm : cls->get_vmethods()) {
      check_method(cls, clsQName, vm, cMethodStrs, &methods, &markedForRemoval);
    }
  }

  // Finally remove marked methods.
  for (auto const& cm : methods) {
    cm.first->remove_method(cm.second);
    std::cout << "Removing " << gen_method_desc(cm.first->get_name()->c_str(), cm.second) << std::endl;
  }

  std::cout << "Removed " << methods.size() << " methods (from " << markedForRemoval << " marked)." << std::endl;
}

void RemoverPass::run_pass(DexStoresVector& dexen, ConfigFiles& cfg, PassManager& mgr) {
  CMethodStrs cMethodStrs;
  std::ifstream cFile;
  std::string cName, mName;

  cFile.open(fName);
  if (!cFile.good()) {
    return;
  }
  std::cout << "Reading list of methods to remove from " << fName << "..." << std::endl;
  while (cFile.good()) {
    std::getline(cFile, cName, ':');
    std::getline(cFile, mName, '\n');
    if (!cName.empty() && !mName.empty()) {
      cMethodStrs.insert(std::make_pair(cName, mName));
    }
  }
  cFile.close();
  std::cout << "Read " << cMethodStrs.size() << " methods." << std::endl;

  Scope scope = build_class_scope(dexen);
  std::cout << "Method removal pass" << std::endl;
  remove_methods(scope, cMethodStrs);
}

static RemoverPass s_pass;
