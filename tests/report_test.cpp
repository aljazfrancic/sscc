// Unit tests for the English sentences and reports in report.hpp.
#include "report.hpp"

#include <gtest/gtest.h>

namespace sscc {
namespace {

TEST(Describe, EmptyCode)
{
    EXPECT_EQ(describe(EmptyCode{}), "The code is empty.");
}

// Invisible characters get a name, so the user can see what is wrong.
TEST(Describe, NonDigitCharacters)
{
    EXPECT_EQ(describe(NonDigitCharacters{{InvalidCharacter{20, 'A'}}}),
              "The code may only contain digits, but it has 'A' at position 20.");
    EXPECT_EQ(describe(NonDigitCharacters{{InvalidCharacter{1, ' '}, InvalidCharacter{2, '\t'},
                                           InvalidCharacter{3, '\x1D'},
                                           InvalidCharacter{4, '\xFF'}}}),
              "The code may only contain digits, but it has a space at position 1, a tab at "
              "position 2, GS (an FNC1 field separator) at position 3 and the byte 0xFF at "
              "position 4.");
}

// The sentences use the singular for one character and the plural otherwise.
TEST(Describe, WrongLength)
{
    EXPECT_EQ(describe(WrongLength{19}),
              "The code has 19 characters instead of 20 (AI 00 and an 18-digit SSCC), so 1 "
              "character is missing.");
    EXPECT_EQ(describe(WrongLength{1}),
              "The code has 1 character instead of 20 (AI 00 and an 18-digit SSCC), so 19 "
              "characters are missing.");
    EXPECT_EQ(describe(WrongLength{21}),
              "The code has 21 characters instead of 20 (AI 00 and an 18-digit SSCC), so there "
              "is 1 character too many.");
    EXPECT_EQ(describe(WrongLength{22}),
              "The code has 22 characters instead of 20 (AI 00 and an 18-digit SSCC), so there "
              "are 2 characters too many.");
}

TEST(Describe, WrongApplicationIdentifier)
{
    EXPECT_EQ(describe(WrongApplicationIdentifier{"02"}),
              "The code starts with AI 02 (GTIN of contained trade items) instead of AI 00 "
              "(SSCC).");
    EXPECT_EQ(describe(WrongApplicationIdentifier{"99"}),
              "The code starts with 99 instead of AI 00 (SSCC).");
}

TEST(Describe, WrongCompanyPrefix)
{
    EXPECT_EQ(describe(WrongCompanyPrefix{"34260321", "34260311", {7}}),
              "The company prefix is 34260321, but it should be 34260311 (digit 7 differs).");
    EXPECT_EQ(describe(WrongCompanyPrefix{"34620311", "34260311", {3, 4}}),
              "The company prefix is 34620311, but it should be 34260311 (digits 3 and 4 "
              "differ).");
    EXPECT_EQ(describe(WrongCompanyPrefix{"99960311", "34260311", {1, 2, 3}}),
              "The company prefix is 99960311, but it should be 34260311 (digits 1, 2 and 3 "
              "differ).");
    EXPECT_EQ(describe(WrongCompanyPrefix{"99999999", "34260311", {1, 2, 3, 4, 5, 6, 7, 8}}),
              "The company prefix is 99999999, but it should be 34260311 (all digits differ).");
}

TEST(Describe, WrongCheckDigit)
{
    EXPECT_EQ(describe(WrongCheckDigit{4, 3}), "The check digit is 4, but it should be 3.");
}

TEST(FormatReport, ShowsThePartsOfAValidLabel)
{
    EXPECT_EQ(formatReport(validate("00034260311130776594")),
              "  Code:    00034260311130776594\n"
              "  Parts:   AI 00 | extension digit 0 | company prefix 34260311 | serial reference "
              "13077659 | check digit 4\n"
              "  Result:  VALID\n");
}

TEST(FormatReport, ListsEveryProblemOfAnInvalidLabel)
{
    EXPECT_EQ(formatReport(validate("02044260311130776512")),
              "  Code:    02044260311130776512\n"
              "  Parts:   AI 02 | extension digit 0 | company prefix 44260311 | serial reference "
              "13077651 | check digit 2\n"
              "  Result:  INVALID, 3 problems:\n"
              "           - The code starts with AI 02 (GTIN of contained trade items) instead of "
              "AI 00 (SSCC).\n"
              "           - The company prefix is 44260311, but it should be 34260311 (digit 1 "
              "differs).\n"
              "           - The check digit is 2, but it should be 7.\n");
}

// The parts can only be located in a code of exactly 20 digits.
TEST(FormatReport, LeavesOutThePartsOfACodeWithTheWrongLength)
{
    EXPECT_EQ(formatReport(validate("0003426031113077646")),
              "  Code:    0003426031113077646\n"
              "  Result:  INVALID, 1 problem:\n"
              "           - The code has 19 characters instead of 20 (AI 00 and an 18-digit "
              "SSCC), so 1 character is missing.\n");
}

TEST(FormatReport, ShowsAnEmptyCode)
{
    EXPECT_EQ(formatReport(validate("")),
              "  Code:    (empty)\n"
              "  Result:  INVALID, 1 problem:\n"
              "           - The code is empty.\n");
}

}  // namespace
}  // namespace sscc
