#pragma once

#include <string>

namespace util {

// Converts a double to a display string without unnecessary trailing zeros.
//
// Uses the "%g" format specifier which automatically selects the shorter of
// fixed and scientific notation and strips trailing zeros.
//
// Examples:
//   FormatDouble(7.0)    -> "7"
//   FormatDouble(3.14)   -> "3.14"
//   FormatDouble(1e20)   -> "1e+20"
//   FormatDouble(-0.5)   -> "-0.5"
std::string FormatDouble(double value);

// Normalizes spacing of a mathematical expression.
//
// Examples:
//   FormatExpression("2+2")           -> "2 + 2"
//   FormatExpression("3 * ( 4-1 )")   -> "3 * (4 - 1)"
//   FormatExpression("-5 + -2")       -> "-5 + -2"
std::string FormatExpression(const std::string& input);

}  // namespace util
