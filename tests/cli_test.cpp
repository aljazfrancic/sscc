// Unit tests for the command line interface in cli.hpp. They run the whole
// program, with string streams in place of the console.
#include "cli.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace sscc {
namespace {

using ::testing::HasSubstr;

/// What one run of the program returned and printed.
struct Outcome {
    int status;
    std::string out;
    std::string err;
};

/// Runs the program as main() does, with `input` as the console input.
Outcome runProgram(const std::vector<std::string_view>& args, const std::string& input = "")
{
    std::istringstream in{input};
    std::ostringstream out;
    std::ostringstream err;
    const int status = run(args, in, out, err);
    return {status, out.str(), err.str()};
}

TEST(Cli, ShowsTheHelp)
{
    // C++17: structured bindings unpack the outcome into three variables.
    const auto [status, out, err] = runProgram({"--help"});
    EXPECT_EQ(status, 0);
    EXPECT_THAT(out, HasSubstr("Usage:"));
    EXPECT_EQ(err, "");
}

// A user who asks for help gets it, even with a wrong option.
TEST(Cli, ShowsTheHelpEvenWithAWrongOption)
{
    const auto [status, out, err] = runProgram({"--bogus", "-h"});
    EXPECT_EQ(status, 0);
    EXPECT_THAT(out, HasSubstr("Usage:"));
    EXPECT_EQ(err, "");
}

TEST(Cli, PrintsATitleAReportForEachLabelAndASummary)
{
    const auto [status, out, err] = runProgram({"00034260311130776144"});
    EXPECT_EQ(status, 1);
    EXPECT_EQ(out,
              "SSCC label check for TSX GmbH (GS1 company prefix 34260311)\n"
              "\n"
              "Label 1\n"
              "  Code:    00034260311130776144\n"
              "  Parts:   AI 00 | extension digit 0 | company prefix 34260311 | serial reference "
              "13077614 | check digit 4\n"
              "  Result:  INVALID, 1 problem:\n"
              "           - The check digit is 4, but it should be 3.\n"
              "\n"
              "Summary: 0 valid, 1 invalid.\n");
    EXPECT_EQ(err, "");
}

TEST(Cli, ExitsWithZeroIfAllLabelsAreValid)
{
    const auto [status, out, err] = runProgram({"00034260311130776594", "00034260311130776570"});
    EXPECT_EQ(status, 0);
    EXPECT_THAT(out, HasSubstr("\nSummary: 2 valid, 0 invalid.\n"));
    EXPECT_EQ(err, "");
}

// The data file holds the six task examples, of which only 1 and 4 are valid.
TEST(Cli, ChecksTheTaskExamplesInTheDataFile)
{
    const auto [status, out, err] = runProgram({"--file", SSCC_EXAMPLES_FILE});
    EXPECT_EQ(status, 1);
    EXPECT_THAT(out, HasSubstr("\nLabel 6\n  Code:    00034260321130774636\n"));
    EXPECT_THAT(out, HasSubstr("\nSummary: 2 valid, 4 invalid.\n"));
    EXPECT_EQ(err, "");
}

TEST(Cli, ChecksCodesAndFilesInTheOrderOfTheCommandLine)
{
    const auto [status, out, err] =
        runProgram({"00034260311130776144", "-f", SSCC_EXAMPLES_FILE, "00034260311130776594"});
    EXPECT_EQ(status, 1);
    EXPECT_THAT(out, HasSubstr("\nLabel 1\n  Code:    00034260311130776144\n"));
    EXPECT_THAT(out, HasSubstr("\nLabel 2\n  Code:    00034260311130776594\n"));
    EXPECT_THAT(out, HasSubstr("\nLabel 8\n  Code:    00034260311130776594\n"));
    EXPECT_THAT(out, HasSubstr("\nSummary: 3 valid, 5 invalid.\n"));
    EXPECT_EQ(err, "");
}

// Notepad and other Windows tools can start a UTF-8 file with a byte order
// mark and end its lines with "\r\n". Blank lines are not labels.
TEST(Cli, ReadsAFileFromWindows)
{
    const std::filesystem::path path = std::filesystem::path{SSCC_BUILD_DIR} / "windows_file.txt";
    // The BOM is a separate literal, because a hex escape would also take
    // the digits after it.
    std::ofstream{path, std::ios::binary}
        << "\xEF\xBB\xBF" "00034260311130776594\r\n \t\r\n\r\n00034260311130776570\r\n";
    const auto [status, out, err] = runProgram({"--file", path.string()});
    std::filesystem::remove(path);

    EXPECT_EQ(status, 0);
    EXPECT_THAT(out, HasSubstr("\nSummary: 2 valid, 0 invalid.\n"));
    EXPECT_EQ(err, "");
}

// Without arguments, the program reads the console. An empty line ends the
// input, so the last code is not checked.
TEST(Cli, ReadsTheConsoleUntilAnEmptyLine)
{
    const auto [status, out, err] =
        runProgram({}, "00034260311130776594\n00034260311130776144\n\n00034260311130776570\n");
    EXPECT_EQ(status, 1);
    EXPECT_THAT(out, HasSubstr("\nSummary: 1 valid, 1 invalid.\n"));
    EXPECT_EQ(err, "");
}

TEST(Cli, ReadsTheConsoleUntilTheEndOfTheInput)
{
    const auto [status, out, err] = runProgram({}, "00034260311130776594");
    EXPECT_EQ(status, 0);
    EXPECT_THAT(out, HasSubstr("\nSummary: 1 valid, 0 invalid.\n"));
    EXPECT_EQ(err, "");
}

// Errors exit with 2, so that scripts can tell them from invalid labels, and
// they come before any label is printed.
TEST(Cli, RejectsAnUnknownOption)
{
    const auto [status, out, err] = runProgram({"00034260311130776594", "--bogus"});
    EXPECT_EQ(status, 2);
    EXPECT_EQ(out, "");
    EXPECT_EQ(err, "Error: Unknown option --bogus.\nRun \"sscc --help\" for usage.\n");
}

TEST(Cli, NeedsAFileNameAfterFile)
{
    const auto [status, out, err] = runProgram({"00034260311130776594", "--file"});
    EXPECT_EQ(status, 2);
    EXPECT_EQ(out, "");
    EXPECT_EQ(err, "Error: --file needs a file name.\nRun \"sscc --help\" for usage.\n");
}

TEST(Cli, ReportsAMissingFile)
{
    const auto [status, out, err] = runProgram({"00034260311130776594", "-f", "no/such/file.txt"});
    EXPECT_EQ(status, 2);
    EXPECT_EQ(out, "");
    EXPECT_EQ(err, "Error: The file no/such/file.txt does not exist.\n");
}

TEST(Cli, ReportsAFolderInsteadOfAFile)
{
    const std::string folder = std::filesystem::temp_directory_path().string();
    const auto [status, out, err] = runProgram({"--file", folder});
    EXPECT_EQ(status, 2);
    EXPECT_EQ(out, "");
    EXPECT_EQ(err, "Error: " + folder + " is a folder, not a file.\n");
}

}  // namespace
}  // namespace sscc
