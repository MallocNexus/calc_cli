#include "model/parser.hpp"

#include <cctype>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace model::internal {

Parser::Parser(const std::string& input, RateResolver resolver)
    : input_(input), pos_(0), rate_resolver_(resolver) {}

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

std::string Parser::ParseCurrency() {
    SkipWhitespace();
    std::string currency = "";
    while (pos_ < input_.size() && std::isalpha(static_cast<unsigned char>(input_[pos_]))) {
        currency += input_[pos_];
        ++pos_;
    }
    if (currency.empty()) {
        throw std::runtime_error("Expected currency identifier (e.g. AUD)");
    }
    std::transform(currency.begin(), currency.end(), currency.begin(), ::toupper);
    return currency;
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
    
    // Check if the next token is the keyword "exchange"
    if (input_.compare(pos_, 8, "exchange") == 0) {
        size_t next_pos = pos_ + 8;
        if (next_pos >= input_.size() || !std::isalpha(static_cast<unsigned char>(input_[next_pos]))) {
            pos_ += 8; // consume "exchange"
            SkipWhitespace();
            if (Peek() != '(') {
                throw std::runtime_error("Expected '(' after exchange");
            }
            Advance(); // consume '('
            std::string base = ParseCurrency();
            SkipWhitespace();
            if (Peek() != ',') {
                throw std::runtime_error("Expected ',' after base currency");
            }
            Advance(); // consume ','
            std::string quote = ParseCurrency();
            SkipWhitespace();
            if (Peek() != ')') {
                throw std::runtime_error("Expected ')' after quote currency");
            }
            Advance(); // consume ')'
            
            if (!rate_resolver_) {
                throw std::runtime_error("Rate resolver is not set");
            }
            return rate_resolver_(base, quote);
        }
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

double Parser::ParseSumExpression() {
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

// ---------------------------------------------------------------------------
// Public: top-level entry point
// ---------------------------------------------------------------------------

double Parser::ParseExpr() {
    double left = ParseSumExpression();
    SkipWhitespace();
    if (input_.compare(pos_, 8, "exchange") == 0) {
        size_t next_pos = pos_ + 8;
        if (next_pos >= input_.size() || !std::isalpha(static_cast<unsigned char>(input_[next_pos]))) {
            pos_ += 8; // consume "exchange"
            SkipWhitespace();
            if (Peek() != '(') {
                throw std::runtime_error("Expected '(' after exchange");
            }
            Advance(); // consume '('
            std::string base = ParseCurrency();
            SkipWhitespace();
            if (Peek() != ',') {
                throw std::runtime_error("Expected ',' after base currency");
            }
            Advance(); // consume ','
            std::string quote = ParseCurrency();
            SkipWhitespace();
            if (Peek() != ')') {
                throw std::runtime_error("Expected ')' after quote currency");
            }
            Advance(); // consume ')'
            
            if (!rate_resolver_) {
                throw std::runtime_error("Rate resolver is not set");
            }
            double rate = rate_resolver_(base, quote);
            left *= rate;
        }
    }
    return left;
}

}  // namespace model::internal
