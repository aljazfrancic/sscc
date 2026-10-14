// Unit tests for validate() and trim() in sscc.hpp, including the six task
// examples with the results that plan.md works out by hand.
#include "sscc.hpp"

#include <gtest/gtest.h>

#include <ostream>
#include <tuple>
#include <vector>

#include "report.hpp"

namespace sscc {

// C++20 can generate these comparisons with = default, but C++17 cannot, so
// they are written by hand. They live here, because only the tests need them.
bool operator==(const EmptyCode&, const EmptyCode&)
{
    return true;
}
bool operator==(const InvalidCharacter& a, const InvalidCharacter& b)
{
    return std::tie(a.position, a.character) == std::tie(b.position, b.character);
}
bool operator==(const NonDigitCharacters& a, const NonDigitCharacters& b)
{
    return a.characters == b.characters;
}
bool operator==(const WrongLength& a, const WrongLength& b)
{
    return a.length == b.length;
}
bool operator==(const WrongApplicationIdentifier& a, const WrongApplicationIdentifier& b)
{
    return a.ai == b.ai;
}
bool operator==(const WrongCompanyPrefix& a, const WrongCompanyPrefix& b)
{
    return std::tie(a.actual, a.expected, a.differingDigits) ==
           std::tie(b.actual, b.expected, b.differingDigits);
}
bool operator==(const WrongCheckDigit& a, const WrongCheckDigit& b)
{
    return std::tie(a.actual, a.expected) == std::tie(b.actual, b.expected);
}

using Problems = std::vector<Problem>;

// If a test fails, GoogleTest prints the problems as sentences from
// report.hpp instead of raw bytes. It finds this function by argument-
// dependent lookup, because the problem types are in namespace sscc. A
// PrintTo() for one Problem would be ignored, because GoogleTest prints every
// std::variant with its own printer.
void PrintTo(const Problems& problems, std::ostream* out)
{
    const char* separator = " ";
    *out << '{';
    for (const Problem& problem : problems) {
        *out << separator << '"' << describe(problem) << '"';
        separator = ", ";
    }
    *out << " }";
}

namespace {

TEST(TaskExamples, Example1IsValid)
{
    EXPECT_EQ(validate("00034260311130776594").problems, Problems{});
}

TEST(TaskExamples, Example2HasTheWrongAiCompanyPrefixAndCheckDigit)
{
    EXPECT_EQ(validate("02044260311130776512").problems,
              (Problems{WrongApplicationIdentifier{"02"},
                        WrongCompanyPrefix{"44260311", "34260311", {1}}, WrongCheckDigit{2, 7}}));
}

TEST(TaskExamples, Example3HasTheWrongCheckDigit)
{
    EXPECT_EQ(validate("00034260311130776144").problems, (Problems{WrongCheckDigit{4, 3}}));
}

TEST(TaskExamples, Example4IsValid)
{
    EXPECT_EQ(validate("00034260311130776570").problems, Problems{});
}

TEST(TaskExamples, Example5IsOneDigitShort)
{
    EXPECT_EQ(validate("0003426031113077646").problems, (Problems{WrongLength{19}}));
}

TEST(TaskExamples, Example6HasTheWrongCompanyPrefix)
{
    EXPECT_EQ(validate("00034260321130774636").problems,
              (Problems{WrongCompanyPrefix{"34260321", "34260311", {7}}}));
}

TEST(Validate, SplitsAValidCodeIntoItsParts)
{
    const ValidationResult result = validate("00034260311130776594");
    EXPECT_TRUE(result.isValid());
    ASSERT_TRUE(result.parts.has_value());
    EXPECT_EQ(result.parts->ai, "00");
    EXPECT_EQ(result.parts->extensionDigit, '0');
    EXPECT_EQ(result.parts->companyPrefix, "34260311");
    EXPECT_EQ(result.parts->serialReference, "13077659");
    EXPECT_EQ(result.parts->checkDigit, '4');
}

// The length of a company prefix depends on the company, and the serial
// reference fills the digits that the prefix leaves free. This SSCC has the
// 7-digit prefix 0614141, which GS1 uses in its examples.
TEST(Validate, SplitsTheSsccForAnyCompanyPrefixLength)
{
    const ValidationResult result = validate("00106141411234567897", "0614141");
    EXPECT_EQ(result.problems, Problems{});
    ASSERT_TRUE(result.parts.has_value());
    EXPECT_EQ(result.parts->extensionDigit, '1');
    EXPECT_EQ(result.parts->companyPrefix, "0614141");
    EXPECT_EQ(result.parts->serialReference, "123456789");
    EXPECT_EQ(result.parts->checkDigit, '7');
}

// A line from a Windows file ends with "\r\n", which is not part of the code.
TEST(Validate, IgnoresWhitespaceAroundTheCode)
{
    const ValidationResult result = validate(" \t00034260311130776594\r\n");
    EXPECT_EQ(result.code, "00034260311130776594");
    EXPECT_EQ(result.problems, Problems{});
}

TEST(Validate, ReportsAnEmptyCode)
{
    EXPECT_EQ(validate("").problems, Problems{EmptyCode{}});
    EXPECT_EQ(validate(" \r\n").problems, Problems{EmptyCode{}});
}

// The text under a barcode shows the AI in parentheses, but the scanner does
// not send them. The parts are only known for a code of exactly 20 digits.
TEST(Validate, ReportsEveryNonDigitWithItsPosition)
{
    EXPECT_EQ(validate("(00)034260311130776594").problems,
              (Problems{NonDigitCharacters{{InvalidCharacter{1, '('}, InvalidCharacter{4, ')'}}},
                        WrongLength{22}}));

    const ValidationResult result = validate("0003426031113077659A");
    EXPECT_EQ(result.problems, (Problems{NonDigitCharacters{{InvalidCharacter{20, 'A'}}}}));
    EXPECT_FALSE(result.parts.has_value());
}

TEST(Validate, ReportsTheWrongLength)
{
    EXPECT_EQ(validate("000342603111307765941").problems, (Problems{WrongLength{21}}));
    EXPECT_EQ(validate("0003426031113077646").problems, (Problems{WrongLength{19}}));
}

// The check digit protects only the 18 SSCC digits, not the AI, so a wrong AI
// can be the only problem of a code.
TEST(Validate, ReportsAWrongAiOfAnOtherwiseCorrectCode)
{
    EXPECT_EQ(validate("01034260311130776594").problems,
              (Problems{WrongApplicationIdentifier{"01"}}));
}

// A wrong AI means that the label carries other data, so it is also reported
// for a code of the wrong length, as long as the AI has two digits.
TEST(Validate, ChecksTheAiOfACodeWithTheWrongLength)
{
    EXPECT_EQ(validate("02").problems, (Problems{WrongLength{2}, WrongApplicationIdentifier{"02"}}));
    EXPECT_EQ(validate("0").problems, (Problems{WrongLength{1}}));
    EXPECT_EQ(validate("A2").problems,
              (Problems{NonDigitCharacters{{InvalidCharacter{1, 'A'}}}, WrongLength{2}}));
}

// A label printed with two swapped digits in the company prefix. Its check
// digit matches its digits, so only the prefix is wrong.
TEST(Validate, ReportsEachDifferingDigitOfTheCompanyPrefix)
{
    EXPECT_EQ(validate("00034620311130776592").problems,
              (Problems{WrongCompanyPrefix{"34620311", "34260311", {3, 4}}}));
}

TEST(Validate, ReportsEveryProblemInOrder)
{
    EXPECT_EQ(validate("02X").problems,
              (Problems{NonDigitCharacters{{InvalidCharacter{3, 'X'}}}, WrongLength{3},
                        WrongApplicationIdentifier{"02"}}));
}

TEST(Trim, RemovesOnlyTheWhitespaceAroundTheText)
{
    EXPECT_EQ(trim(" \t12 34\r\n"), "12 34");
    EXPECT_EQ(trim("\v\f"), "");
    EXPECT_EQ(trim(""), "");
}

}  // namespace
}  // namespace sscc
