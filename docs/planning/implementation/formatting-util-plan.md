# Formatting Utility Extraction Plan
## Refactoring `FormatValue` / `FormatResult` into `src/util/formatting.cpp`

---

## 1. What Exists Today — The Duplication

The same `%g` double-to-string logic exists in **two separate translation units**,
under two different names:

**`src/service/calculator.cpp` (line 9–14):**
```cpp
static std::string FormatValue(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}
```

**`src/controller/app_controller.cpp` (line 15–19):**
```cpp
std::string AppController::FormatResult(double value) const {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}
```

They are identical in behaviour. This is a DRY violation — if the format string ever
needs to change (e.g. to limit decimal places, add thousands separators, or handle
locale), both sites must be updated in sync.

---

## 2. Design Decisions

### 2.1 Class vs. namespace of free functions

The user requested a "util class". Two options:

| Approach | Example | C++ idiom? |
|----------|---------|------------|
| **Class with static methods** | `Formatting::FormatDouble(v)` | ⚠️ Java/C# style — discouraged in C++ (no state, no polymorphism, no reason to instantiate) |
| **Namespace of free functions** | `util::FormatDouble(v)` | ✅ Idiomatic C++ — the standard library, Abseil, and Google's own codebase all use this pattern for stateless utilities |

**Decision: namespace `util`, free function.** This matches Google C++ style and
avoids the anti-pattern of a class that can never be instantiated meaningfully.
The file is still named `formatting.cpp` as requested.

> If a class is ever needed (e.g. to hold configurable formatting options), it can
> be added to the same file — the namespace wrapper stays the same.

### 2.2 What namespace?

`util` — all-lowercase, matches Google style for namespace names.
No nesting needed at this scale (`util::formatting` would be over-scoped for one function).

### 2.3 What does `FormatDouble` do?

Formats a `double` for terminal display without unnecessary trailing zeros:
- `7.0`   → `"7"`
- `3.14`  → `"3.14"`
- `1e20`  → `"1e+20"`
- `-0.5`  → `"-0.5"`

The implementation uses `std::snprintf` with `"%g"` format specifier — unchanged.

### 2.4 Where does `util/` sit in the architecture?

`util/` is a **cross-cutting** layer — it has no dependency on model, controller,
or view, and can be used by any of them:

```
┌─────────────────────────────────────────┐
│                 view/                    │  (FTXUI)
├─────────────────────────────────────────┤
│              controller/                 │  ← uses util::FormatDouble
├─────────────────────────────────────────┤
│                model/                   │  ← uses util::FormatDouble
├─────────────────────────────────────────┤
│                 util/                   │  no deps — pure C++ std only
└─────────────────────────────────────────┘
```

### 2.5 Does `AppController::FormatResult` get removed?

**Yes.** It is replaced by a call to `util::FormatDouble`. The private method
declaration in `app_controller.hpp` is removed too, cleaning up the header.

---

## 3. Target File Structure

```
src/
├── util/                          ← NEW folder
│   ├── formatting.hpp             ← declares util::FormatDouble
│   └── formatting.cpp             ← defines  util::FormatDouble
│
├── model/
│   ├── calculator.cpp             ← removes static FormatValue, adds #include "util/formatting.hpp"
│   └── ... (rest unchanged)
│
└── controller/
    ├── app_controller.hpp         ← removes private FormatResult_ declaration
    └── app_controller.cpp         ← removes FormatResult body, adds #include "util/formatting.hpp"
```

---

## 4. `formatting.hpp` Design

```cpp
#pragma once
#include <string>

namespace util {

// Converts a double to a display string without unnecessary trailing zeros.
//
// Examples:
//   FormatDouble(7.0)   -> "7"
//   FormatDouble(3.14)  -> "3.14"
//   FormatDouble(1e20)  -> "1e+20"
//   FormatDouble(-0.5)  -> "-0.5"
std::string FormatDouble(double value);

}  // namespace util
```

### 5. `formatting.cpp` Design

```cpp
#include "util/formatting.hpp"

#include <cstdio>
#include <string>

namespace util {

std::string FormatDouble(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}

}  // namespace util
```

---

## 6. Callers After Refactor

### `src/service/calculator.cpp`

```cpp
// Before:
static std::string FormatValue(double value) { ... }
// ...
history_.emplace_back(expression, FormatValue(value));

// After:
#include "util/formatting.hpp"
// ...
history_.emplace_back(expression, util::FormatDouble(value));
```

### `src/controller/app_controller.cpp`

```cpp
// Before:
std::string AppController::FormatResult(double value) const { ... }
// ...
state_.result_display = "= " + FormatResult(res.value);

// After:
#include "util/formatting.hpp"
// ...
state_.result_display = "= " + util::FormatDouble(res.value);
```

### `src/controller/app_controller.hpp`

```cpp
// Before:
  private:
    // ...
    std::string FormatResult(double value) const;   // ← remove this line

// After: line deleted entirely
```

---

## 7. Updated `CMakeLists.txt`

```cmake
# Utility library — no FTXUI, no model/controller deps
add_library(util_lib src/util/formatting.cpp)
target_include_directories(util_lib PUBLIC src/)

# Layer 1: Model — now links util_lib
add_library(calc_lib
    src/service/calculator.cpp
    src/service/parser.cpp
)
target_include_directories(calc_lib PUBLIC src/)
target_link_libraries(calc_lib PUBLIC util_lib)

# Layer 2: Controller — gets util_lib transitively via calc_lib,
# but declare it explicitly for clarity
add_library(controller_lib src/controller/app_controller.cpp)
target_include_directories(controller_lib PUBLIC src/)
target_link_libraries(controller_lib PUBLIC calc_lib util_lib)

# Layer 3: View — unchanged
# Main + Tests — unchanged (inherit util_lib transitively)
```

---

## 8. Test Impact

**No test file changes required.**

- `util::FormatDouble` is exercised indirectly through the existing Calculator and
  AppController tests (e.g. `state.result_display == "= 7"` implicitly validates formatting).
- If dedicated unit tests for `FormatDouble` are ever desired, they can
  `#include "util/formatting.hpp"` and test directly — no FTXUI needed.

---

## 9. Implementation Checklist

- [ ] Create `src/util/formatting.hpp` — declare `util::FormatDouble`
- [ ] Create `src/util/formatting.cpp` — define `util::FormatDouble`
- [ ] Update `src/service/calculator.cpp`:
  - Remove `static FormatValue` function
  - Add `#include "util/formatting.hpp"`
  - Replace `FormatValue(value)` → `util::FormatDouble(value)`
- [ ] Update `src/controller/app_controller.hpp`:
  - Remove `std::string FormatResult(double value) const;` private declaration
- [ ] Update `src/controller/app_controller.cpp`:
  - Remove `AppController::FormatResult` method body
  - Add `#include "util/formatting.hpp"`
  - Replace `FormatResult(res.value)` → `util::FormatDouble(res.value)`
- [ ] Update `CMakeLists.txt`:
  - Add `util_lib` target
  - Link `calc_lib` and `controller_lib` against `util_lib`
- [ ] Build: `cmake --build build`
- [ ] Verify: `./build/run_tests` — all 61 assertions pass
