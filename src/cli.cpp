#include "cli.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

#include "report.hpp"
#include "sscc.hpp"

namespace sscc {
namespace {

// C++17: std::string_view literals. With the sv suffix, auto deduces
// std::string_view, which knows the length of the text without strlen.
using namespace std::string_view_literals;

constexpr auto kUsage = R"(
Usage:
  sscc CODE...        Check the given codes.
  sscc --file FILE    Check the codes in a text file, one code per line.
  sscc                Check the codes that you type or scan, one per line.
  sscc --help         Show this help.

Codes and files can be combined, and -f and -h are short for --file and --help.
A code is the AI 00 followed by the 18-digit SSCC, exactly as the scanner sends
it (without FNC1).

Examples:
  sscc 00034260311130776594
  sscc --file data/examples.txt

Exit status: 0 if all labels are valid, 1 if a label is invalid, and 2 if the
command line is wrong or a file cannot be read.
)"sv;

// The exit statuses follow grep and diff: 2 means that the program could not
// do its job, so scripts cannot mistake an error for an invalid label.
enum ExitStatus { kAllValid = 0, kInvalidLabel = 1, kError = 2 };

/// The number of valid and invalid labels so far, for the summary.
struct Tally {
    std::size_t valid = 0;
    std::size_t invalid = 0;
};

/// The first line of the output, which names the company of the labels.
void printTitle(std::ostream& out)
{
    out << "SSCC label check for " << kTsxCompanyName << " (GS1 company prefix "
        << kTsxCompanyPrefix << ")\n";
}

/// An error in the command line, with a pointer to the help.
std::runtime_error usageError(const std::string& message)
{
    return std::runtime_error{message + "\nRun \"sscc --help\" for usage."};
}

/// The codes in a text file, one code per line, without the blank lines.
/// Throws std::runtime_error if the file cannot be read.
std::vector<std::string> readCodes(const std::filesystem::path& path)
{
    // C++17: std::filesystem can tell why a file cannot be read, while a
    // std::ifstream only fails.
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error{"The file " + path.string() + " does not exist."};
    }
    if (std::filesystem::is_directory(path)) {
        throw std::runtime_error{path.string() + " is a folder, not a file."};
    }

    std::ifstream file{path};  // C++17: file streams also accept a std::filesystem::path.
    std::vector<std::string> codes;
    std::string line;
    for (bool firstLine = true; std::getline(file, line); firstLine = false) {
        // Notepad and other Windows tools can start a UTF-8 file with a byte
        // order mark (BOM), the bytes EF BB BF, which are not part of a code.
        if (firstLine && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            line.erase(0, 3);
        }
        if (!trim(line).empty()) {  // A blank line is not a scanned label.
            codes.push_back(line);
        }
    }
    // The loop also stops if the file cannot be opened or read to its end.
    if (!file.eof()) {
        throw std::runtime_error{"The file " + path.string() + " cannot be read."};
    }
    return codes;
}

/// The codes to check: the arguments and the lines of the files that --file
/// names, in the order of the command line. Throws std::runtime_error if the
/// command line is wrong or a file cannot be read.
std::vector<std::string> collectCodes(const std::vector<std::string_view>& args)
{
    std::vector<std::string> codes;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "-f" || arg == "--file") {
            if (++i == args.size() || args[i].empty()) {
                throw usageError(std::string{arg} + " needs a file name.");
            }
            const std::vector<std::string> fileCodes = readCodes(std::filesystem::path{args[i]});
            codes.insert(codes.end(), fileCodes.begin(), fileCodes.end());
        } else if (!arg.empty() && arg.front() == '-') {
            throw usageError("Unknown option " + std::string{arg} + '.');
        } else {
            codes.emplace_back(arg);
        }
    }
    return codes;
}

/// Checks one code and prints its report.
void check(std::string_view code, std::ostream& out, Tally& tally)
{
    const ValidationResult result = validate(code);
    if (result.isValid()) {
        ++tally.valid;
    } else {
        ++tally.invalid;
    }
    out << "\nLabel " << tally.valid + tally.invalid << '\n' << formatReport(result);
}

/// Checks the codes that the user types or scans, one per line, until an
/// empty line or the end of the input.
void checkConsole(std::istream& in, std::ostream& out, Tally& tally)
{
    out << "Type or scan a code and press Enter. Press Enter on an empty line to finish.\n";
    std::string line;
    while (true) {
        out << "\n> ";
        if (!std::getline(in, line) || trim(line).empty()) {
            return;
        }
        check(line, out, tally);
    }
}

}  // namespace

int run(const std::vector<std::string_view>& args, std::istream& in, std::ostream& out,
        std::ostream& err)
{
    // --help wins over all other arguments, even wrong ones.
    const auto isHelp = [](std::string_view arg) { return arg == "-h" || arg == "--help"; };
    if (std::any_of(args.begin(), args.end(), isHelp)) {
        printTitle(out);
        out << kUsage;
        return kAllValid;
    }

    try {
        // The codes are collected first, so that a wrong command line or a
        // missing file is reported before any label.
        const std::vector<std::string> codes = collectCodes(args);

        printTitle(out);
        Tally tally;
        if (args.empty()) {
            checkConsole(in, out, tally);
        } else {
            for (const std::string& code : codes) {
                check(code, out, tally);
            }
        }
        out << "\nSummary: " << tally.valid << " valid, " << tally.invalid << " invalid.\n";
        return tally.invalid == 0 ? kAllValid : kInvalidLabel;
    } catch (const std::exception& error) {
        err << "Error: " << error.what() << '\n';
        return kError;
    }
}

}  // namespace sscc
