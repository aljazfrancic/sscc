// Console program that checks scanned SSCC labels and explains every problem.
// main() only connects the program to the real console. Everything else is in
// sscc::run(), so the unit tests can run the program with their own streams.

#include <iostream>
#include <string_view>
#include <vector>

#include "cli.hpp"

int main(int argc, char* argv[])
{
    // C++17: std::string_view refers to each argument without copying it.
    // argc is 0 if the program was started without even its own name.
    const std::vector<std::string_view> args(argc > 0 ? argv + 1 : argv, argv + argc);
    return sscc::run(args, std::cin, std::cout, std::cerr);
}
