#include "DictationParser.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <map>
#include <optional>
#include <regex>
#include <sstream>
#include <unordered_map>

namespace {

const std::unordered_map<std::string, int> UNITS = {
    {"zero", 0},
    {"one", 1},
    {"two", 2},
    {"three", 3},
    {"four", 4},
    {"five", 5},
    {"six", 6},
    {"seven", 7},
    {"eight", 8},
    {"nine", 9},
    {"ten", 10},
    {"eleven", 11},
    {"twelve", 12},
    {"thirteen", 13},
    {"fourteen", 14},
    {"fifteen", 15},
    {"sixteen", 16},
    {"seventeen", 17},
    {"eighteen", 18},
    {"nineteen", 19}
};

const std::unordered_map<std::string, int> TENS = {
    {"twenty", 20},
    {"thirty", 30},
    {"forty", 40},
    {"fifty", 50},
    {"sixty", 60},
    {"seventy", 70},
    {"eighty", 80},
    {"ninety", 90}
};

const std::unordered_map<std::string, long long> SCALES = {
    {"hundred", 100},
    {"thousand", 1000},
    {"million", 1000000},
    {"billion", 1000000000LL},
    {"trillion", 1000000000000LL}
};

const std::unordered_map<std::string, int> ORDINAL_SMALL = {
    {"zeroth", 0},
    {"first", 1},
    {"second", 2},
    {"third", 3},
    {"fourth", 4},
    {"fifth", 5},
    {"sixth", 6},
    {"seventh", 7},
    {"eighth", 8},
    {"ninth", 9},
    {"tenth", 10},
    {"eleventh", 11},
    {"twelfth", 12},
    {"thirteenth", 13},
    {"fourteenth", 14},
    {"fifteenth", 15},
    {"sixteenth", 16},
    {"seventeenth", 17},
    {"eighteenth", 18},
    {"nineteenth", 19}
};

const std::unordered_map<std::string, int> ORDINAL_TENS = {
    {"twentieth", 20},
    {"thirtieth", 30},
    {"fortieth", 40},
    {"fiftieth", 50},
    {"sixtieth", 60},
    {"seventieth", 70},
    {"eightieth", 80},
    {"ninetieth", 90}
};

const std::unordered_map<std::string, int> ORDINAL_SCALES = {
    {"hundredth", 100},
    {"thousandth", 1000},
    {"millionth", 1000000},
    {"billionth", 1000000000}
};

const std::unordered_map<std::string, int> MONTHS = {
    {"january", 1},
    {"february", 2},
    {"march", 3},
    {"april", 4},
    {"may", 5},
    {"june", 6},
    {"july", 7},
    {"august", 8},
    {"september", 9},
    {"october", 10},
    {"november", 11},
    {"december", 12}
};

const std::unordered_map<std::string, std::string> COMMANDS = {
    {"apostrophe", "'"},
    {"open square bracket", "["},
    {"close square bracket", "]"},
    {"open parenthesis", "("},
    {"close parenthesis", ")"},
    {"open brace", "{"},
    {"close brace", "}"},
    {"open angle bracket", "<"},
    {"close angle bracket", ">"},
    {"colon", ":"},
    {"comma", ","},
    {"dash", "–"},
    {"ellipsis", "…"},
    {"exclamation mark", "!"},
    {"exclamation point", "!"},
    {"hyphen", "-"},
    {"period", "."},
    // {"point", "."},
    {"dot", "."},
    {"full stop", "."},
    {"question mark", "?"},
    {"quote", "\""},
    {"end quote", "\""},
    {"begin single quote", "'"},
    {"end single quote", "'"},
    {"semicolon", ";"},

    {"ampersand", "&"},
    {"asterisk", "*"},
    {"at sign", "@"},
    {"backslash", "\\"},
    {"forward slash", "/"},
    {"caret", "^"},
    {"center dot", "・"},
    {"large center dot", "●"},
    {"degree sign", "°"},
    {"hashtag", "#"},
    {"pound sign", "#"},
    {"percent sign", "%"},
    {"underscore", "_"},
    {"vertical bar", "|"},

    {"equal sign", "="},
    {"greater than sign", ">"},
    {"less than sign", "<"},
    {"minus sign", "-"},
    {"multiplication sign", "×"},
    {"plus sign", "+"},

    {"dollar sign", "$"},
    {"cent sign", "¢"},
    {"pound sterling", "£"},
    {"euro sign", "€"},
    {"yen sign", "¥"},

    {"smiley face", ":-)"},
    {"frowny face", ":-("},
    {"winky face", ";-)"},
    {"cross-eyed laughing face", "XD"},

    {"copyright sign", "©"},
    {"registered sign", "®"},
    {"trademark sign", "™"},

    {"new line", "\n"},
    {"new paragraph", "\n\n"},
    {"tab key", "\t"}
};

const std::unordered_map<std::string, std::string> TIME_WORDS = {
    {"noon", "12:00 PM"},
    {"midnight", "12:00 AM"}
};

std::vector<std::string> splitHyphenated(
    const std::string& word
) {
    std::vector<std::string> result;
    std::string current;

    for (char c : word) {
        if (c == '-') {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(
                static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c))
                )
            );
        }
    }

    if (!current.empty())
        result.push_back(current);

    return result;
}

bool isAsciiWordChar(char c) {
    return std::isalpha(
        static_cast<unsigned char>(c)
    );
}

bool isAsciiDigit(char c) {
    return std::isdigit(
        static_cast<unsigned char>(c)
    );
}

} // namespace


DictationParser::DictationParser() = default;


std::string DictationParser::Token::lower() const {
    std::string result = text;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return result;
}


std::string DictationParser::normalizeUnicode(
    std::string text
) {
    /*
     * We intentionally don't perform general Unicode normalization here.
     *
     * The parser works primarily with ASCII command words and emits
     * UTF-8 symbols. This keeps the executable dependency-free.
     */

    const std::vector<std::pair<std::string, std::string>> replacements = {
        {"\xE2\x80\x98", "'"},   // ‘
        {"\xE2\x80\x99", "'"},   // ’
        {"\xE2\x80\x9C", "\""}, // “
        {"\xE2\x80\x9D", "\""}, // ”
        {"\xE2\x80\x93", "-"},   // –
        {"\xE2\x80\x94", "-"},   // —
    };

    for (const auto& [from, to] : replacements) {
        size_t pos = 0;

        while ((pos = text.find(from, pos)) != std::string::npos) {
            text.replace(pos, from.size(), to);
            pos += to.size();
        }
    }

    return text;
}


std::vector<DictationParser::Token>
DictationParser::tokenize(std::string_view input) const {
    std::string text = normalizeUnicode(std::string(input));

    std::vector<Token> tokens;

    size_t i = 0;

    while (i < text.size()) {

        if (text[i] == '\n' || text[i] == '\r') {
            if (
                text[i] == '\r' &&
                i + 1 < text.size() &&
                text[i + 1] == '\n'
            ) {
                ++i;
            }

            tokens.push_back({
                "\n",
                Token::Kind::Newline
            });

            ++i;
            continue;
        }

        if (std::isspace(
                static_cast<unsigned char>(text[i])
            )) {
            ++i;
            continue;
        }

        if (isAsciiWordChar(text[i])) {

            size_t start = i;

            while (i < text.size()) {

                if (isAsciiWordChar(text[i])) {
                    ++i;
                    continue;
                }

                if (
                    text[i] == '-' &&
                    i + 1 < text.size() &&
                    isAsciiWordChar(text[i + 1])
                ) {
                    ++i;
                    continue;
                }

                if (
                    text[i] == '\'' &&
                    i + 1 < text.size() &&
                    isAsciiWordChar(text[i + 1])
                ) {
                    ++i;
                    continue;
                }

                break;
            }

            tokens.push_back({
                text.substr(start, i - start),
                Token::Kind::Word
            });

            continue;
        }

        if (isAsciiDigit(text[i])) {

            size_t start = i;

            while (
                i < text.size() &&
                (
                    isAsciiDigit(text[i]) ||
                    text[i] == '.'
                )
            ) {
                ++i;
            }

            tokens.push_back({
                text.substr(start, i - start),
                Token::Kind::Number
            });

            continue;
        }

        tokens.push_back({
            std::string(1, text[i]),
            Token::Kind::Symbol
        });

        ++i;
    }

    return tokens;
}


bool DictationParser::match(
    const std::vector<Token>& tokens,
    size_t index,
    std::string_view phrase
) const {
    std::istringstream stream{std::string(phrase)};

    std::vector<std::string> words;
    std::string word;

    while (stream >> word)
        words.push_back(word);

    if (index + words.size() > tokens.size())
        return false;

    for (size_t i = 0; i < words.size(); ++i) {
        if (tokens[index + i].lower() != words[i])
            return false;
    }

    return true;
}


bool DictationParser::isNumberWord(
    std::string_view word
) {
    std::string w(word);

    if (
        UNITS.contains(w) ||
        TENS.contains(w) ||
        SCALES.contains(w)
    ) {
        return true;
    }

    return w == "and" ||
           w == "point" ||
           w == "negative" ||
           w == "minus";
}


bool DictationParser::isOrdinalWord(
    std::string_view word
) {
    std::string w(word);

    return ORDINAL_SMALL.contains(w) ||
           ORDINAL_TENS.contains(w) ||
           ORDINAL_SCALES.contains(w);
}


bool DictationParser::isMonth(
    std::string_view word
) {
    return MONTHS.contains(std::string(word));
}


int DictationParser::parseIntegerWords(
    const std::vector<std::string>& words,
    bool& success
) {
    success = false;

    if (words.empty())
        return 0;

    long long total = 0;
    long long current = 0;

    bool negative = false;
    bool found = false;

    for (size_t i = 0; i < words.size(); ++i) {

        std::string word = words[i];

        if (word.find('-') != std::string::npos) {
            auto pieces = splitHyphenated(word);

            std::vector<std::string> expanded;

            for (const auto& p : pieces)
                expanded.push_back(p);

            std::vector<std::string> remaining(
                expanded.begin(),
                expanded.end()
            );

            bool nestedSuccess = false;
            int nested = parseIntegerWords(
                remaining,
                nestedSuccess
            );

            if (!nestedSuccess)
                return 0;

            current += nested;
            found = true;
            continue;
        }

        if (word == "negative" || word == "minus") {
            if (i != 0)
                return 0;

            negative = true;
            continue;
        }

        if (word == "and")
            continue;

        auto unit = UNITS.find(word);

        if (unit != UNITS.end()) {
            current += unit->second;
            found = true;
            continue;
        }

        auto ten = TENS.find(word);

        if (ten != TENS.end()) {
            current += ten->second;
            found = true;
            continue;
        }

        if (word == "hundred") {

            if (!found)
                return 0;

            if (current == 0)
                current = 1;

            current *= 100;
            continue;
        }

        auto scale = SCALES.find(word);

        if (scale != SCALES.end()) {

            if (!found)
                return 0;

            total += current * scale->second;
            current = 0;
            continue;
        }

        return 0;
    }

    if (!found)
        return 0;

    long long result = total + current;

    if (negative)
        result = -result;

    success = true;

    return static_cast<int>(result);
}


std::optional<std::string>
DictationParser::parseDecimal(
    const std::vector<std::string>& words
) {
    auto point = std::find(
        words.begin(),
        words.end(),
        "point"
    );

    if (point == words.end())
        return std::nullopt;

    std::vector<std::string> integerWords(
        words.begin(),
        point
    );

    std::vector<std::string> fractionWords(
        point + 1,
        words.end()
    );

    bool success = false;

    int integer = parseIntegerWords(
        integerWords,
        success
    );

    if (!success)
        return std::nullopt;

    std::string result =
        std::to_string(integer) + ".";

    for (const auto& word : fractionWords) {

        auto pieces = splitHyphenated(word);

        for (const auto& piece : pieces) {

            auto unit = UNITS.find(piece);

            if (unit == UNITS.end())
                return std::nullopt;

            result += std::to_string(unit->second);
        }
    }

    return result;
}


std::optional<int>
DictationParser::parseOrdinal(
    const std::vector<std::string>& words
) {
    if (words.empty())
        return std::nullopt;

    std::vector<std::string> expanded;

    for (const auto& word : words) {

        auto pieces = splitHyphenated(word);

        expanded.insert(
            expanded.end(),
            pieces.begin(),
            pieces.end()
        );
    }

    const std::string& last = expanded.back();

    auto small = ORDINAL_SMALL.find(last);

    if (small != ORDINAL_SMALL.end()) {

        if (expanded.size() == 1)
            return small->second;

        std::vector<std::string> base(
            expanded.begin(),
            expanded.end() - 1
        );

        bool success = false;

        int value = parseIntegerWords(
            base,
            success
        );

        if (!success)
            return std::nullopt;

        return value + small->second;
    }

    auto tens = ORDINAL_TENS.find(last);

    if (tens != ORDINAL_TENS.end()) {

        if (expanded.size() == 1)
            return tens->second;

        std::vector<std::string> base(
            expanded.begin(),
            expanded.end() - 1
        );

        bool success = false;

        int value = parseIntegerWords(
            base,
            success
        );

        if (!success)
            return std::nullopt;

        return value + tens->second;
    }

    auto scale = ORDINAL_SCALES.find(last);

    if (scale != ORDINAL_SCALES.end()) {

        if (expanded.size() == 1)
            return scale->second;

        std::vector<std::string> base(
            expanded.begin(),
            expanded.end() - 1
        );

        bool success = false;

        int value = parseIntegerWords(
            base,
            success
        );

        if (!success)
            return std::nullopt;

        return value * scale->second;
    }

    return std::nullopt;
}


std::string DictationParser::ordinalSuffix(
    int number
) {
    int n100 = number % 100;

    if (n100 >= 10 && n100 <= 20)
        return "th";

    switch (number % 10) {
        case 1: return "st";
        case 2: return "nd";
        case 3: return "rd";
        default: return "th";
    }
}


std::optional<std::string>
DictationParser::toRoman(int number) {
    if (number < 1 || number > 3999)
        return std::nullopt;

    const std::vector<std::pair<int, std::string>> values = {
        {1000, "M"},
        {900, "CM"},
        {500, "D"},
        {400, "CD"},
        {100, "C"},
        {90, "XC"},
        {50, "L"},
        {40, "XL"},
        {10, "X"},
        {9, "IX"},
        {5, "V"},
        {4, "IV"},
        {1, "I"}
    };

    std::string result;

    for (const auto& [value, symbol] : values) {

        while (number >= value) {
            result += symbol;
            number -= value;
        }
    }

    return result;
}


std::pair<std::string, size_t>
DictationParser::consumeNumber(
    const std::vector<Token>& tokens,
    size_t index
) const {
    std::vector<std::string> words;

    size_t i = index;

    while (i < tokens.size()) {

        const auto& token = tokens[i];

        if (
            token.kind != Token::Kind::Word &&
            token.kind != Token::Kind::Number
        ) {
            break;
        }

        std::string lower = token.lower();

        if (
            token.kind == Token::Kind::Number ||
            isNumberWord(lower) ||
            isOrdinalWord(lower)
        ) {
            words.push_back(lower);
            ++i;
            continue;
        }

        break;
    }

    if (words.empty())
        return {"", index};

    if (
        words.size() == 1 &&
        !words[0].empty() &&
        std::all_of(
            words[0].begin(),
            words[0].end(),
            [](char c) {
                return std::isdigit(
                    static_cast<unsigned char>(c)
                );
            }
        )
    ) {
        return {words[0], i};
    }

    if (auto decimal = parseDecimal(words))
        return {*decimal, i};

    bool hasOrdinal = false;

    for (const auto& word : words) {
        if (isOrdinalWord(word)) {
            hasOrdinal = true;
            break;
        }
    }

    if (hasOrdinal) {

        auto ordinal = parseOrdinal(words);

        if (ordinal) {
            return {
                std::to_string(*ordinal) +
                ordinalSuffix(*ordinal),
                i
            };
        }
    }

    bool success = false;

    int value = parseIntegerWords(
        words,
        success
    );

    if (success)
        return {std::to_string(value), i};

    return {"", index};
}


std::pair<std::string, size_t>
DictationParser::tryCommand(
    const std::vector<Token>& tokens,
    size_t index
) const {
    std::vector<std::pair<std::string, std::string>> commands;

    commands.reserve(COMMANDS.size());

    for (const auto& [phrase, replacement] : COMMANDS)
        commands.emplace_back(phrase, replacement);

    std::sort(
        commands.begin(),
        commands.end(),
        [](const auto& a, const auto& b) {
            return a.first.size() > b.first.size();
        }
    );

    for (const auto& [phrase, replacement] : commands) {

        if (match(tokens, index, phrase)) {

            size_t count =
                std::count(
                    phrase.begin(),
                    phrase.end(),
                    ' '
                ) + 1;

            return {
                replacement,
                index + count
            };
        }
    }

    return {"", index};
}


std::pair<std::string, size_t>
DictationParser::tryDate(
    const std::vector<Token>& tokens,
    size_t index
) const {
    if (index >= tokens.size())
        return {"", index};

    std::string month = tokens[index].lower();

    if (!isMonth(month))
        return {"", index};

    size_t i = index + 1;

    std::vector<std::string> dayWords;

    while (i < tokens.size()) {

        const auto& token = tokens[i];

        if (token.kind == Token::Kind::Number) {
            dayWords.push_back(token.lower());
            ++i;
            break;
        }

        if (token.kind != Token::Kind::Word)
            break;

        if (
            isNumberWord(token.lower()) ||
            isOrdinalWord(token.lower())
        ) {
            dayWords.push_back(token.lower());
            ++i;
        } else {
            break;
        }

        auto ordinal = parseOrdinal(dayWords);

        if (
            ordinal &&
            *ordinal >= 1 &&
            *ordinal <= 31
        ) {
            break;
        }
    }

    if (dayWords.empty())
        return {"", index};

    std::optional<int> day =
        parseOrdinal(dayWords);

    if (!day) {
        bool success = false;
        int value = parseIntegerWords(
            dayWords,
            success
        );

        if (success)
            day = value;
    }

    if (
        !day ||
        *day < 1 ||
        *day > 31
    ) {
        return {"", index};
    }

    // Optional "of".
    if (
        i < tokens.size() &&
        tokens[i].lower() == "of"
    ) {
        ++i;
    }

    std::vector<std::string> yearWords;

    while (i < tokens.size()) {

        const auto& token = tokens[i];

        if (token.kind == Token::Kind::Number) {
            yearWords.push_back(token.lower());
            ++i;
            continue;
        }

        if (
            token.kind == Token::Kind::Word &&
            (
                isNumberWord(token.lower()) ||
                isOrdinalWord(token.lower())
            )
        ) {
            yearWords.push_back(token.lower());
            ++i;
            continue;
        }

        break;
    }

    std::optional<int> year;

    if (!yearWords.empty()) {

        bool success = false;

        int value = parseIntegerWords(
            yearWords,
            success
        );

        if (success)
            year = value;
    }

    std::string displayMonth = month;

    displayMonth[0] =
        static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(
                    displayMonth[0]
                )
            )
        );

    std::string result =
        displayMonth + " " +
        std::to_string(*day) +
        ordinalSuffix(*day);

    if (year)
        result += ", " + std::to_string(*year);

    return {result, i};
}


std::pair<std::string, size_t>
DictationParser::tryTime(
    const std::vector<Token>& tokens,
    size_t index
) const {
    if (index >= tokens.size())
        return {"", index};

    const std::string first = tokens[index].lower();

    if (
        first == "noon" ||
        first == "midnight"
    ) {
        return {
            TIME_WORDS.at(first),
            index + 1
        };
    }

    // We deliberately require either AM/PM or a second number
    // to prevent ordinary numbers from becoming times.
    std::vector<std::string> hourWords;

    size_t i = index;

    while (
        i < tokens.size() &&
        hourWords.size() < 2
    ) {
        const auto& token = tokens[i];

        if (token.kind == Token::Kind::Number) {
            hourWords.push_back(token.lower());
            ++i;
            continue;
        }

        if (
            token.kind == Token::Kind::Word &&
            isNumberWord(token.lower())
        ) {
            hourWords.push_back(token.lower());
            ++i;
            continue;
        }

        break;
    }

    if (hourWords.empty())
        return {"", index};

    bool success = false;

    int hour = parseIntegerWords(
        hourWords,
        success
    );

    if (!success || hour < 0 || hour > 24)
        return {"", index};

    // "three o'clock"
    if (
        i < tokens.size() &&
        tokens[i].lower() == "o'clock"
    ) {
        ++i;

        std::string suffix;

        if (i < tokens.size()) {
            std::string ampm = tokens[i].lower();

            if (ampm == "am" || ampm == "pm") {
                suffix =
                    ampm == "am" ? "AM" : "PM";
                ++i;
            }
        }

        int displayHour = hour;

        if (!suffix.empty()) {
            if (displayHour == 0)
                displayHour = 12;
            else if (displayHour > 12)
                displayHour -= 12;
        }

        std::string result =
            std::to_string(displayHour) + ":00";

        if (!suffix.empty())
            result += " " + suffix;

        return {result, i};
    }

    // "three PM"
    if (i < tokens.size()) {

        std::string ampm = tokens[i].lower();

        if (ampm == "am" || ampm == "pm") {

            std::string suffix =
                ampm == "am" ? "AM" : "PM";

            ++i;

            int displayHour = hour;

            if (displayHour == 0)
                displayHour = 12;
            else if (displayHour > 12)
                displayHour -= 12;

            return {
                std::to_string(displayHour) +
                ":00 " +
                suffix,
                i
            };
        }
    }

    // "three thirty PM"
    std::vector<std::string> minuteWords;

    while (
        i < tokens.size() &&
        minuteWords.size() < 2
    ) {
        const auto& token = tokens[i];

        if (token.kind == Token::Kind::Number) {
            minuteWords.push_back(token.lower());
            ++i;
            continue;
        }

        if (
            token.kind == Token::Kind::Word &&
            isNumberWord(token.lower())
        ) {
            minuteWords.push_back(token.lower());
            ++i;
            continue;
        }

        break;
    }

    if (minuteWords.empty())
        return {"", index};

    bool minuteSuccess = false;

    int minute = parseIntegerWords(
        minuteWords,
        minuteSuccess
    );

    if (
        !minuteSuccess ||
        minute < 0 ||
        minute > 59
    ) {
        return {"", index};
    }

    std::string suffix;

    if (i < tokens.size()) {

        std::string ampm = tokens[i].lower();

        if (ampm == "am" || ampm == "pm") {
            suffix =
                ampm == "am" ? "AM" : "PM";
            ++i;
        }
    }

    int displayHour = hour;

    if (!suffix.empty()) {
        if (displayHour == 0)
            displayHour = 12;
        else if (displayHour > 12)
            displayHour -= 12;
    }

    std::ostringstream result;

    result << displayHour
           << ':'
           << (minute < 10 ? "0" : "")
           << minute;

    if (!suffix.empty())
        result << ' ' << suffix;

    return {result.str(), i};
}


std::string DictationParser::formatWord(
    std::string word
) {
    if (state_.allCapsNext) {
        state_.allCapsNext = false;

        std::transform(
            word.begin(),
            word.end(),
            word.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::toupper(c)
                );
            }
        );

        return word;
    }

    if (state_.allCapsOn) {

        std::transform(
            word.begin(),
            word.end(),
            word.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::toupper(c)
                );
            }
        );

        return word;
    }

    if (state_.capsOn) {

        bool first = true;

        for (char& c : word) {

            if (first) {
                c = static_cast<char>(
                    std::toupper(
                        static_cast<unsigned char>(c)
                    )
                );

                first = false;
            } else {
                c = static_cast<char>(
                    std::tolower(
                        static_cast<unsigned char>(c)
                    )
                );
            }
        }
    }

    return word;
}


void DictationParser::stripTrailingSpaces(
    std::string& text
) {
    while (
        !text.empty() &&
        (
            text.back() == ' ' ||
            text.back() == '\t'
        )
    ) {
        text.pop_back();
    }
}


void DictationParser::add(
    std::string& output,
    const std::string& value
) {
    if (value.empty())
        return;

    if (value == "\n") {
        stripTrailingSpaces(output);
        output += '\n';
        return;
    }

    if (value == "\n\n") {
        stripTrailingSpaces(output);

        if (
            !output.empty() &&
            output.back() == '\n'
        ) {
            output += '\n';
        } else {
            output += "\n\n";
        }

        return;
    }

    if (value == "\t") {
        stripTrailingSpaces(output);
        output += '\t';
        return;
    }

    const std::string last =
        output.empty()
            ? ""
            : output.substr(output.size() - 1);

    // Punctuation attaches to the previous word.
    if (
        value == "." ||
        value == "," ||
        value == ";" ||
        value == ":" ||
        value == "!" ||
        value == "?" ||
        value == "…" ||
        value == ")" ||
        value == "]" ||
        value == "}" ||
        value == ">" ||
        value == "'" ||
        value == "%" ||
        value == "°"
    ) {
        stripTrailingSpaces(output);
        output += value;
        return;
    }

    // Opening punctuation.
    if (
        value == "(" ||
        value == "[" ||
        value == "{" ||
        value == "<"
    ) {
        if (state_.noSpaceOn)
            stripTrailingSpaces(output);

        output += value;
        return;
    }

    // Symbols that normally don't get surrounding spaces.
    if (
        value == "@" ||
        value == "#" ||
        value == "_" ||
        value == "/" ||
        value == "\\" ||
        value == "^" ||
        value == "*" ||
        value == "=" ||
        value == "+" ||
        value == "-" ||
        value == "×" ||
        value == "|"
    ) {
        stripTrailingSpaces(output);
        output += value;
        return;
    }

    // Currency symbols attach to the number/text following them.
    if (
        last == "$" ||
        last == "£" ||
        last == "€" ||
        last == "¥"
    ) {
        output += value;
        return;
    }

    if (
        state_.noSpaceOn ||
        last == "=" ||
        last == "+" ||
        last == "-" ||
        last == "×"
    ) {
        stripTrailingSpaces(output);
        output += value;
        return;
    }

    if (
        !output.empty() &&
        output.back() != ' ' &&
        output.back() != '\n' &&
        output.back() != '\t'
    ) {
        output += ' ';
    }

    output += value;
}


std::string DictationParser::cleanup(
    std::string text
) {
    // Remove spaces before punctuation.
    text = std::regex_replace(
        text,
        std::regex(R"([ \t]+([,.;:!?]))"),
        "$1"
    );

    // Remove spaces immediately after opening punctuation.
    text = std::regex_replace(
        text,
        std::regex(R"(([\(\[\{<])[ \t]+)"),
        "$1"
    );

    // Remove spaces immediately before closing punctuation.
    text = std::regex_replace(
        text,
        std::regex(R"([ \t]+([\)\]\}>]))"),
        "$1"
    );

    // Collapse repeated spaces.
    text = std::regex_replace(
        text,
        std::regex(R"([ \t]{2,})"),
        " "
    );

    // Maximum two consecutive newlines.
    text = std::regex_replace(
        text,
        std::regex(R"(\n{3,})"),
        "\n\n"
    );

    stripTrailingSpaces(text);

    return text;
}


std::string DictationParser::smartTitle(
    const std::vector<std::string>& words
) {
    static const std::unordered_map<
        std::string,
        bool
    > SMALL = {
        {"a", true},
        {"an", true},
        {"and", true},
        {"as", true},
        {"at", true},
        {"but", true},
        {"by", true},
        {"for", true},
        {"from", true},
        {"in", true},
        {"nor", true},
        {"of", true},
        {"on", true},
        {"or", true},
        {"the", true},
        {"to", true},
        {"up", true},
        {"via", true}
    };

    std::string result;

    for (size_t i = 0; i < words.size(); ++i) {

        std::string word = words[i];

        std::transform(
            word.begin(),
            word.end(),
            word.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );

        bool small =
            i > 0 &&
            i + 1 < words.size() &&
            SMALL.contains(word);

        if (!small && !word.empty()) {
            word[0] = static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(
                        word[0]
                    )
                )
            );
        }

        if (!result.empty())
            result += ' ';

        result += word;
    }

    return result;
}


std::string DictationParser::parseTokens(
    const std::vector<Token>& tokens
) {
    std::string output;

    size_t i = 0;

    while (i < tokens.size()) {

        const auto& token = tokens[i];

        // Preserve actual newlines from Whisper output.
        if (token.kind == Token::Kind::Newline) {
            add(output, "\n");
            ++i;
            continue;
        }

        // Persistent capitalization modes.
        if (match(tokens, i, "caps on")) {
            state_.capsOn = true;
            state_.allCapsOn = false;
            i += 2;
            continue;
        }

        if (match(tokens, i, "caps off")) {
            state_.capsOn = false;
            i += 2;
            continue;
        }

        if (match(tokens, i, "all caps on")) {
            state_.allCapsOn = true;
            state_.capsOn = false;
            i += 3;
            continue;
        }

        if (match(tokens, i, "all caps off")) {
            state_.allCapsOn = false;
            i += 3;
            continue;
        }

        if (match(tokens, i, "no space on")) {
            state_.noSpaceOn = true;
            i += 3;
            continue;
        }

        if (match(tokens, i, "no space off")) {
            state_.noSpaceOn = false;
            i += 3;
            continue;
        }

        // One-shot all caps.
        if (match(tokens, i, "all caps")) {
            state_.allCapsNext = true;
            i += 2;
            continue;
        }

        // Literal command escape.
        if (
            match(tokens, i, "literal") &&
            i + 1 < tokens.size()
        ) {
            auto command =
                tryCommand(tokens, i + 1);

            if (!command.first.empty()) {
                add(
                    output,
                    tokens[i + 1].text
                );

                i = command.second;
                continue;
            }
        }

        // Roman numeral.
        if (match(tokens, i, "roman numeral")) {

            auto [value, next] =
                consumeNumber(tokens, i + 2);

            if (!value.empty()) {

                int number = 0;

                try {
                    number = std::stoi(value);
                }
                catch (...) {
                    number = 0;
                }

                auto roman = toRoman(number);

                if (roman) {
                    add(output, *roman);
                    i = next;
                    continue;
                }
            }
        }

        // Explicit numeral.
        if (match(tokens, i, "numeral")) {

            auto [value, next] =
                consumeNumber(tokens, i + 1);

            if (!value.empty()) {
                add(output, value);
                i = next;
                continue;
            }
        }

        // Dates.
        {
            auto [value, next] =
                tryDate(tokens, i);

            if (!value.empty()) {
                add(output, value);
                i = next;
                continue;
            }
        }

        // Times.
        {
            auto [value, next] =
                tryTime(tokens, i);

            if (!value.empty()) {
                add(output, value);
                i = next;
                continue;
            }
        }

        // Dictation command.
        {
            auto [value, next] =
                tryCommand(tokens, i);

            if (!value.empty()) {
                add(output, value);
                i = next;
                continue;
            }
        }

        // Normal word.
        add(
            output,
            formatWord(token.text)
        );

        ++i;
    }

    return cleanup(output);
}


std::string DictationParser::parse(
    std::string_view input
) {
    state_ = State{};

    auto tokens = tokenize(input);

    return parseTokens(tokens);
}
