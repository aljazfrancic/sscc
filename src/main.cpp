// Placeholder that checks the CMake build and the VS Code debugger.
// The SSCC validator replaces it in the next phase.

#include <iostream>
#include <string_view>

int main()
{
    // C++17: std::string_view, a cheap non-owning view of a string.
    constexpr std::string_view greeting{"Hello, World!"};
    std::cout << greeting << '\n';

    // 201703 means that the compiler really builds as C++17.
    std::cout << "C++ standard: " << __cplusplus << '\n';
    return 0;
}
