#include "model/parser.hpp"

#include <cctype>
#include <stdexcept>
#include <string>

namespace model::internal {

Parser::Parser(const std::string& input) : input_(input), pos_(0) {}

size_t Parser::Position() const { return pos_; }

// ---------------------------------------------------------------------------
// Private: low-level character access
// ---------------------------------------------------------------------------

char Parser::Peek() const {
    if (pos_ < input_.size()) return input_[pos_];
    return '\0';
}

char Parser::Advance() { return input_[pos_++]; }

void Parser::SkipWhitespace() {
    while (pos_ < input_.size() &&
           std::isspace(static_cast<unsigned char>(input_[pos_]))) {
        ++pos_;
    }
}

// ---------------------------------------------------------------------------
// Private: grammar rule implementations
// ---------------------------------------------------------------------------

double Parser::ParseNumber() {
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

double Parser::ParsePrimary() {
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

double Parser::ParseFactor() {
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

double Parser::ParseTerm() {
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

// ---------------------------------------------------------------------------
// Public: top-level entry point
// ---------------------------------------------------------------------------

double Parser::ParseExpr() {
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

}  // namespace model::internal
