// Console program that checks scanned SSCC labels and explains every problem.
// Without arguments it checks the six examples from the task, otherwise each
// argument is checked as a scanned code.

#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

#include "report.hpp"
#include "sscc.hpp"

namespace {

// C++17: std::string_view literals ("..."sv).
using namespace std::string_view_literals;

// The six labels that were scanned in the task (see instructions.md).
// TODO(Phase 3, see plan.md): These built-in examples are temporary. They move
// to data/examples.txt, and running the program without arguments will read
// codes typed into the console instead.
// C++17: class template argument deduction makes this std::array<std::string_view, 6>.
constexpr std::array kTaskExamples{
    "00034260311130776594"sv, "02044260311130776512"sv, "00034260311130776144"sv,
    "00034260311130776570"sv, "0003426031113077646"sv,  "00034260321130774636"sv,
};

}  // namespace

int main(int argc, char* argv[])
{
    std::vector<std::string_view> codes(argv + 1, argv + argc);
    if (codes.empty()) {
        codes.assign(kTaskExamples.begin(), kTaskExamples.end());
    }

    std::cout << "SSCC label check for " << sscc::kTsxCompanyName << " (GS1 company prefix "
              << sscc::kTsxCompanyPrefix << ")\n";

    std::size_t validCount = 0;
    for (std::size_t i = 0; i < codes.size(); ++i) {
        const sscc::ValidationResult result = sscc::validate(codes[i]);
        if (result.isValid()) {
            ++validCount;
        }
        std::cout << "\nLabel " << i + 1 << '\n' << sscc::formatReport(result);
    }
    std::cout << "\nSummary: " << validCount << " of " << codes.size() << " labels are valid.\n";

    // Like other checking tools, the program fails if any label is invalid,
    // so scripts can react to the result.
    return validCount == codes.size() ? EXIT_SUCCESS : EXIT_FAILURE;
}
