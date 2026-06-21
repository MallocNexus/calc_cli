#pragma once

#include <cstddef>
#include <string>
#include <functional>

using RateResolver = std::function<double(const std::string&, const std::string&)>;

namespace service::internal {

// ---------------------------------------------------------------------------
// Parser
// Recursive descent parser for infix arithmetic expressions.
//
// Grammar:
//   expr    = sum_expr [ "exchange" "(" IDENTIFIER "," IDENTIFIER ")" ]
//   sum_expr = term   (('+' | '-') term)*
//   term    = factor (('*' | '/') factor)*
//   factor  = ('+' | '-')? primary
//   primary = NUMBER | '(' expr ')' | "exchange" "(" IDENTIFIER "," IDENTIFIER ")"
//   NUMBER  = [0-9]+ ('.' [0-9]+)?
//
// Usage:
//   Parser p(input, resolver);
//   double result = p.ParseExpr();  // throws std::runtime_error on syntax error
//   size_t end    = p.Position();   // inspect for trailing garbage
//
// This is an internal implementation detail of Calculator.
// It lives in model::internal to signal it is not part of the public API.
// ---------------------------------------------------------------------------
class Parser {
  public:
    explicit Parser(const std::string& input, RateResolver resolver = nullptr);

    // Parses a complete expression and returns its numeric value.
    // Throws std::runtime_error with a human-readable message on any syntax
    // error (unexpected character, mismatched parenthesis, division by zero).
    double ParseExpr();

    // Returns the index of the first unconsumed character after ParseExpr().
    // The caller uses this to detect trailing garbage (e.g. "3 + 4 abc").
    size_t Position() const;

  private:
    const std::string& input_;
    size_t pos_;
    RateResolver rate_resolver_;

    // Low-level character access.
    char Peek() const;
    char Advance();
    void SkipWhitespace();

    // Grammar rule implementations (call each other recursively).
    double ParseSumExpression();
    double ParseTerm();
    double ParseFactor();
    double ParsePrimary();
    double ParseNumber();
    std::string ParseCurrency();
};

}  // namespace service::internal
