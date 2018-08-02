/**
   @file
   @brief Utility classes for clue-redex tests.
*/

#pragma once

#include <vector>
#include "DexStore.h"
#include "RedexTest.h"


/**
   @brief Test fixture class for clue-redex unit tests.

   This class initializes the @ref RedexContext.
*/
struct ClueTest : public RedexTest {

  /// Construct the fixture
  ClueTest() { }

  /// List of DexStore stores for the test context
  std::vector<DexStore> stores;

  /**
  	@brief Load classes from a .dex file

  	The method will load classes found in the `dexfile`.
  	The dexfile will be added to the stores. 

  	In a multidex scenario, dex files should be loaded in the order that they
  	would be named in the .apk file, i.e., first the "classes" file, then the
  	"classes2" file, etc.
    */
  DexClasses& load_classes(const std::string& dexfile);

  /// Destroy the fixture
  ~ClueTest() { }
};



