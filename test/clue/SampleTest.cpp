/**
  @file 
  @brief Sample test file.
  
  This is a file that you can copy in order to create your own test suite.

 */

#include <cstdint>
#include <cstdio>
#include <gtest/gtest.h>

#include "ClueTest.h"

/**
  @fn SampleTests_sample1_Test
  @brief A sample test case for libredex.

  The purpose of this test function is to give a guide into writing your own test functions.
  In this example, `SampleTests` is the name of the test suite and `sample1` is the name of
  the test case.
  */
TEST(SampleTests, sample1) {

  // This test is a self-contained routine, that does nothing
  ASSERT_EQ(1,1);

}


/**
  @fn ClueTest_sample2_Test
  @brief A sample test case for clue-redex.

  The purpose of this test function is to give a guide into writing your own test functions.
  In this example, the test uses fixture @ref ClueTest  and `sample2` is the name of
  the test case.
  */
TEST_F(ClueTest, sample2) {

  // This test is a self-contained routine, that does nothing
  ASSERT_EQ(1,1);
  
  ASSERT_EQ(0, stores.size());

  DexClasses& classes = load_classes("sample-classes");

  ASSERT_EQ(1, classes.size());
}
