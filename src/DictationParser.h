#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class DictationParser {
public:
    DictationParser();

    std::string parse(std::string_view input);

private:
    struct State {
        bool capsOn = false;
        bool allCapsOn = false;
        bool noSpaceOn = false;

        bool allCapsNext = false;
        bool titleNext = false;
    };

    struct Token {
        std::string text;

        enum class Kind {
            Word,
            Number,
            Symbol,
            Newline
        };

        Kind kind;

        std::string lower() const;
    };

    State state_;

    std::vector<Token> tokenize(std::string_view text) const;

    std::string parseTokens(
        const std::vector<Token>& tokens
    );

    bool match(
        const std::vector<Token>& tokens,
        size_t index,
        std::string_view phrase
    ) const;

    std::pair<std::string, size_t> tryCommand(
        const std::vector<Token>& tokens,
        size_t index
    ) const;

    std::pair<std::string, size_t> consumeNumber(
        const std::vector<Token>& tokens,
        size_t index
    ) const;

    std::pair<std::string, size_t> tryDate(
        const std::vector<Token>& tokens,
        size_t index
    ) const;

    std::pair<std::string, size_t> tryTime(
        const std::vector<Token>& tokens,
        size_t index
    ) const;

    void add(
        std::string& output,
        const std::string& value
    );

    std::string formatWord(
        std::string word
    );

    static std::string normalizeUnicode(
        std::string text
    );

    static std::string cleanup(
        std::string text
    );

    static void stripTrailingSpaces(
        std::string& text
    );

    static bool isNumberWord(
        std::string_view word
    );

    static bool isOrdinalWord(
        std::string_view word
    );

    static bool isMonth(
        std::string_view word
    );

    static int parseIntegerWords(
        const std::vector<std::string>& words,
        bool& success
    );

    static std::optional<std::string> parseDecimal(
        const std::vector<std::string>& words
    );

    static std::optional<int> parseOrdinal(
        const std::vector<std::string>& words
    );

    static std::string ordinalSuffix(int number);

    static std::optional<std::string> toRoman(
        int number
    );

    static std::string smartTitle(
        const std::vector<std::string>& words
    );
};
