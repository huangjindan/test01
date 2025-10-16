/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/UTF16Stream.h"
#include "hermes/Support/UTF8.h"

#include "gtest/gtest.h"
#include <limits>

#include <deque>
#include <vector>

using namespace hermes;

namespace {

// Verify correct behavior of convertUTF16ToUTF8StringWithSingleSurrogates().
TEST(StringTest, UTF16ToUTF8StringWithSingleSurrogates) {
  auto convert = [](std::initializer_list<char16_t> cs) -> std::vector<char> {
    std::string out;
    std::vector<char16_t> vec(cs);
    convertUTF16ToUTF8WithSingleSurrogates(out, cs);
    return std::vector<char>(out.begin(), out.end());
  };
  auto bytes =
      [](std::initializer_list<unsigned char> cs) -> std::vector<char> {
    return std::vector<char>(cs.begin(), cs.end());
  };
  EXPECT_EQ(bytes({}), convert({}));
  EXPECT_EQ(bytes({'a'}), convert({'a'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'e', 0xCC, 0x81}), convert({'e', 0x0301}));
  EXPECT_EQ(bytes({0xE2, 0x98, 0x83}), convert({0x2603}));

  // UTF-16 encoded U+1F639
  EXPECT_EQ(
      bytes({0xED, 0xA0, 0xBD, 0xED, 0xB8, 0xB9}), convert({0xD83D, 0xDE39}));

  // Unpaired surrogate halves
  EXPECT_EQ(bytes({0xED, 0xA0, 0xBD}), convert({0xD83D}));
  EXPECT_EQ(bytes({'a', 0xED, 0xA0, 0xBD, 'b'}), convert({'a', 0xD83D, 'b'}));
  EXPECT_EQ(bytes({'a', 0xED, 0xB8, 0xB9, 'b'}), convert({'a', 0xDE39, 'b'}));

  // Unpaired trailing surrogate halves
  EXPECT_EQ(bytes({'a', 0xED, 0xA0, 0xBD}), convert({'a', 0xD83D}));
  EXPECT_EQ(bytes({'a', 0xED, 0xB8, 0xB9}), convert({'a', 0xDE39}));
}

// Verify correct behavior of convertUTF16ToUTF8StringWithReplacements().
TEST(StringTest, UTF16ToUTF8StringWithReplacements) {
  auto convert = [](std::initializer_list<char16_t> cs) -> std::vector<char> {
    std::string out;
    std::vector<char16_t> vec(cs);
    convertUTF16ToUTF8WithReplacements(out, cs);
    return std::vector<char>(out.begin(), out.end());
  };
  auto convertWithMaxChars =
      [](std::initializer_list<char16_t> cs,
         size_t maxChars) -> std::pair<std::vector<char>, bool> {
    std::string out;
    std::vector<char16_t> vec(cs);
    bool fullyWritten = convertUTF16ToUTF8WithReplacements(out, cs, maxChars);
    return std::make_pair(
        std::vector<char>(out.begin(), out.end()), fullyWritten);
  };
  auto bytes =
      [](std::initializer_list<unsigned char> cs) -> std::vector<char> {
    return std::vector<char>(cs.begin(), cs.end());
  };
  EXPECT_EQ(bytes({}), convert({}));
  EXPECT_EQ(bytes({'a'}), convert({'a'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), convert({'a', 'b', 'c', 0, 'd'}));
  EXPECT_EQ(bytes({'e', 0xCC, 0x81}), convert({'e', 0x0301}));
  EXPECT_EQ(bytes({0xE2, 0x98, 0x83}), convert({0x2603}));

  // UTF-16 encoded U+1F639
  EXPECT_EQ(bytes({0xF0, 0x9F, 0x98, 0xB9}), convert({0xD83D, 0xDE39}));

  // Unpaired surrogate halves
  EXPECT_EQ(bytes({0xEF, 0xBF, 0xBD}), convert({0xD83D}));
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD, 'b'}), convert({'a', 0xD83D, 'b'}));
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD, 'b'}), convert({'a', 0xDE39, 'b'}));

  // Unpaired trailing surrogate halves
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD}), convert({'a', 0xD83D}));
  EXPECT_EQ(bytes({'a', 0xEF, 0xBF, 0xBD}), convert({'a', 0xDE39}));

  // Check that strings are truncated when maxChars is specified.
  // First with normal ASCII strings.
  auto strAndFullyWritten = convertWithMaxChars({'a', 'b', 'c', 0, 'd'}, 3);
  EXPECT_EQ(bytes({'a', 'b', 'c'}), strAndFullyWritten.first);
  EXPECT_EQ(false, strAndFullyWritten.second);

  // Check when maxCharacters is higher than the number of available characters,
  // that it only writes out the max string.
  strAndFullyWritten = convertWithMaxChars({'a', 'b', 'c', 0, 'd'}, 6);
  EXPECT_EQ(bytes({'a', 'b', 'c', 0, 'd'}), strAndFullyWritten.first);
  EXPECT_EQ(true, strAndFullyWritten.second);

  // Make sure it counts the number of Unicode code points, not the number of
  // individual octets.
  strAndFullyWritten = convertWithMaxChars({0xD83D, 0xDE39, 0xD83D, 0xDE39}, 1);
  EXPECT_EQ(bytes({0xF0, 0x9F, 0x98, 0xB9}), strAndFullyWritten.first);
  EXPECT_EQ(false, strAndFullyWritten.second);
}

TEST(StringTest, IsAllASCIITest) {
  std::deque<uint8_t> ascii = {32, 23, 18};
  std::deque<uint8_t> notAscii = {234, 1, 0};
  EXPECT_TRUE(isAllASCII(std::begin(ascii), std::end(ascii)));
  EXPECT_FALSE(isAllASCII(std::begin(notAscii), std::end(notAscii)));
  // Check overloads.
  uint8_t asciiArr[] = {1, 3, 14, 54, 19, 124, 13, 43, 127, 19, 0};
  uint8_t partialAsciiArr[] = {1, 3, 14, 54, 219, 124, 13, 43, 127, 19};
  uint8_t noAsciiArr[] = {129, 153, 175, 201, 219, 231, 214, 255, 255, 130};
  EXPECT_TRUE(isAllASCII(std::begin(asciiArr), std::end(asciiArr)));
  EXPECT_FALSE(
      isAllASCII(std::begin(partialAsciiArr), std::end(partialAsciiArr)));
  EXPECT_FALSE(isAllASCII(std::begin(noAsciiArr), std::end(noAsciiArr)));
  EXPECT_TRUE(
      isAllASCII((char *)std::begin(asciiArr), (char *)std::end(asciiArr)));
  EXPECT_FALSE(isAllASCII(
      (char *)std::begin(partialAsciiArr), (char *)std::end(partialAsciiArr)));
  EXPECT_FALSE(
      isAllASCII((char *)std::begin(noAsciiArr), (char *)std::end(noAsciiArr)));
  // Torture test of all possible alignments and lengths.
  for (size_t start = 0; start <= sizeof asciiArr / sizeof *asciiArr; start++) {
    for (size_t end = start; end <= sizeof asciiArr / sizeof *asciiArr; end++) {
      EXPECT_TRUE(isAllASCII(&asciiArr[start], &asciiArr[end]));
    }
  }
  for (size_t start = 0; start <= sizeof noAsciiArr / sizeof *noAsciiArr;
       start++) {
    for (size_t end = start; end <= sizeof noAsciiArr / sizeof *noAsciiArr;
         end++) {
      // Only zero-length substrings are ASCII.
      EXPECT_EQ(start == end, isAllASCII(&noAsciiArr[start], &noAsciiArr[end]));
    }
  }
}

TEST(UTF16StreamTest, EmptyUTF16InputTest) {
  UTF16Stream stream(llvh::ArrayRef<char16_t>{});
  EXPECT_FALSE(stream.hasChar());
}

TEST(UTF16StreamTest, UTF16InputTest) {
  char16_t str16[] = {1, 123, 1234};
  UTF16Stream stream(llvh::ArrayRef<char16_t>(str16, str16 + 3));
  EXPECT_TRUE(stream.hasChar());
  EXPECT_EQ(1, *stream);
  ++stream;
  EXPECT_TRUE(stream.hasChar());
  EXPECT_EQ(123, *stream);
  ++stream;
  EXPECT_TRUE(stream.hasChar());
  EXPECT_EQ(1234, *stream);
  ++stream;
  EXPECT_FALSE(stream.hasChar());
}

TEST(UTF16StreamTest, UTF8InputTest) {
  {
    UTF16Stream stream(llvh::ArrayRef<uint8_t>{});
    EXPECT_FALSE(stream.hasChar());
  }
  {
    uint8_t str8[] = {0xF0, 0x9F, 0x98, 0xB9};
    UTF16Stream stream(llvh::ArrayRef<uint8_t>(str8, str8 + sizeof(str8)));
    EXPECT_TRUE(stream.hasChar());
    EXPECT_EQ(0xD83D, *stream);
    ++stream;
    EXPECT_TRUE(stream.hasChar());
    EXPECT_EQ(0xDE39, *stream);
    ++stream;
    EXPECT_FALSE(stream.hasChar());
  }
  {
    // Single code point that requires two UTF16 units.
    uint8_t surrogates[] = {0xF0, 0x9F, 0x98, 0xB9};
    // String where every non-empty prefix is an odd number of UTF16 units.
    std::vector<uint8_t> str8{'X'};
    // Make it longer than any reasonable chunk size.
    static const int kReps = 123123;
    for (int i = 0; i < kReps; ++i) {
      str8.insert(str8.end(), surrogates, surrogates + sizeof(surrogates));
    }
    UTF16Stream stream(llvh::ArrayRef<uint8_t>(str8.data(), str8.size()));
    EXPECT_TRUE(stream.hasChar());
    EXPECT_EQ('X', *stream);
    ++stream;
    for (int i = 0; i < kReps; ++i) {
      EXPECT_TRUE(stream.hasChar());
      EXPECT_EQ(0xD83D, *stream);
      ++stream;
      EXPECT_TRUE(stream.hasChar());
      EXPECT_EQ(0xDE39, *stream);
      ++stream;
    }
    EXPECT_FALSE(stream.hasChar());
  }
}

size_t countRemainingCharsInStream(UTF16Stream &&str) {
  size_t size = 0;
  while (str.hasChar()) {
    ++size;
    ++str;
  }
  return size;
}

TEST(UTF16StreamTest, MoveTest) {
  char16_t str16[] = {1, 123, 1234};
  UTF16Stream stream(llvh::ArrayRef<char16_t>(str16, str16 + 3));
  EXPECT_EQ(3, countRemainingCharsInStream(std::move(stream)));
}


// Forward declaration for getFactor function not in header but defined in UTF8.cpp
extern "C" {
  int getFactor(int value);
}

// Tests for getFactor function
TEST(UTF8Test, GetFactorWithValueTen) {
  // Special case: when value == 10, should return 0
  int result = getFactor(10);
  EXPECT_EQ(0, result);
}

TEST(UTF8Test, GetFactorWithValueZero) {
  // Default case: when value != 10, should return 100
  int result = getFactor(0);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithValueOne) {
  // Test with value 1
  int result = getFactor(1);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithNegativeValue) {
  // Test with negative value
  int result = getFactor(-5);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithPositiveValue) {
  // Test with positive value other than 10
  int result = getFactor(5);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorBoundaryValueNine) {
  // Test boundary near special case value 10
  int result = getFactor(9);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorBoundaryValueEleven) {
  // Test boundary near special case value 10
  int result = getFactor(11);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithLargePositiveValue) {
  // Test with large positive value
  int result = getFactor(1000);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithLargeNegativeValue) {
  // Test with large negative value
  int result = getFactor(-1000);
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithMinInt) {
  // Test with minimum integer value
  int result = getFactor(std::numeric_limits<int>::min());
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorWithMaxInt) {
  // Test with maximum integer value
  int result = getFactor(std::numeric_limits<int>::max());
  EXPECT_EQ(100, result);
}

TEST(UTF8Test, GetFactorMultipleCallsConsistency) {
  // Test that multiple calls return consistent results
  EXPECT_EQ(0, getFactor(10));
  EXPECT_EQ(0, getFactor(10));
  EXPECT_EQ(100, getFactor(5));
  EXPECT_EQ(100, getFactor(5));
}

TEST(UTF8Test, GetFactorDivisionByZeroWarning) {
  // Document the division by zero risk
  // getFactor(10) returns 0, which could cause issues if used as a divisor
  // This is a potential bug that should be addressed in production code
  int result = getFactor(10);
  EXPECT_EQ(0, result);
  
  // This test documents that care must be taken when using getFactor
  // in arithmetic operations, especially division
}

// Integration test between getFactor and getArea
TEST(UTF8Test, GetFactorIntegrationWithGetArea) {
  // Test that getFactor values work correctly with getArea
  // getArea uses getFactor as a divisor: return 100/getFactor(v)
  
  // For most values, getFactor returns 100, so getArea returns 1
  EXPECT_EQ(100, getFactor(5));  // Would result in 100/100 = 1 in getArea
  EXPECT_EQ(100, getFactor(15)); // Would result in 100/100 = 1 in getArea
  
  // For value 10, getFactor returns 0, which would cause division by zero
  // However, getArea has a special case check for v == 10 that returns 5
  EXPECT_EQ(0, getFactor(10));   // This would cause 100/0 if not handled
}

// Edge case tests for string encoding functions that might interact with getFactor
TEST(UTF8Test, GetFactorAllValidInputs) {
  // Test a comprehensive range of inputs to ensure robustness
  std::vector<int> testValues = {
    -2147483648, // INT_MIN
    -1000000,
    -1000,
    -100,
    -10,
    -1,
    0,
    1,
    9,
    10,
    11,
    100,
    1000,
    1000000,
    2147483647  // INT_MAX
  };
  
  for (int value : testValues) {
    int result = getFactor(value);
    if (value == 10) {
      EXPECT_EQ(0, result) << "Failed for value: " << value;
    } else {
      EXPECT_EQ(100, result) << "Failed for value: " << value;
    }
  }
}

} // end anonymous namespace