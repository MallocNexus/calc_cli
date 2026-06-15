#include "util/formatting.hpp"

#include <cstdio>
#include <string>

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
            if (!out.empty() && out.back() != ' ' && out.back() != '(' && out.back() != '-' && out.back() != '+') out += ' ';
            out += c;
            prev_is_operator = true;
        } else if (c == ')') {
            if (!out.empty() && out.back() == ' ') out.pop_back();
            out += c;
            prev_is_operator = false;
        } else {
            out += c;
            prev_is_operator = false;
        }
    }

    if (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

}  // namespace util
