# SSCC Task

> Verbatim English translation of the original task description.

## Introduction

The company TSX GmbH uses euro pallets in its warehouse operations, which are now labelled with SSCC barcodes according to the GS1-128 standard. The registered company ID of TSX with the GS1-128 organization is `34260311`. They ordered a few samples of the new barcodes from a local printing company. At first glance the barcodes look OK, but they are not sure whether the data was generated correctly.

Users scanned 6 sample labels with an RF terminal into Windows Mobile Notepad. Some are OK, some are not. When scanning, the Application Identifier (AI) is also printed in Notepad, but `[FNC1]` is not printed.

| Example | Scanned data           |
| :-----: | ---------------------- |
|    1    | `00034260311130776594` |
|    2    | `02044260311130776512` |
|    3    | `00034260311130776144` |
|    4    | `00034260311130776570` |
|    5    | `0003426031113077646`  |
|    6    | `00034260321130774636` |

## Task

Write a program in the C++ language that checks whether the given examples of scanned SSCC barcodes comply with the GS1-128 specification. While doing so, try to use as many features of the C++17 standard as possible.

- The results should be user-friendly and should be printed to the console.
- **Bonus 1:** The program should print what exactly is wrong with the content of the label.
- **Bonus 2:** The program should also support user input (CLI or GUI).
- **Bonus 3:** Write a unit test that verifies the correctness of the program.

Read more about the GS1-128 standard here:

- <https://en.wikipedia.org/wiki/GS1-128>
- <https://www.gs1-128.info/sscc-18/>
