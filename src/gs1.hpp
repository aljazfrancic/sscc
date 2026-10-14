// General GS1 rules that are not specific to SSCC labels or to TSX: the
// modulo-10 check digit and the titles of common Application Identifiers.
// Everything here is constexpr, so it can also be checked at compile time.
#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

// C++17: nested namespace definition, short for namespace sscc { namespace gs1 {.
namespace sscc::gs1 {

/// True for the ASCII digits 0 to 9. Unlike std::isdigit, it does not depend
/// on the locale and is safe for negative char values.
[[nodiscard]] constexpr bool isDigit(char c) noexcept  // C++17: [[nodiscard]]
{
    return c >= '0' && c <= '9';
}

/// The GS1 modulo-10 check digit of a string of digits, as used by SSCCs and
/// GTINs. From the rightmost digit on, the digits are weighted 3, 1, 3, 1, ...,
/// and the check digit tops their sum up to a multiple of ten.
/// Precondition: `digits` contains only the digits 0 to 9.
[[nodiscard]] constexpr int checkDigit(std::string_view digits) noexcept
{
    // C++17: lambdas are implicitly constexpr, so this one also works in the
    // compile-time checks below.
    constexpr auto weight = [](std::size_t positionFromRight) {
        return positionFromRight % 2 == 0 ? 3 : 1;
    };

    int sum = 0;
    for (std::size_t i = 0; i < digits.size(); ++i) {
        sum += weight(i) * (digits[digits.size() - 1 - i] - '0');
    }
    return (10 - sum % 10) % 10;
}

// C++17: static_assert without a message. The algorithm is checked at compile
// time with examples 1 and 4 from the task, whose weighted sums are 136 and 130.
// These checks stay here even with unit tests: they cost nothing at run time,
// and they break the build if checkDigit stops being constexpr.
static_assert(checkDigit("03426031113077659") == 4);
static_assert(checkDigit("03426031113077657") == 0);

/// A GS1 Application Identifier (AI) and a short title of its meaning.
struct ApplicationIdentifier {
    std::string_view code;
    std::string_view title;
};

/// Common 2-digit AIs on logistic labels, used to explain a wrong AI.
// C++17: inline variable, so the table exists only once in the program even
// though this header is included in several source files. Class template
// argument deduction (CTAD) deduces std::array<ApplicationIdentifier, 11>.
inline constexpr std::array kCommonAis{
    ApplicationIdentifier{"00", "SSCC"},
    ApplicationIdentifier{"01", "GTIN"},
    ApplicationIdentifier{"02", "GTIN of contained trade items"},
    ApplicationIdentifier{"10", "batch or lot number"},
    ApplicationIdentifier{"11", "production date"},
    ApplicationIdentifier{"13", "packaging date"},
    ApplicationIdentifier{"15", "best before date"},
    ApplicationIdentifier{"17", "expiration date"},
    ApplicationIdentifier{"20", "product variant"},
    ApplicationIdentifier{"21", "serial number"},
    ApplicationIdentifier{"37", "count of contained trade items"},
};

/// The title of a common AI, or std::nullopt if the AI is not in the table.
// C++17: std::optional expresses "maybe no value" without a magic value.
[[nodiscard]] constexpr std::optional<std::string_view> aiTitle(std::string_view code) noexcept
{
    for (const auto& ai : kCommonAis) {
        if (ai.code == code) {
            return ai.title;
        }
    }
    return std::nullopt;
}

}  // namespace sscc::gs1
