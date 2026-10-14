# SSCC label check

[![CI](https://github.com/aljazfrancic/sscc/actions/workflows/ci.yml/badge.svg)](https://github.com/aljazfrancic/sscc/actions/workflows/ci.yml)

A C++17 console program that checks scanned GS1-128 SSCC labels of TSX GmbH and explains exactly what is wrong with each label. The task is in [instructions.md](instructions.md), and the plan with its decisions is in [plan.md](plan.md).

## Results

Two of the six sample labels are valid:

| #   | Scanned data           | Result                                                                                         |
| :-: | ---------------------- | ---------------------------------------------------------------------------------------------- |
| 1   | `00034260311130776594` | Valid.                                                                                         |
| 2   | `02044260311130776512` | Invalid: the AI is `02`, the company prefix is `44260311`, and the check digit should be `7`.  |
| 3   | `00034260311130776144` | Invalid: the check digit should be `3`.                                                        |
| 4   | `00034260311130776570` | Valid.                                                                                         |
| 5   | `0003426031113077646`  | Invalid: the code has 19 digits instead of 20.                                                 |
| 6   | `00034260321130774636` | Invalid: the company prefix is `34260321`, although the check digit is correct.                |

<details>
<summary>The full output of <code>sscc --file data/examples.txt</code></summary>

```text
SSCC label check for TSX GmbH (GS1 company prefix 34260311)

Label 1
  Code:    00034260311130776594
  Parts:   AI 00 | extension digit 0 | company prefix 34260311 | serial reference 13077659 | check digit 4
  Result:  VALID

Label 2
  Code:    02044260311130776512
  Parts:   AI 02 | extension digit 0 | company prefix 44260311 | serial reference 13077651 | check digit 2
  Result:  INVALID, 3 problems:
           - The code starts with AI 02 (GTIN of contained trade items) instead of AI 00 (SSCC).
           - The company prefix is 44260311, but it should be 34260311 (digit 1 differs).
           - The check digit is 2, but it should be 7.

Label 3
  Code:    00034260311130776144
  Parts:   AI 00 | extension digit 0 | company prefix 34260311 | serial reference 13077614 | check digit 4
  Result:  INVALID, 1 problem:
           - The check digit is 4, but it should be 3.

Label 4
  Code:    00034260311130776570
  Parts:   AI 00 | extension digit 0 | company prefix 34260311 | serial reference 13077657 | check digit 0
  Result:  VALID

Label 5
  Code:    0003426031113077646
  Result:  INVALID, 1 problem:
           - The code has 19 characters instead of 20 (AI 00 and an 18-digit SSCC), so 1 character is missing.

Label 6
  Code:    00034260321130774636
  Parts:   AI 00 | extension digit 0 | company prefix 34260321 | serial reference 13077463 | check digit 6
  Result:  INVALID, 1 problem:
           - The company prefix is 34260321, but it should be 34260311 (digit 7 differs).

Summary: 2 valid, 4 invalid.
```

</details>

## Build and test

You need CMake 3.20 or newer and a C++17 compiler. CI builds and tests with GCC and Clang on Linux and with MSVC on Windows. The first configure downloads GoogleTest 1.18.0 for the unit tests.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release
```

Make and Ninja take the build type when configuring, and Visual Studio when building, so the commands set it in both places, as CI does. Without it, Make and Ninja build without optimization and debug information. For a debug build, write `Debug` instead of `Release`. The program is `build/sscc`, or `build\Release\sscc.exe` with Visual Studio. To build without the tests and the download, add `-DBUILD_TESTING=OFF` to the first command. In VS Code, the CMake Tools extension builds the selected variant (Debug by default) in its own folder, and F5 debugs the program with the examples file.

## Usage

```text
sscc CODE...        Check the given codes.
sscc --file FILE    Check the codes in a text file, one code per line.
sscc                Check the codes that you type or scan, one per line.
sscc --help         Show this help.
```

A code is the AI `00` followed by the 18-digit SSCC, exactly as the scanner sends it, without FNC1. Codes and files can be combined, blank lines in files are skipped, and an empty line ends the console input. As with grep and diff, the exit status is 0 if all labels are valid, 1 if a label is invalid, and 2 if the command line is wrong or a file cannot be read.

## How a label is checked

A TSX label holds 20 digits, as in example 1:

| Digits | Field              | Example 1  |
| :----: | ------------------ | ---------- |
| 1–2    | AI `00` (SSCC)     | `00`       |
| 3      | Extension digit    | `0`        |
| 4–11   | GS1 company prefix | `34260311` |
| 12–19  | Serial reference   | `13077659` |
| 20     | Check digit        | `4`        |

The program reports every problem of a label, not only the first one:

1. **Characters:** only digits are allowed. Any other character is named with its position, for example a space or the GS character that a scanner sends for FNC1.
2. **Length:** the code must have 20 digits. Otherwise the fields cannot be located, and only the AI is checked further.
3. **AI:** the code must start with `00`, the AI of an SSCC. Common other AIs are named, such as `02` for the GTIN of contained trade items.
4. **Company prefix:** digits 4 to 11 must be `34260311`, and the digits that differ are listed.
5. **Check digit:** GS1 modulo 10. The 17 digits before the check digit are weighted 3, 1, 3, 1, … from the right, and the check digit tops their sum up to a multiple of ten. In example 1 the sum is 136, so the check digit is 4. The check digit does not cover the AI.

## C++17 features

Each feature is marked with a `// C++17:` comment where it is used.

| Feature                                                                        | Where                                                 |
| ------------------------------------------------------------------------------ | ----------------------------------------------------- |
| `std::string_view` and its `sv` literals                                       | The codes, without copies; `cli.cpp`, `main.cpp`      |
| `std::optional`                                                                | `gs1::aiTitle()`, `ValidationResult::parts`           |
| `std::variant` with `std::visit`                                               | `Problem` in `sscc.hpp`, `describe()` in `report.cpp` |
| The overloaded idiom: `using` with a pack, a deduction guide, aggregate bases  | `report.cpp`                                          |
| Class template argument deduction                                              | `gs1::kCommonAis`                                     |
| Structured bindings                                                            | `report.cpp`, `cli_test.cpp`                          |
| `if` with an initializer                                                       | `sscc.cpp`                                            |
| Fold expression                                                                | `isAnyOf()` in `sscc.cpp`                             |
| `inline constexpr` variables                                                   | `gs1.hpp`, `sscc.hpp`                                 |
| `constexpr` lambda and `static_assert` without a message                       | `gs1::checkDigit()`                                   |
| Nested namespace definition                                                    | `namespace sscc::gs1`                                 |
| `[[nodiscard]]`                                                                | Every public function that returns a result           |
| `std::filesystem`                                                              | Reading files in `cli.cpp`                            |

## Project layout

| Path                     | Contents                                                                    |
| ------------------------ | --------------------------------------------------------------------------- |
| `src/gs1.hpp`            | General GS1 rules: the check digit, checked at compile time, and AI titles. |
| `src/sscc.hpp`, `.cpp`   | `validate()`: the checks, without any input or output.                      |
| `src/report.hpp`, `.cpp` | The English sentence for each problem and the report for each label.        |
| `src/cli.hpp`, `.cpp`    | `run()`: the command line, files and console input.                         |
| `src/main.cpp`           | `main()`, which passes the real console to `run()`.                         |
| `tests/`                 | GoogleTest unit tests for each source file.                                 |
| `data/examples.txt`      | The six sample labels from the task.                                        |

## License

[MIT](LICENSE)
