/**
   @file
   @brief Utility classes for clue-redex tests.
*/

#pragma once

#include "RedexTest.h"


/**
   @brief Test fixture class for clue-redex unit tests.

   This class initializes the @ref RedexContext.
*/
struct ClueTest : public RedexTest {

  /// Construct the fixture
  ClueTest() { }


  /// Destroy the fixture
  ~ClueTest() { }
};



