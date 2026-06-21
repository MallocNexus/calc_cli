# Test Suite Split Plan

This plan details the steps to split the monolithic test file [test_main.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/test_main.cpp) into a modular, clean structure under `tests/` that mirrors the directory layout of `src/`. We will create a test file per class/module being tested.

## 1. Directory Structure Mapping

The new structure in `tests/` will mirror the `src/` directory layout:

```text
src/                                  tests/
├── util/                             ├── util/
│   └── formatting.cpp         ➡️     │   └── test_formatting.cpp
├── model/                            ├── model/
│   ├── calculator.cpp         ➡️     │   ├── test_calculator.cpp
│   ├── parser.cpp             ➡️     │   ├── test_parser.cpp
│   ├── history_repository.cpp ➡️     │   ├── test_history_repository.cpp
│   └── exchange_rate.cpp      ➡️     │   └── test_exchange_rate.cpp
├── controller/                       ├── controller/
│   ├── app_controller.cpp     ➡️     │   ├── test_app_controller.cpp
│   ├── history_controller.cpp ➡️     │   ├── test_history_controller.cpp
│   └── exchange_rate_controller.cpp ➡️│   └── test_exchange_rate_controller.cpp
└── view/                             └── view/
    ├── app.cpp                ➡️     │   └── test_app.cpp
    └── custom_exchange.cpp    ➡️     └── test_custom_exchange.cpp
```

---

## 2. Test File Definitions & Mapping

Below is the layout of each test file, detailing the original test cases to move and any new unit test coverage we will add.

### 2.1. Utilities
* **[tests/util/test_formatting.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/util/test_formatting.cpp)**
  * **Tested Module:** `src/util/formatting.cpp`
  * **Test Cases:**
    * `TEST_CASE("Util — FormatExpression", "[util]")`
      * `SECTION("Basic spacing")`
      * `SECTION("Parentheses")`
      * `SECTION("Unary minus and plus")`
      * `SECTION("Exchange function")`

### 2.2. Models
* **[tests/model/test_calculator.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_calculator.cpp)**
  * **Tested Class:** `Calculator` (`src/model/calculator.cpp`)
  * **Test Cases:**
    * `TEST_CASE("Calculator — basic arithmetic", "[calculator]")`
    * `TEST_CASE("Calculator — operator precedence", "[calculator]")`
    * `TEST_CASE("Calculator — unary minus and floats", "[calculator]")`
    * `TEST_CASE("Calculator — error cases", "[calculator]")`

* **[tests/model/test_parser.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_parser.cpp)**
  * **Tested Module:** `src/model/parser.cpp` (specifically expression evaluation with currency resolver injection)
  * **Test Cases:**
    * `TEST_CASE("Parser - exchange function with mock resolver", "[parser]")`

* **[tests/model/test_history_repository.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_history_repository.cpp)**
  * **Tested Class:** `HistoryRepository` (`src/model/history_repository.cpp`)
  * **Test Cases:**
    * `TEST_CASE("HistoryRepository — database operation", "[history]")`
      * Focuses strictly on `HistoryRepository` DB queries: `Initialize()`, `Add()`, `Clear()`, and `GetHistory()`.

* **[tests/model/test_exchange_rate.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/model/test_exchange_rate.cpp)**
  * **Tested Class:** `ExchangeRate` (`src/model/exchange_rate.cpp`)
  * **Test Cases:**
    * `TEST_CASE("ExchangeRate Cache Model", "[exchange_rate]")`
      * `SECTION("Get cached rate for missing pair returns false")`
      * `SECTION("Save and retrieve rate")`

### 2.3. Controllers
* **[tests/controller/test_app_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/controller/test_app_controller.cpp)**
  * **Tested Class:** `AppController` (`src/controller/app_controller.cpp`)
  * **Test Cases:**
    * `TEST_CASE("AppController — OnEvaluate", "[controller]")`
    * `TEST_CASE("AppController — OnClear", "[controller]")`
    * `TEST_CASE("AppController — OnQuit", "[controller]")`
    * `TEST_CASE("AppController — modal flags", "[controller]")`
    * `TEST_CASE("AppController — OnClearHistory", "[controller]")`

* **[tests/controller/test_history_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/controller/test_history_controller.cpp)**
  * **Tested Class:** `HistoryController` (`src/controller/history_controller.cpp`)
  * **Test Cases:**
    * `TEST_CASE("HistoryController — logic mapping", "[history]")`
      * Verifies that the controller maps method calls correctly to the underlying `HistoryRepository` reference.

* **[tests/controller/test_exchange_rate_controller.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/controller/test_exchange_rate_controller.cpp)**
  * **Tested Class:** `ExchangeRateController` (`src/controller/exchange_rate_controller.cpp`)
  * **Test Cases:**
    * `TEST_CASE("ExchangeRateController — caching and fallback logic", "[exchange_rate_controller]")`
      * Tests cache hits (fresh rate) where cpr API call is skipped.
      * Tests cache stale condition where an API call is made.
      * Tests offline fallback where cpr fails (throws/timeout) but database has a stale cached rate that is returned as a fallback.

### 2.4. Views
* **[tests/view/test_app.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_app.cpp)**
  * **Tested Class:** `App` (`src/view/app.cpp`)
  * **Test Cases:**
    * `TEST_CASE("App — evaluate expression via keypress", "[app]")`
    * `TEST_CASE("App — error display on bad expression", "[app]")`
    * `TEST_CASE("App — Escape clears input and result", "[app]")`
    * `TEST_CASE("App — Edit -> Clear via menu", "[app]")`
    * `TEST_CASE("App — Edit -> Clear History via menu", "[app]")`
    * `TEST_CASE("App — Help -> Version opens modal", "[app]")`
    * `TEST_CASE("App — File -> Quit calls on_quit", "[app]")`
    * `TEST_CASE("App — Exchange -> AUD -> USD inserts shorthand", "[app]")`

* **[tests/view/test_custom_exchange.cpp](file:///Users/mattswart/Source/CPP/calc-cli/tests/view/test_custom_exchange.cpp)**
  * **Tested Class:** `CustomExchange` (`src/view/custom_exchange.cpp`)
  * **Test Cases:**
    * `TEST_CASE("CustomExchange View — interaction", "[custom_exchange]")`
      * Verifies direct component rendering with state.
      * Verifies Escape key event triggers `show_custom_modal = false`.
      * Verifies filling inputs and submitting inserts the expected `exchange(src, dst)` string at `cursor_position` inside `AppState`.

---

## 3. Build Configuration (CMake)

We will modify [CMakeLists.txt](file:///Users/mattswart/Source/CPP/calc-cli/CMakeLists.txt) to compile all separated test files, replacing the reference to `tests/test_main.cpp`.

```cmake
# ---------------------------------------------------------------------------
# Test executable
# ---------------------------------------------------------------------------
add_executable(run_tests
    tests/util/test_formatting.cpp
    tests/model/test_calculator.cpp
    tests/model/test_parser.cpp
    tests/model/test_history_repository.cpp
    tests/model/test_exchange_rate.cpp
    tests/controller/test_app_controller.cpp
    tests/controller/test_history_controller.cpp
    tests/controller/test_exchange_rate_controller.cpp
    tests/view/test_app.cpp
    tests/view/test_custom_exchange.cpp
)
```

Catch2 is linked via `Catch2::Catch2WithMain`, so Catch2 provides its own `main` entry point; no manual test entry runner file is needed.

---

## 4. Implementation Steps & Checklist

- [x] Create the target subdirectories under `tests/` (`util/`, `model/`, `controller/`, `view/`).
- [x] Implement `tests/util/test_formatting.cpp` (formatting functions).
- [x] Implement `tests/model/test_calculator.cpp` (arithmetic/errors).
- [x] Implement `tests/model/test_parser.cpp` (currency expression parser).
- [x] Implement `tests/model/test_history_repository.cpp` (database operations).
- [x] Implement `tests/model/test_exchange_rate.cpp` (cache DB).
- [x] Implement `tests/controller/test_app_controller.cpp` (app flow coordination).
- [x] Implement `tests/controller/test_history_controller.cpp` (history controller logic).
- [x] Implement `tests/controller/test_exchange_rate_controller.cpp` (API & cache stale/hit/offline fallback).
- [x] Implement `tests/view/test_app.cpp` (ftxui component interaction).
- [x] Implement `tests/view/test_custom_exchange.cpp` (modal dialog event verification).
- [x] Remove `tests/test_main.cpp`.
- [x] Update `CMakeLists.txt` to compile all target files.
- [x] Configure CMake and build `run_tests` to verify that all tests compile and pass.
