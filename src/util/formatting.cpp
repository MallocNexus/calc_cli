#include "util/formatting.hpp"

#include <cstdio>
#include <string>
#include <cctype>

namespace util {

std::string FormatDouble(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}

std::string FormatExpression(const std::string& input) {
    std::string out;
    bool prev_is_operator = true; 

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (std::isspace(static_cast<unsigned char>(c))) continue;

        // Check for "exchange" keyword
        if (input.compare(i, 8, "exchange") == 0) {
            size_t next_pos = i + 8;
            if (next_pos >= input.size() || !std::isalpha(static_cast<unsigned char>(input[next_pos]))) {
                if (!out.empty() && (std::isalnum(static_cast<unsigned char>(out.back())) || out.back() == ')')) {
                    out += ' ';
                }
                out += "exchange";
                i += 7; // skip "exchange" characters (loop will increment i to i+8)
                prev_is_operator = false;
                continue;
            }
        }

        if (c == '+' || c == '-' || c == '*' || c == '/') {
            if ((c == '-' || c == '+') && prev_is_operator) {
                if (!out.empty() && (out.back() == '-' || out.back() == '+')) {
                    out += ' ';
                }
                out += c;
            } else {
                if (!out.empty() && out.back() != ' ' && out.back() != '(') out += ' ';
                out += c;
                out += ' ';
                prev_is_operator = true;
            }
        } else if (c == '(') {
            // Only add a space before '(' if it does not follow "exchange"
            if (!out.empty() && out.back() != ' ' && out.back() != '(' && out.back() != '-' && out.back() != '+' &&
                (out.size() < 8 || out.compare(out.size() - 8, 8, "exchange") != 0)) {
                out += ' ';
            }
            out += c;
            prev_is_operator = true;
        } else if (c == ')') {
            if (!out.empty() && out.back() == ' ') out.pop_back();
            out += c;
            prev_is_operator = false;
        } else if (c == ',') {
            if (!out.empty() && out.back() == ' ') out.pop_back();
            out += ", ";
            prev_is_operator = true;
        } else {
            out += c;
            prev_is_operator = false;
        }
    }

    if (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

}  // namespace util
