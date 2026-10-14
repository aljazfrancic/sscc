// The command line interface: reads codes from the arguments, from text files
// or from the console, and prints a report for each label. The streams are
// parameters, so the unit tests can run the whole program without a console.
#pragma once

#include <iosfwd>
#include <string_view>
#include <vector>

namespace sscc {

/// Runs the program with the arguments that follow the program name, and
/// returns the exit status: 0 if all labels are valid, 1 if any label is
/// invalid, and 2 if the command line is wrong or a file cannot be read.
/// Without arguments, it reads the codes from `in` until an empty line.
[[nodiscard]] int run(const std::vector<std::string_view>& args, std::istream& in,
                      std::ostream& out, std::ostream& err);

}  // namespace sscc
