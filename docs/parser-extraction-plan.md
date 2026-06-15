# Parser Extraction Plan
## Refactoring `Parser` from `calculator.cpp` into `src/model/parser.cpp`

---

## 1. What Exists Today

Inside `src/model/calculator.cpp`, the `Parser` class and the `FormatValue` helper
currently live in an **anonymous namespace** — meaning they are file-local and invisible
to any other translation unit:

```
src/model/calculator.cpp
├── namespace { ... }          ← anonymous — INVISIBLE outside this file
│   ├── class Parser { ... }   ← recursive descent parser
│   └── FormatValue(double)    ← display formatting helper
└── Calculator::Evaluate()     ← uses Parser and FormatValue
└── Calculator::ClearHistory()
└── Calculator::GetHistory()
```

**Consequence:** To split `Parser` into `parser.cpp`, it must be given a **named** home
(a header + implementation file) so `calculator.cpp` can `#include` it. The anonymous
namespace trick no longer works across translation units.

---

## 2. Design Decisions

### 2.1 Should `Parser` get its own header?

**Yes.** This is the only way `calculator.cpp` can use it after the split.
`parser.hpp` declares the class; `parser.cpp` defines all the methods.

### 2.2 Should `Parser` be public API or internal?

`Parser` is an implementation detail of `Calculator` — it should never be used by
controller, view, or test code directly. To communicate this:

- Use the `model::internal` namespace convention (matches Google's recommendation
  from the style guide: *"use namespaces with 'internal' in the name to document
  parts of an API that should not be mentioned by users of the API"*).
- Keep the header in `src/model/` alongside `calculator.hpp` — not in a public `include/`.

```cpp
// parser.hpp
namespace model::internal {
class Parser { ... };
}  // namespace model::internal
```

### 2.3 Where does `FormatValue` go?

`FormatValue` is also a `Calculator` implementation detail (formats a `double` for
the history display string). It is **not** parser logic — it does not belong in
`parser.hpp/cpp`.

It moves to a dedicated `src/model/format_helpers.hpp` (inline helper, header-only)
so both `calculator.cpp` and any future model code can use it without a circular dep.

Alternatively (simpler): keep `FormatValue` as a `static` function inside
`calculator.cpp` — it has no reason to be shared and adding a whole file for a
single `snprintf` wrapper is over-engineering.

**Decision: keep `FormatValue` as a `static` function in `calculator.cpp`.**
Only `Parser` is extracted.

### 2.4 Does `CMakeLists.txt` change?

Yes — `parser.cpp` must be added to the `calc_lib` source list.

---

## 3. Target File Structure

```
src/model/
├── app_state.hpp           # unchanged
├── calculator.hpp          # unchanged (public API)
├── calculator.cpp          # trimmed: removes Parser class, keeps Evaluate/Clear/GetHistory
├── parser.hpp              # NEW: declares Parser in model::internal namespace
└── parser.cpp              # NEW: defines all Parser methods
```

---

## 4. `parser.hpp` Design

```cpp
#pragma once

#include <cstddef>   // size_t
#include <string>

namespace model::internal {

// Recursive descent parser for infix arithmetic expressions.
//
// Grammar:
//   expr    = term   (('+' | '-') term)*
//   term    = factor (('*' | '/') factor)*
//   factor  = ('+' | '-')? primary
//   primary = NUMBER | '(' expr ')'
//   NUMBER  = [0-9]+ ('.' [0-9]+)?
//
// Usage:
//   Parser p(input);
//   double result = p.ParseExpr();   // throws std::runtime_error on syntax error
//   size_t end    = p.Position();    // check for trailing garbage
class Parser {
  public:
    explicit Parser(const std::string& input);

    // Parses a complete expression and returns its value.
    // Throws std::runtime_error on any syntax error.
    double ParseExpr();

    // Returns the index of the first unconsumed character.
    // Caller uses this to detect trailing garbage after a valid expression.
    size_t Position() const;

  private:
    const std::string& input_;
    size_t pos_;

    char   Peek() const;
    char   Advance();
    void   SkipWhitespace();
    double ParseTerm();
    double ParseFactor();
    double ParsePrimary();
    double ParseNumber();
};

}  // namespace model::internal
```

---

## 5. `parser.cpp` Design

```cpp
#include "model/parser.hpp"

#include <cctype>
#include <stdexcept>
#include <string>

namespace model::internal {

// ... all method bodies move here verbatim from calculator.cpp ...

}  // namespace model::internal
```

---

## 6. Updated `calculator.cpp`

```cpp
#include "model/calculator.hpp"
#include "model/parser.hpp"     // NEW include

#include <cctype>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

// File-local helper (not parser logic — stays here)
static std::string FormatValue(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}

EvaluationResult Calculator::Evaluate(const std::string& expression) {
    if (expression.empty()) {
        return {false, 0.0, "Empty expression"};
    }
    try {
        model::internal::Parser parser(expression);  // qualified name
        double value = parser.ParseExpr();
        // ... trailing garbage check, isfinite check, history append (unchanged)
    } catch (const std::exception& e) {
        return {false, 0.0, e.what()};
    }
}

// ClearHistory and GetHistory unchanged
```

---

## 7. Updated `CMakeLists.txt`

```cmake
# Layer 1: Model (no FTXUI)
add_library(calc_lib
    src/model/calculator.cpp
    src/model/parser.cpp       # NEW
)
target_include_directories(calc_lib PUBLIC src/)
```

---

## 8. Test Impact

**No test changes required.**

- `Parser` is in `model::internal` — tests never use it directly.
- `Calculator` public API is unchanged.
- All 16 existing test cases and 61 assertions continue to pass as-is.
- If parser-specific unit tests are ever desired, they can `#include "model/parser.hpp"`
  and instantiate `model::internal::Parser` directly.

---

## 9. Implementation Checklist

- [ ] Create `src/model/parser.hpp` — declare `Parser` in `model::internal` namespace
- [ ] Create `src/model/parser.cpp` — move all `Parser` method bodies from `calculator.cpp`
- [ ] Update `src/model/calculator.cpp`:
  - Remove the anonymous namespace block containing `Parser`
  - Add `#include "model/parser.hpp"`
  - Change `Parser parser(expression)` to `model::internal::Parser parser(expression)`
  - Move `FormatValue` from anonymous namespace to file-local `static` function
- [ ] Update `CMakeLists.txt` — add `src/model/parser.cpp` to `calc_lib` sources
- [ ] Build: `cmake --build build`
- [ ] Verify: `./build/run_tests` — all 61 assertions pass

---

## 10. Key Conventions Applied

| Decision | Rationale |
|----------|-----------|
| `model::internal` namespace | Google style: signals internal API, prevents accidental use |
| `parser.hpp` in `src/model/` | Co-located with the code it supports, not in a public `include/` |
| `FormatValue` stays in `calculator.cpp` as `static` | Single-use helper — no benefit to a shared file |
| No test changes | `Parser` was always an impl detail; extraction doesn't change observable behaviour |
| `add_library(calc_lib src/model/calculator.cpp src/model/parser.cpp)` | Both compile as part of the same `calc_lib` target |
