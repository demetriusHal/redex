#pragma once

#include "PassManager.h"
#include "DexClass.h"

extern int64_t cutoff;

class RemoverPass : public Pass {
 public:
  RemoverPass() : Pass("RemoverPass") {}

  virtual void run_pass(DexStoresVector&, ConfigFiles&, PassManager&) override;
};
