// Turns validation results into user-friendly English text. The wording is
// kept apart from the checks, so it can change without touching the rules.
#pragma once

#include <string>

#include "sscc.hpp"

namespace sscc {

/// One sentence that explains a problem, for example
/// "The check digit is 4, but it should be 3."
[[nodiscard]] std::string describe(const Problem& problem);

/// Indented lines with the code, its parts (if known) and the verdict, with
/// one line for each problem.
[[nodiscard]] std::string formatReport(const ValidationResult& result);

}  // namespace sscc
