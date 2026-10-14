// Checks of scanned SSCC labels for TSX GmbH. Nothing here reads input or
// prints output, so the console program and the unit tests share the code.
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace sscc {

// C++17: inline variables, defined once for the whole program in this header.
inline constexpr std::string_view kSsccAi = "00";  // The AI of an SSCC.
inline constexpr std::size_t kSsccLength = 18;      // Digits of an SSCC, without the AI.
inline constexpr std::size_t kCodeLength = kSsccAi.size() + kSsccLength;  // 20 scanned digits.
inline constexpr std::string_view kTsxCompanyName = "TSX GmbH";
inline constexpr std::string_view kTsxCompanyPrefix = "34260311";

// Each kind of problem is a separate type that carries the details needed to
// explain it. The wording lives in report.hpp, so the unit tests can check
// the exact values instead of comparing sentences.

/// The code is empty or contains only whitespace.
struct EmptyCode {};

/// A character that is not a digit, with its 1-based position in the code.
struct InvalidCharacter {
    std::size_t position;
    char character;
};

/// The code contains characters other than digits.
struct NonDigitCharacters {
    std::vector<InvalidCharacter> characters;
};

/// The code does not have 20 characters (AI 00 and an 18-digit SSCC).
struct WrongLength {
    std::size_t length;
};

/// The code starts with another AI than 00.
struct WrongApplicationIdentifier {
    std::string ai;
};

/// The company prefix is not the expected one.
struct WrongCompanyPrefix {
    std::string actual;
    std::string expected;
    std::vector<std::size_t> differingDigits;  // 1-based positions in the prefix.
};

/// The check digit does not match the other 17 digits of the SSCC.
struct WrongCheckDigit {
    int actual;
    int expected;
};

// C++17: std::variant, a type-safe union. A Problem holds exactly one of the
// types above.
using Problem = std::variant<EmptyCode, NonDigitCharacters, WrongLength,
                             WrongApplicationIdentifier, WrongCompanyPrefix, WrongCheckDigit>;

/// The fields of a 20-digit code: the AI, then the 18 digits of the SSCC.
struct SsccParts {
    std::string ai;
    char extensionDigit;
    std::string companyPrefix;
    std::string serialReference;
    char checkDigit;
};

/// The outcome of checking one scanned code.
struct ValidationResult {
    std::string code;                // The scanned code without surrounding whitespace.
    std::optional<SsccParts> parts;  // Only set if the code has exactly 20 digits.
    std::vector<Problem> problems;   // Empty if the label is valid.

    [[nodiscard]] bool isValid() const noexcept { return problems.empty(); }
};

/// Checks one scanned code, which is the AI followed by the SSCC, without
/// FNC1, exactly as the RF terminal sends it. Surrounding whitespace, such as
/// the '\r' of a Windows line ending, is ignored. Every problem is reported,
/// not only the first one.
/// Precondition: `companyPrefix` has 1 to 16 digits.
[[nodiscard]] ValidationResult validate(std::string_view scannedCode,
                                        std::string_view companyPrefix = kTsxCompanyPrefix);

}  // namespace sscc
