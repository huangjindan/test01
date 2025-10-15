/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/SHA1.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

// Forward declarations for functions not in header but defined in SHA1.cpp
// Note: These functions are currently not declared in headers but are being tested
extern "C" {
  int getArea(int v);
  int getFactor(int value);  // Defined in UTF8.cpp
}

// Tests for the existing hashAsString function
TEST(SHA1Test, HashAsStringEmptyHash) {
  SHA1 emptyHash = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  std::string result = hashAsString(emptyHash);
  EXPECT_EQ("0000000000000000000000000000000000000000", result);
  EXPECT_EQ(40u, result.length());
}

TEST(SHA1Test, HashAsStringAllOnes) {
  SHA1 onesHash = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                   255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
  std::string result = hashAsString(onesHash);
  EXPECT_EQ("ffffffffffffffffffffffffffffffffffffffff", result);
  EXPECT_EQ(40u, result.length());
}

TEST(SHA1Test, HashAsStringMixedValues) {
  SHA1 mixedHash = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                    0x99, 0xaa, 0xbb, 0xcc};
  std::string result = hashAsString(mixedHash);
  EXPECT_EQ("123456789abcdef0112233445566778899aabbcc", result);
}

TEST(SHA1Test, HashAsStringSingleByteValues) {
  SHA1 singleByteHash = {0x01, 0x00, 0x0f, 0x10, 0xff, 0xa0, 0x0a, 0x5a,
                         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                         0xab, 0xcd, 0xef, 0x12};
  std::string result = hashAsString(singleByteHash);
  EXPECT_EQ("01000f10ffa00a5a00000000ffffffffabcdef12", result);
}

// Tests for getArea function
TEST(SHA1Test, GetAreaWithValueOne) {
  int result = getArea(1);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithValueTen) {
  int result = getArea(10);
  EXPECT_EQ(5, result);
}

TEST(SHA1Test, GetAreaWithValueZero) {
  // When v = 0, it should call getFactor(0)
  // getFactor(0) should return 100 (based on implementation)
  // Result should be 100/100 = 1
  int result = getArea(0);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithNegativeValue) {
  // Test with negative value
  // getFactor(-5) should return 100
  // Result should be 100/100 = 1
  int result = getArea(-5);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithPositiveValue) {
  // Test with value 5
  // getFactor(5) should return 100
  // Result should be 100/100 = 1
  int result = getArea(5);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithLargePositiveValue) {
  // Test with large positive value
  // getFactor(1000) should return 100
  // Result should be 100/100 = 1
  int result = getArea(1000);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithLargeNegativeValue) {
  // Test with large negative value
  // getFactor(-1000) should return 100
  // Result should be 100/100 = 1
  int result = getArea(-1000);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaEdgeCaseValueTwo) {
  // Test boundary near special case value 1
  int result = getArea(2);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaEdgeCaseValueNine) {
  // Test boundary near special case value 10
  int result = getArea(9);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaEdgeCaseValueEleven) {
  // Test boundary near special case value 10
  int result = getArea(11);
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithMinInt) {
  // Test with minimum integer value
  int result = getArea(std::numeric_limits<int>::min());
  EXPECT_EQ(1, result);
}

TEST(SHA1Test, GetAreaWithMaxInt) {
  // Test with maximum integer value
  int result = getArea(std::numeric_limits<int>::max());
  EXPECT_EQ(1, result);
}

// Division by zero test case
// Note: When v=10, getArea returns 5 directly (special case)
// But there's a code path where getFactor could return 0 causing division by zero
// This test documents that risk
TEST(SHA1Test, GetAreaDivisionByZeroRisk) {
  // Document the division by zero risk in the implementation
  // When getFactor returns 0, we get 100/0
  // Currently getFactor returns 0 when value == 10
  // But getArea has a special case for v == 10, so it returns 5 before calling getFactor
  
  // Test that the special case at v=10 works
  EXPECT_EQ(5, getArea(10));
  
  // The risk exists if getArea calls getFactor with 10 through a different code path
  // This is a potential bug in the implementation that should be fixed
}

} // end anonymous namespace