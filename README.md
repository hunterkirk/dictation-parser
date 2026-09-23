# dictation-parser

A standalone native macOS command-line utility that converts Whisper.cpp speech-to-text transcripts into text formatted similarly to macOS Dictation.

The program runs **after Whisper.cpp** and has no dependency on Whisper.cpp, Python, CMake, or any runtime libraries beyond the macOS system toolchain.

## Overview

```text
Audio
  │
  ▼
Whisper.cpp
  │
  │ transcript.txt
  ▼
dictation-parser
  │
  │ formatted text
  ▼
output.txt
```

The parser interprets spoken Dictation commands such as:

```text
hello comma world period
```

and produces:

```text
hello, world.
```

It also maintains formatting state for commands such as:

```text
caps on
caps off
all caps
all caps on
all caps off
no space on
no space off
```

The long-term goal is to closely reproduce useful portions of macOS Dictation's text-formatting behavior while remaining a simple standalone executable.

---

## Requirements

Currently targeted platform:

* macOS
* Apple Silicon (`arm64`)
* Apple Clang
* C++20

The project intentionally has no third-party runtime dependencies.

You need Apple's command-line developer tools:

```bash
xcode-select --install
```

Verify the compiler:

```bash
clang++ --version
```

---

## Project Structure

```text
dictation-parser/
├── Makefile
├── README.md
├── SESSION.md
├── src/
│   ├── main.cpp
│   ├── DictationParser.cpp
│   └── DictationParser.h
└── tests/
```

---

## Building

The project uses a simple Makefile.

Normal build:

```bash
make
```

Clean build:

```bash
make rebuild
```

Clean:

```bash
make clean
```

Debug build:

```bash
make debug
```

The resulting executable is:

```text
build/dictation-parser
```

Verify the architecture:

```bash
file build/dictation-parser
```

Expected:

```text
Mach-O 64-bit executable arm64
```

---

## Usage

### Parse a file

```bash
./build/dictation-parser transcript.txt
```

The formatted result is written to stdout.

### Write to a file

```bash
./build/dictation-parser transcript.txt -o formatted.txt
```

### Read from stdin

```bash
cat transcript.txt | ./build/dictation-parser -
```

### Pipeline directly from Whisper.cpp

The intended workflow is:

```bash
whisper-cli ... | ./build/dictation-parser - > formatted.txt
```

This keeps Whisper.cpp and the formatting parser completely independent.

---

## Installation

Install the executable into `~/bin`:

```bash
make install
```

If `~/bin` is not already in your PATH:

```bash
export PATH="$HOME/bin:$PATH"
```

Then:

```bash
dictation-parser transcript.txt
```

---

# Supported Commands

The parser is intended to support the useful text-formatting commands available through macOS Dictation.

## Punctuation

| Spoken command         | Output                |
| ---------------------- | --------------------- |
| `apostrophe`           | `'`                   |
| `open square bracket`  | `[`                   |
| `close square bracket` | `]`                   |
| `open parenthesis`     | `(`                   |
| `close parenthesis`    | `)`                   |
| `open brace`           | `{`                   |
| `close brace`          | `}`                   |
| `open angle bracket`   | `<`                   |
| `close angle bracket`  | `>`                   |
| `colon`                | `:`                   |
| `comma`                | `,`                   |
| `dash`                 | `–` / normalized dash |
| `ellipsis`             | `…`                   |
| `exclamation mark`     | `!`                   |
| `hyphen`               | `-`                   |
| `period`               | `.`                   |
| `point`                | `.`                   |
| `dot`                  | `.`                   |
| `full stop`            | `.`                   |
| `question mark`        | `?`                   |
| `quote`                | `"`                   |
| `end quote`            | `"`                   |
| `begin single quote`   | `'`                   |
| `end single quote`     | `'`                   |
| `semicolon`            | `;`                   |

## Typography

| Spoken command     | Output |   |
| ------------------ | ------ | - |
| `ampersand`        | `&`    |   |
| `asterisk`         | `*`    |   |
| `at sign`          | `@`    |   |
| `backslash`        | `\`    |   |
| `forward slash`    | `/`    |   |
| `caret`            | `^`    |   |
| `center dot`       | `・`    |   |
| `large center dot` | `●`    |   |
| `degree sign`      | `°`    |   |
| `hashtag`          | `#`    |   |
| `pound sign`       | `#`    |   |
| `percent sign`     | `%`    |   |
| `underscore`       | `_`    |   |
| `vertical bar`     | `      | ` |

## Formatting

| Spoken command  | Behavior                       |
| --------------- | ------------------------------ |
| `new line`      | New line                       |
| `new paragraph` | Blank line / new paragraph     |
| `tab key`       | Tab                            |
| `no space on`   | Disable automatic word spacing |
| `no space off`  | Restore automatic word spacing |

## Capitalization

| Spoken command | Behavior                                  |
| -------------- | ----------------------------------------- |
| `caps on`      | Title-style capitalization until disabled |
| `caps off`     | Disable caps mode                         |
| `all caps`     | Capitalize the next word                  |
| `all caps on`  | ALL CAPS until disabled                   |
| `all caps off` | Disable ALL CAPS mode                     |

## Numbers

The parser is designed to recognize spoken numbers such as:

```text
one
twenty
twenty-three
one hundred
one hundred twenty-three
one thousand two hundred
```

Examples:

```text
numeral one hundred twenty-three
```

produces:

```text
123
```

The number grammar should eventually support substantially more cases, including large numbers and mixed numeric expressions.

## Roman Numerals

Example:

```text
roman numeral twenty-three
```

produces:

```text
XXIII
```

The current Roman numeral implementation supports values from:

```text
1–3999
```

## Ordinals

The parser is intended to recognize:

```text
first
second
third
fourth
twenty-first
one hundred twenty-third
```

and produce numeric ordinals such as:

```text
1st
2nd
3rd
4th
21st
123rd
```

## Dates

The parser supports spoken date patterns such as:

```text
January fifth twenty twenty-six
```

and attempts to produce:

```text
January 5th, 2026
```

Date parsing is still an area that requires additional test coverage and refinement.

## Times

Examples intended to work include:

```text
three PM
three thirty PM
three o'clock
three o'clock PM
noon
midnight
```

producing forms such as:

```text
3:00 PM
3:30 PM
3:00 PM
12:00 PM
12:00 AM
```

Time parsing also requires additional test coverage.

---

# Spacing

The formatter automatically manages spacing around punctuation.

Input:

```text
hello comma world period
```

Output:

```text
hello, world.
```

Opening punctuation should attach naturally:

```text
open parenthesis hello close parenthesis
```

produces:

```text
(hello)
```

`no space on` disables normal word spacing:

```text
no space on
one two three
no space off
four five
```

is intended to produce:

```text
onetwo three four five
```

The exact semantics of `no space on` still need to be validated against actual macOS Dictation behavior.

---

# Literal Commands

The parser supports a `literal` escape intended to allow a command word to be emitted as ordinary text.

For example:

```text
literal comma
```

should produce:

```text
comma
```

rather than:

```text
,
```

This mechanism needs additional testing and refinement.

---

# Design Principles

## Whisper-independent

The parser must not depend on Whisper.cpp internals.

Whisper is responsible for speech recognition.

This program is responsible for interpreting the resulting transcript.

That separation allows either component to be replaced independently.

## Standalone

The release executable should not require:

* Python
* virtual environments
* Homebrew runtime packages
* Whisper.cpp
* external configuration
* network access

## Deterministic

Given identical input text, the parser should produce identical output.

## Testable

Parser behavior should be testable independently of the CLI.

## Data-driven commands

Simple command substitutions should remain data-driven rather than requiring custom parser code for every punctuation symbol.

Complex constructs such as numbers, dates, times, and capitalization should be implemented as explicit parser states/grammars.

---

# Development

Build:

```bash
make rebuild
```

Run:

```bash
./build/dictation-parser input.txt
```

Debug:

```bash
make debug
```

Check architecture:

```bash
file build/dictation-parser
```

---

# Future Work

Priority areas:

1. Comprehensive parser test suite.
2. More accurate macOS Dictation spacing behavior.
3. Better `no space on/off` semantics.
4. More complete number grammar.
5. Decimal numbers.
6. Fractions.
7. Negative numbers.
8. Currency expressions.
9. Dates with multiple spoken formats.
10. Times and time ranges.
11. Better ordinal handling.
12. Better capitalization/title-case behavior.
13. Command-vs-natural-language disambiguation.
14. Robust `literal` handling.
15. Whisper-specific transcript normalization.
16. Optional JSON/debug output.
17. macOS universal binary support if Intel distribution becomes necessary.
18. Code signing/notarization for distribution.

---

# License

License has not yet been selected.

Do not add a license declaration until the project owner decides which license to use.

