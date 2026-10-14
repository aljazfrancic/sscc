# Project Plan

This is the plan for the SSCC task described in [instructions.md](instructions.md), based on the initial development notes. Items are ticked off as the work progresses.

## Ground rules

- Write this plan before doing anything else.
- Never commit or push without asking first.
- Target C++17, use as many of its features as fit naturally, and mark each one with a `// C++17:` comment.
- Write useful comments that explain _why_ and refer to the GS1 rules.
- Write all code and documentation in English, in full sentences.
- Keep the program small and simple, so that the tooling does not overshadow it.

## Checklist

### Phase 0: Documentation

- [x] Translate the task description into English, verbatim, as [instructions.md](instructions.md).
- [x] Turn the development notes into this plan.
- [x] Keep the first commit to these two Markdown files.

### Phase 1: Build setup and "Hello, World!"

- [x] Decide between CMake and Make (see [Decisions](#decisions)).
- [x] Install the VS Code extensions C/C++ and CMake Tools.
- [x] Install CMake, Ninja and Clang.
- [x] Write a "Hello, World!" program with a minimal `CMakeLists.txt` that requires C++17.
- [x] Add the VS Code configuration, and check that the program can be built and debugged in VS Code.

### Phase 2: SSCC validator

- [x] Accept the scanner output as it is: the AI followed by the SSCC, without `[FNC1]`.
- [x] Check that the input contains only digits and is 20 digits long (a 2-digit AI and an 18-digit SSCC).
- [x] Check that the AI is `00` (SSCC), and name other known AIs, such as `02`.
- [x] Split the SSCC into the extension digit, the company prefix, the serial reference and the check digit.
- [x] Check that the company prefix is the TSX prefix `34260311`.
- [x] Verify the check digit with the GS1 modulo-10 algorithm.
- [x] Report every problem of a label and say exactly what is wrong (Bonus 1).
- [x] Print a clear, user-friendly verdict for each label to the console.

### Phase 3: Input (Bonus 2)

- [x] Read codes from a text file, one code per line, with the six examples in `data/examples.txt`.
- [x] Read codes that the user types into the console.
- [x] Show usage help with `--help`.

### Phase 4: Unit tests (Bonus 3)

- [x] Add GoogleTest 1.18.0 with `FetchContent`, and register the tests with CTest.
- [x] Test the check digit, each type of error, a label with several errors, and the six examples.

### Phase 5: Continuous integration

- [x] Build and test in a GitHub Actions matrix: GCC and Clang on Linux, MSVC on Windows, each in Debug and Release.
- [x] Treat compiler warnings as errors, and add a CI badge to the README.

### Phase 6: Final documentation and publishing

- [x] Write `README.md` with build instructions, usage, the results, and notes on GS1 and C++17.
- [x] Review the code against the ground rules.
- [x] Add the GitHub "About" description and topics, with `gh` or in the web UI.
- [x] Make the repository public.

## Decisions

- **CMake instead of Make.** CMake is overkill for "Hello, World!", but the project also needs several compilers, Windows, a CI matrix and GoogleTest. Make cannot build with MSVC without a second build script, while a single `CMakeLists.txt` covers everything and still generates Makefiles on Linux.
- **Input from both a file and the console.** The file holds the six examples, and the console lets a user check any label. A GUI is not needed, because the task allows a CLI.
- **`main()` only calls `sscc::run()`.** `run()` gets the arguments and the streams as parameters, so the unit tests can run the whole program without a console.
- **Exit status as in grep and diff.** 0 means that all labels are valid, 1 that a label is invalid, and 2 that the command line is wrong or a file cannot be read.
- **GoogleTest for the unit tests.** It is the most widely used C++ test framework and works well with CMake, CTest and VS Code. `FetchContent` pins its version, so nothing has to be installed, and version 1.18 itself requires C++17.
- **Only AI `00` (SSCC) is accepted.** Any other AI makes a label invalid, and the message names that AI from a small table of common AIs. A general GS1 parser for all AIs is out of scope, because it would need fixed and variable field lengths, the invisible FNC1 separators (lost in the Notepad captures) and separate checks for each AI.
- **Windows Mobile Notepad does not matter.** It is only how the samples were captured. What matters is their format: the AI is included, but `[FNC1]` is not.
- **GCC, Clang and MSVC.** The code uses standard C++17 only and is tested with all three compilers in CI.
- **Debug and Release in CI.** Release is what users run, and its optimizer finds more warnings. Debug keeps the asserts and, with MSVC, the bounds checks of the standard library.

## C++17 features

These are the candidates, each used only where it fits: `std::string_view`, `std::optional`, `std::variant` with `std::visit`, structured bindings, `if` and `switch` with initializers, `[[nodiscard]]`, `inline constexpr` variables, nested namespaces, `constexpr` lambdas with `static_assert`, `std::filesystem`, class template argument deduction and fold expressions. All of them are used except `switch` with an initializer, which had no natural place, and the README lists where.

## Expected results

A TSX label has 20 digits: the AI `00`, an extension digit, the company prefix `34260311`, an 8-digit serial reference and a check digit. Worked out by hand, the six examples give these results:

| #   | Scanned data           | Result                                                                                   |
| :-: | ---------------------- | ---------------------------------------------------------------------------------------- |
| 1   | `00034260311130776594` | Valid.                                                                                   |
| 2   | `02044260311130776512` | Invalid: the AI is `02`, the company prefix is `44260311`, and the check digit should be `7`. |
| 3   | `00034260311130776144` | Invalid: the check digit should be `3`.                                                  |
| 4   | `00034260311130776570` | Valid.                                                                                   |
| 5   | `0003426031113077646`  | Invalid: the code has 19 digits instead of 20.                                           |
| 6   | `00034260321130774636` | Invalid: the company prefix is `34260321`, although the check digit is correct.          |

## Glossary

| Term                   | Meaning                                                                                                                                                 |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| TSX GmbH               | The company from the task; GmbH is the German form of a limited liability company.                                                                      |
| Euro pallet            | The standard European pallet (1200 × 800 mm); each pallet gets its own SSCC label.                                                                      |
| SSCC                   | Serial Shipping Container Code: an 18-digit, globally unique number of a logistic unit, such as a pallet.                                               |
| GS1-128                | A barcode standard based on Code 128, which encodes AIs together with their data.                                                                      |
| Company ID             | The GS1 Company Prefix that GS1 assigns to a company; for TSX it is `34260311`.                                                                         |
| RF terminal            | A handheld computer with a barcode scanner and a wireless network connection.                                                                           |
| Windows Mobile Notepad | A text editor on the RF terminal, into which the scanner types the scanned codes.                                                                       |
| AI                     | Application Identifier: a 2 to 4 digit prefix that defines the meaning and format of the data after it; `00` means SSCC.                                |
| FNC1                   | A non-printable Code 128 character that marks a symbol as GS1-128 and separates variable-length fields.                                                |
| Check digit            | The last SSCC digit. The other 17 digits are multiplied by 3 and 1 alternately, starting with 3 on the right, and the check digit tops their sum up to a multiple of ten. |

## Open questions

- **Will the interview focus on knowing the standards?** Probably, on both GS1 and C++17, so the README and comments should explain the rules, and every C++17 feature should be easy to justify.
- **Is this overengineered?** A bit, because one `.cpp` file would solve the task. The structure is deliberate, but each part should stay minimal.
