#pragma once

#include <string>
#include <utility>
#include <vector>
#include <functional>

using RateResolver = std::function<double(const std::string&, const std::string&)>;

// Result of a single expression evaluation.
// On success: ok == true and value holds the computed result.
// On failure: ok == false and error holds a human-readable message.
struct EvaluationResult {
    bool ok;
    double value;
    std::string error;
};

// Calculator evaluates infix arithmetic expressions and tracks history.
// This class has NO FTXUI dependency and can be tested in complete isolation.
class Calculator {
  public:
    // Evaluates `expression`.
    EvaluationResult Evaluate(const std::string& expression, RateResolver resolver = nullptr);

  private:
    // Internal recursive descent parser.
    EvaluationResult ParseAndEvaluate(const std::string& expr);
};
