#include "sscc.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

#include "gs1.hpp"

namespace sscc {
namespace {

// C++17: fold expression. ((c == candidates) || ...) expands to
// (c == ' ') || (c == '\t') || ... for all the arguments.
template <typename... Candidates>
constexpr bool isAnyOf(char c, Candidates... candidates) noexcept
{
    return ((c == candidates) || ...);
}

constexpr bool isWhitespace(char c) noexcept
{
    return isAnyOf(c, ' ', '\t', '\r', '\n', '\v', '\f');
}

/// The text without leading and trailing whitespace.
std::string_view trim(std::string_view text) noexcept
{
    while (!text.empty() && isWhitespace(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && isWhitespace(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

}  // namespace

ValidationResult validate(std::string_view scannedCode, std::string_view companyPrefix)
{
    // An SSCC has 18 digits: the extension digit, the company prefix, the serial
    // reference and the check digit. The prefix and the serial reference share
    // the 16 digits in between, so the prefix can have at most 16 digits.
    assert(!companyPrefix.empty() && companyPrefix.size() <= kSsccLength - 2);

    const std::string_view code = trim(scannedCode);
    ValidationResult result;
    result.code = std::string{code};

    if (code.empty()) {
        result.problems.emplace_back(EmptyCode{});
        return result;
    }

    NonDigitCharacters nonDigits;
    for (std::size_t i = 0; i < code.size(); ++i) {
        if (!gs1::isDigit(code[i])) {
            nonDigits.characters.push_back({i + 1, code[i]});
        }
    }
    const bool onlyDigits = nonDigits.characters.empty();
    if (!onlyDigits) {
        result.problems.emplace_back(std::move(nonDigits));
    }

    if (code.size() != kCodeLength) {
        result.problems.emplace_back(WrongLength{code.size()});
    }

    // The AI is checked even if the rest of the code is broken, because a
    // wrong AI means that the label carries a different kind of data.
    // C++17: if statement with an initializer, so `ai` exists only here.
    if (const auto ai = code.substr(0, kSsccAi.size());
        ai.size() == kSsccAi.size() && std::all_of(ai.begin(), ai.end(), gs1::isDigit) &&
        ai != kSsccAi) {
        result.problems.emplace_back(WrongApplicationIdentifier{std::string{ai}});
    }

    // The fields can only be located in a code of exactly 20 digits.
    if (!onlyDigits || code.size() != kCodeLength) {
        return result;
    }

    // An SSCC consists of the extension digit, the company prefix, the serial
    // reference and the check digit. The serial reference fills the digits
    // that the company prefix leaves free.
    const std::string_view sscc = code.substr(kSsccAi.size());
    const std::size_t serialLength = kSsccLength - 2 - companyPrefix.size();
    SsccParts parts{
        std::string{code.substr(0, kSsccAi.size())},
        sscc.front(),
        std::string{sscc.substr(1, companyPrefix.size())},
        std::string{sscc.substr(1 + companyPrefix.size(), serialLength)},
        sscc.back(),
    };

    if (parts.companyPrefix != companyPrefix) {
        WrongCompanyPrefix wrongPrefix{parts.companyPrefix, std::string{companyPrefix}, {}};
        for (std::size_t i = 0; i < companyPrefix.size(); ++i) {
            if (parts.companyPrefix[i] != companyPrefix[i]) {
                wrongPrefix.differingDigits.push_back(i + 1);
            }
        }
        result.problems.emplace_back(std::move(wrongPrefix));
    }

    // The check digit protects the 17 SSCC digits before it, but not the AI.
    const int expectedCheckDigit = gs1::checkDigit(sscc.substr(0, kSsccLength - 1));
    if (const int actualCheckDigit = parts.checkDigit - '0';
        actualCheckDigit != expectedCheckDigit) {
        result.problems.emplace_back(WrongCheckDigit{actualCheckDigit, expectedCheckDigit});
    }

    result.parts = std::move(parts);
    return result;
}

}  // namespace sscc
