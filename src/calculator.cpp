#include "calculator.hpp"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Recursive descent parser
//
// Grammar:
//   expr    = term   (('+' | '-') term)*
//   term    = factor (('*' | '/') factor)*
//   factor  = ('+' | '-')? primary
//   primary = NUMBER | '(' expr ')'
//   NUMBER  = [0-9]+ ('.' [0-9]+)?
// ---------------------------------------------------------------------------

namespace {

class Parser {
  public:
    explicit Parser(const std::string& input) : input_(input), pos_(0) {}

    double ParseExpr() {
        double left = ParseTerm();
        while (true) {
            SkipWhitespace();
            if (Peek() == '+') {
                Advance();
                left += ParseTerm();
            } else if (Peek() == '-') {
                Advance();
                left -= ParseTerm();
            } else {
                break;
            }
        }
        return left;
    }

    // Returns the current position (used to detect trailing garbage).
    size_t Position() const { return pos_; }

  private:
    const std::string& input_;
    size_t pos_;

    char Peek() const {
        if (pos_ < input_.size()) return input_[pos_];
        return '\0';
    }

    char Advance() { return input_[pos_++]; }

    void SkipWhitespace() {
        while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_]))) {
            ++pos_;
        }
    }

    double ParseTerm() {
        double left = ParseFactor();
        while (true) {
            SkipWhitespace();
            if (Peek() == '*') {
                Advance();
                left *= ParseFactor();
            } else if (Peek() == '/') {
                Advance();
                double right = ParseFactor();
                if (right == 0.0) {
                    throw std::runtime_error("Division by zero");
                }
                left /= right;
            } else {
                break;
            }
        }
        return left;
    }

    double ParseFactor() {
        SkipWhitespace();
        if (Peek() == '-') {
            Advance();
            return -ParsePrimary();
        }
        if (Peek() == '+') {
            Advance();
        }
        return ParsePrimary();
    }

    double ParsePrimary() {
        SkipWhitespace();
        if (Peek() == '(') {
            Advance();  // consume '('
            double val = ParseExpr();
            SkipWhitespace();
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing ')'");
            }
            Advance();  // consume ')'
            return val;
        }
        return ParseNumber();
    }

    double ParseNumber() {
        SkipWhitespace();
        size_t start = pos_;
        if (pos_ >= input_.size() ||
            !std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            throw std::runtime_error("Expected a number");
        }
        while (pos_ < input_.size() &&
               std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            ++pos_;
        }
        if (pos_ < input_.size() && input_[pos_] == '.') {
            ++pos_;  // consume '.'
            while (pos_ < input_.size() &&
                   std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
        }
        return std::stod(input_.substr(start, pos_ - start));
    }
};

// Format a double without unnecessary trailing zeros.
// e.g. 7.0 -> "7", 3.14 -> "3.14", 1e20 -> "1e+20"
std::string FormatValue(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}

}  // namespace

// ---------------------------------------------------------------------------
// Calculator public interface
// ---------------------------------------------------------------------------

EvaluationResult Calculator::Evaluate(const std::string& expression) {
    if (expression.empty()) {
        return {false, 0.0, "Empty expression"};
    }

    try {
        Parser parser(expression);
        double value = parser.ParseExpr();

        // Reject trailing garbage after a valid expression (e.g. "3 + 4 abc").
        size_t pos = parser.Position();
        while (pos < expression.size() &&
               std::isspace(static_cast<unsigned char>(expression[pos]))) {
            ++pos;
        }
        if (pos < expression.size()) {
            std::string msg = "Unexpected character: '";
            msg += expression[pos];
            msg += "'";
            return {false, 0.0, msg};
        }

        if (!std::isfinite(value)) {
            return {false, 0.0, "Result is not finite"};
        }

        history_.emplace_back(expression, FormatValue(value));
        return {true, value, ""};

    } catch (const std::exception& e) {
        return {false, 0.0, e.what()};
    }
}

void Calculator::ClearHistory() { history_.clear(); }

const std::vector<std::pair<std::string, std::string>>& Calculator::GetHistory() const {
    return history_;
}
