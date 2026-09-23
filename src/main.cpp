#include "DictationParser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


static void printUsage(
    const char* program
) {
    std::cerr
        << "Usage:\n"
        << "  " << program
        << " input.txt [-o output.txt]\n"
        << "  " << program
        << " - [-o output.txt]\n"
        << "\n"
        << "Use '-' as the input filename to read stdin.\n"
        << "If -o is omitted, formatted text is written to stdout.\n";
}


static bool readInput(
    const std::string& filename,
    std::string& result
) {
    if (filename == "-") {
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        result = buffer.str();
        return true;
    }

    std::ifstream file(
        filename,
        std::ios::binary
    );

    if (!file)
        return false;

    std::ostringstream buffer;
    buffer << file.rdbuf();

    result = buffer.str();

    return true;
}


static bool writeOutput(
    const std::string& filename,
    const std::string& text
) {
    if (filename == "-") {
        std::cout << text;
        return true;
    }

    std::ofstream file(
        filename,
        std::ios::binary
    );

    if (!file)
        return false;

    file << text;

    return static_cast<bool>(file);
}


int main(int argc, char* argv[]) {

    if (argc < 2) {
        printUsage(argv[0]);
        return 2;
    }

    std::string inputFile;
    std::string outputFile = "-";

    for (int i = 1; i < argc; ++i) {

        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }

        if (arg == "--version" || arg == "-v") {
            std::cout
                << "dictation-parser 1.0.0\n";
            return 0;
        }

        if (arg == "-o") {

            if (i + 1 >= argc) {
                std::cerr
                    << "error: -o requires a filename\n";
                return 2;
            }

            outputFile = argv[++i];
            continue;
        }

        if (inputFile.empty()) {
            inputFile = arg;
            continue;
        }

        std::cerr
            << "error: unexpected argument: "
            << arg << '\n';

        return 2;
    }

    if (inputFile.empty()) {
        printUsage(argv[0]);
        return 2;
    }

    std::string input;

    if (!readInput(inputFile, input)) {
        std::cerr
            << "error: unable to read input: "
            << inputFile << '\n';
        return 1;
    }

    DictationParser parser;

    std::string output =
        parser.parse(input);

    if (!writeOutput(outputFile, output)) {
        std::cerr
            << "error: unable to write output: "
            << outputFile << '\n';
        return 1;
    }

    return 0;
}
