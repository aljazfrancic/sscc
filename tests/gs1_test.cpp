// Unit tests for the general GS1 rules in gs1.hpp.
#include "gs1.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <string>

namespace sscc::gs1 {
namespace {

// The 17 SSCC digits before the check digit in the task examples. Example 5
// is left out, because a digit is missing. Examples 2 and 3 end with 2 and 4
// on the label, which is why they are invalid.
TEST(CheckDigit, MatchesTheTaskExamples)
{
    EXPECT_EQ(checkDigit("03426031113077659"), 4);  // Example 1.
    EXPECT_EQ(checkDigit("04426031113077651"), 7);  // Example 2.
    EXPECT_EQ(checkDigit("03426031113077614"), 3);  // Example 3.
    EXPECT_EQ(checkDigit("03426031113077657"), 0);  // Example 4, whose sum 130 needs nothing.
    EXPECT_EQ(checkDigit("03426032113077463"), 6);  // Example 6.
}

// The worked example from GS1 ("How to calculate a check digit manually"):
// the weighted sum of the GTIN-13 629104150021 is 57, so 3 tops it up to 60.
TEST(CheckDigit, MatchesTheGs1Example)
{
    EXPECT_EQ(checkDigit("629104150021"), 3);
}

// The weights start with 3 at the rightmost digit, whatever the length, so
// the same function works for GS1 keys of every length.
TEST(CheckDigit, WeightsTheRightmostDigitWithThree)
{
    EXPECT_EQ(checkDigit("1"), 7);    // 1*3 = 3
    EXPECT_EQ(checkDigit("10"), 9);   // 1*1 + 0*3 = 1
    EXPECT_EQ(checkDigit("100"), 7);  // 1*3 + 0*1 + 0*3 = 3
    EXPECT_EQ(checkDigit(""), 0);     // The sum 0 is already a multiple of ten.
}

// The check digit catches every error in a single digit: changing a digit by d
// changes the sum by d or 3*d, and for d from -9 to 9 (but not 0) neither is a
// multiple of ten, because 1 and 3 have no common factor with 10.
TEST(CheckDigit, DetectsEverySingleDigitError)
{
    const std::string digits = "03426031113077659";  // Example 1, whose check digit is 4.
    for (std::size_t i = 0; i < digits.size(); ++i) {
        for (char digit = '0'; digit <= '9'; ++digit) {
            if (digit != digits[i]) {
                std::string changed = digits;
                changed[i] = digit;
                EXPECT_NE(checkDigit(changed), 4) << changed;
            }
        }
    }
}

TEST(IsDigit, AcceptsOnlyTheDigitsZeroToNine)
{
    for (char c = '0'; c <= '9'; ++c) {
        EXPECT_TRUE(isDigit(c)) << c;
    }
    EXPECT_FALSE(isDigit('/'));     // The character just before '0'.
    EXPECT_FALSE(isDigit(':'));     // The character just after '9'.
    EXPECT_FALSE(isDigit(' '));
    EXPECT_FALSE(isDigit('\xFF'));  // Negative where char is signed.
}

TEST(AiTitle, NamesCommonAis)
{
    EXPECT_EQ(aiTitle("00"), "SSCC");
    EXPECT_EQ(aiTitle("02"), "GTIN of contained trade items");
}

TEST(AiTitle, IsEmptyForOtherAis)
{
    EXPECT_EQ(aiTitle("99"), std::nullopt);
    EXPECT_EQ(aiTitle(""), std::nullopt);
}

}  // namespace
}  // namespace sscc::gs1
