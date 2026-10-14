#include "report.hpp"

#include <sstream>
#include <string_view>
#include <variant>
#include <vector>

#include "gs1.hpp"

namespace sscc {
namespace {

// C++17: the "overloaded" idiom. It combines several lambdas into one object,
// so that std::visit can call the lambda that matches the type in the variant.
template <typename... Ts>     // Ts: the types of the lambdas, any number of them.
struct Overloaded : Ts... {   // Inherit from every lambda to get its operator().
    using Ts::operator()...;  // C++17: let a call choose among all those operator()s.
};

// C++17: deduction guide. Lambda types have no names, so they cannot be written
// as Overloaded<...> by hand. This rule lets the compiler deduce them instead.
template <typename... Ts>                // For lambdas of any types Ts...,
Overloaded(Ts...) -> Overloaded<Ts...>;  // Overloaded{lambdas...} is an Overloaded<Ts...>.

/// "1 character", "2 characters".
std::string count(std::size_t n, std::string_view noun)
{
    std::string text = std::to_string(n) + ' ' + std::string{noun};
    if (n != 1) {
        text += 's';
    }
    return text;
}

/// "a", "a and b", "a, b and c".
std::string joinWithAnd(const std::vector<std::string>& items)
{
    std::string text;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            text += (i + 1 == items.size()) ? " and " : ", ";
        }
        text += items[i];
    }
    return text;
}

/// A readable name for a character that is not a digit.
std::string describeCharacter(char c)
{
    // Scanners send an FNC1 that separates two fields as the invisible ASCII
    // control character GS (29). An SSCC label never needs one.
    constexpr char kGroupSeparator = '\x1D';
    if (c == ' ') {
        return "a space";
    }
    if (c == '\t') {
        return "a tab";
    }
    if (c == kGroupSeparator) {
        return "GS (an FNC1 field separator)";
    }
    const auto byte = static_cast<unsigned char>(c);
    if (byte > 0x20 && byte < 0x7F) {
        return std::string{'\''} + c + '\'';
    }
    constexpr std::string_view kHexDigits = "0123456789ABCDEF";
    return std::string{"the byte 0x"} + kHexDigits[byte / 16] + kHexDigits[byte % 16];
}

}  // namespace

std::string describe(const Problem& problem)
{
    // C++17: std::visit calls the lambda that matches the type in the variant.
    // A kind of problem without a lambda is a compile error, not a runtime bug.
    return std::visit(
        Overloaded{  // C++17: aggregate initialization, each lambda becomes one base class.
            [](const EmptyCode&) -> std::string { return "The code is empty."; },
            [](const NonDigitCharacters& p) -> std::string {
                std::vector<std::string> items;
                // C++17: structured bindings unpack each InvalidCharacter.
                for (const auto& [position, character] : p.characters) {
                    items.push_back(describeCharacter(character) + " at position " +
                                    std::to_string(position));
                }
                return "The code may only contain digits, but it has " + joinWithAnd(items) + '.';
            },
            [](const WrongLength& p) -> std::string {
                std::string text = "The code has " + count(p.length, "character") +
                                   " instead of " + std::to_string(kCodeLength) +
                                   " (AI 00 and an 18-digit SSCC), so ";
                if (p.length < kCodeLength) {
                    const std::size_t missing = kCodeLength - p.length;
                    return text + count(missing, "character") + (missing == 1 ? " is" : " are") +
                           " missing.";
                }
                const std::size_t extra = p.length - kCodeLength;
                return text + (extra == 1 ? "there is " : "there are ") +
                       count(extra, "character") + " too many.";
            },
            [](const WrongApplicationIdentifier& p) -> std::string {
                if (const auto title = gs1::aiTitle(p.ai)) {
                    return "The code starts with AI " + p.ai + " (" + std::string{*title} +
                           ") instead of AI 00 (SSCC).";
                }
                return "The code starts with " + p.ai + " instead of AI 00 (SSCC).";
            },
            [](const WrongCompanyPrefix& p) -> std::string {
                std::string detail = "all digits differ";
                if (p.differingDigits.size() < p.expected.size()) {
                    std::vector<std::string> positions;
                    for (const std::size_t position : p.differingDigits) {
                        positions.push_back(std::to_string(position));
                    }
                    const bool single = positions.size() == 1;
                    detail = (single ? "digit " : "digits ") + joinWithAnd(positions) +
                             (single ? " differs" : " differ");
                }
                return "The company prefix is " + p.actual + ", but it should be " + p.expected +
                       " (" + detail + ").";
            },
            [](const WrongCheckDigit& p) -> std::string {
                return "The check digit is " + std::to_string(p.actual) + ", but it should be " +
                       std::to_string(p.expected) + '.';
            },
        },
        problem);
}

std::string formatReport(const ValidationResult& result)
{
    std::ostringstream out;
    out << "  Code:    " << (result.code.empty() ? "(empty)" : result.code) << '\n';
    if (result.parts) {
        const SsccParts& parts = *result.parts;
        out << "  Parts:   AI " << parts.ai << " | extension digit " << parts.extensionDigit
            << " | company prefix " << parts.companyPrefix << " | serial reference "
            << parts.serialReference << " | check digit " << parts.checkDigit << '\n';
    }
    if (result.isValid()) {
        out << "  Result:  VALID\n";
        return out.str();
    }
    out << "  Result:  INVALID, " << count(result.problems.size(), "problem") << ":\n";
    for (const Problem& problem : result.problems) {
        out << "           - " << describe(problem) << '\n';
    }
    return out.str();
}

}  // namespace sscc
