# Plan: `src/util/constants.hpp` — Phased Implementation

## Overview

**23 constants** extracted in **4 phases**. Each phase follows the same structure:
1. **Implementation** — create/update source files
2. **New test cases** — what to write and where
3. **Testing & validation** — build, run, verify
4. **Human review** — engineer confirms changes before proceeding

**Deferred to future pass:** UI strings, error messages, CLI flags, formatting
→ See [`docs/future-refactoring-ideas.md`](file:///Users/mattswart/Source/CPP/calc-cli/docs/future-refactoring-ideas.md)

---

## Pre-Flight: Create the Header

Before any phase begins, create the empty scaffolded file:

**Create:** `src/util/constants.hpp`
```cpp
#pragma once

#include <cstddef>
#include <string_view>

namespace calc_cli {

// Constants are added here phase by phase.
// See docs/future-refactoring-ideas.md for deferred constants.

}  // namespace calc_cli
```

> No build step needed yet — the file is empty. Each phase fills a section and immediately validates.

---

## Phase 1 · `kFileSys*` — File System Paths

> **6 constants** · Affects: `main.cpp`
> ⚠️ Prefix renamed from `kFs*` → `kFileSys*` in Phase 1.1 — see below.

### Step 1 — Implementation

**Add to `src/util/constants.hpp`:**
```cpp
// ── File System ────────────────────────────────────────────────────────────
constexpr std::string_view kFileSysEnvHome                = "HOME";
constexpr std::string_view kFileSysEnvUserProfile         = "USERPROFILE";
constexpr std::string_view kFileSysAppDataSubdir          = "/.calc_cli";
constexpr std::string_view kFileSysAppDataFallbackDir     = ".calc_cli";
constexpr std::string_view kFileSysHistoryDbFilename      = "/calc_history.db";
constexpr std::string_view kFileSysExchangeRateDbFilename = "/exchange_rate.db";
```

**Update `src/main.cpp`:**
- Add `#include "util/constants.hpp"`
- Replace all 6 literals in `GetDatabasePath()` and `GetExchangeRateDatabasePath()`
  with their `calc_cli::kFileSys*` equivalents

### Step 2 — New Test Cases

**File:** `tests/util/test_constants.cpp` *(new file)*

| Test | What it asserts |
|---|---|
| `FileSysConstants_EnvVarNames` | `kFileSysEnvHome == "HOME"` and `kFileSysEnvUserProfile == "USERPROFILE"` |
| `FileSysConstants_AppDataPaths` | `kFileSysAppDataSubdir` starts with `/`, `kFileSysAppDataFallbackDir` has no `/` prefix |
| `FileSysConstants_DbFilenames` | Both filenames start with `/` and end with `.db` |
| `FileSysConstants_PathConcatenation` | `std::string("/home/user") + std::string(kFileSysAppDataSubdir) + std::string(kFileSysHistoryDbFilename)` == `"/home/user/.calc_cli/calc_history.db"` |

### Step 3 — Testing & Validation

```bash
# Build
cmake --build build/

# Run new constants test
./build/run_tests "[constants]"

# Confirm no raw literals remain in main.cpp for fs constants
grep -n '"HOME"\|"USERPROFILE"\|"/.calc_cli"\|".calc_cli"\|"calc_history.db"\|"exchange_rate.db"' src/main.cpp
# Expected: no matches
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Phase 1.1 begins.
>
> Checklist:
> - [ ] `src/util/constants.hpp` — 6 `kFileSys*` constants look correct
> - [ ] `src/main.cpp` — `GetDatabasePath()` and `GetExchangeRateDatabasePath()` use constants, no raw literals remain
> - [ ] `tests/util/test_constants.cpp` — 4 new test cases present and meaningful
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 1 looks good, proceed to 1.1"_

---

## Phase 1.1 · Rename `kFs*` → `kFileSys*`

> **Naming clarity fix** · `kFs` is ambiguous — `kFileSys` makes the intent immediately obvious.
> Affects: `src/util/constants.hpp`, `src/main.cpp`, `tests/util/test_constants.cpp`

### Step 1 — Implementation

Rename all 6 constants in `src/util/constants.hpp`:

| Old name | New name |
|---|---|
| `kFsEnvHome` | `kFileSysEnvHome` |
| `kFsEnvUserProfile` | `kFileSysEnvUserProfile` |
| `kFsAppDataSubdir` | `kFileSysAppDataSubdir` |
| `kFsAppDataFallbackDir` | `kFileSysAppDataFallbackDir` |
| `kFsHistoryDbFilename` | `kFileSysHistoryDbFilename` |
| `kFsExchangeRateDbFilename` | `kFileSysExchangeRateDbFilename` |

Update all call sites in `src/main.cpp` and `tests/util/test_constants.cpp`.

### Step 2 — No new test cases

This is a pure rename — existing `FileSysConstants_*` tests already cover the values.
All test names are updated to use the `FileSys` prefix for consistency.

### Step 3 — Testing & Validation

```bash
# Build
cmake --build build/

# Run constants tests — all 4 cases must still pass
./build/run_tests "[constants]"

# Confirm no kFs* identifiers survive anywhere in the codebase
grep -rn 'kFs[A-Z]' src/ tests/
# Expected: no matches
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Phase 2 begins.
>
> Checklist:
> - [ ] All 6 constants renamed from `kFs*` → `kFileSys*` in `constants.hpp`
> - [ ] All call sites updated in `src/main.cpp`
> - [ ] All test names and references updated in `test_constants.cpp`
> - [ ] No `kFs[A-Z]` identifiers survive anywhere in `src/` or `tests/`
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 1.1 looks good, proceed to 2"_

---

## Phase 2 · `kFrankFurterApi*` — Exchange Rate API

> **6 constants** · Affects: `controller/exchange_rate_controller.cpp`
> ⚠️ Prefix renamed from `kApi*` → `kFrankFurterApi*` in Phase 2.1 — see below.

### Step 1 — Implementation

**Add to `src/util/constants.hpp`:**
```cpp
// ── Exchange Rate API ───────────────────────────────────────────────────────
constexpr std::string_view kApiBaseUrl              = "https://api.frankfurter.dev/v2/rate/";
constexpr int              kApiCacheTtlSeconds      = 86400;
constexpr int              kApiTimeoutMs            = 5000;
constexpr int              kApiHttpStatusOk         = 200;
constexpr std::string_view kApiRateJsonKey          = "rate";
constexpr double           kApiInvalidRateSentinel  = 0.0;
```

**Update `src/controller/exchange_rate_controller.cpp`:**
- Add `#include "util/constants.hpp"`
- Replace `86400` → `calc_cli::kApiCacheTtlSeconds`
- Replace `"https://api.frankfurter.dev/v2/rate/"` → `calc_cli::kApiBaseUrl`
- Replace `5000` (cpr::Timeout) → `calc_cli::kApiTimeoutMs`
- Replace `200` (HTTP check) → `calc_cli::kApiHttpStatusOk`
- Replace `"rate"` (JSON key) → `calc_cli::kApiRateJsonKey`
- Replace `0.0` (stale-rate guard) → `calc_cli::kApiInvalidRateSentinel`

### Step 2 — New Test Cases

**File:** `tests/util/test_constants.cpp` *(append)*

| Test | What it asserts |
|---|---|
| `ApiConstants_BaseUrl` | `kApiBaseUrl` starts with `"https://"` and ends with `"/"` |
| `ApiConstants_CacheTtl` | `kApiCacheTtlSeconds == 86400` and equals `24 * 60 * 60` |
| `ApiConstants_Timeout` | `kApiTimeoutMs > 0` |
| `ApiConstants_HttpStatusOk` | `kApiHttpStatusOk == 200` |
| `ApiConstants_RateJsonKey` | `kApiRateJsonKey == "rate"` |
| `ApiConstants_InvalidSentinel` | `kApiInvalidRateSentinel == 0.0` |

**File:** `tests/controller/test_exchange_rate_controller.cpp` *(extend existing)*

| Test | What it asserts |
|---|---|
| `CacheTtl_UsesConstant` | Mock a cached rate with age = `kApiCacheTtlSeconds - 1` → returns cached value; age = `kApiCacheTtlSeconds + 1` → triggers re-fetch |

### Step 3 — Testing & Validation

```bash
# Build
cmake --build build/

# Run constants + controller tests
./build/tests/test_constants
./build/tests/test_exchange_rate_controller

# Confirm no raw literals remain
grep -n '86400\|5000\|"https://api.frankfurter\|"rate"\|0\.0' src/controller/exchange_rate_controller.cpp
# Expected: no matches
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Phase 3 begins.
>
> Checklist:
> - [ ] 6 `kApi*` constants added to `constants.hpp` with correct values
> - [ ] `exchange_rate_controller.cpp` — all 6 literals replaced, no magic numbers remain
> - [ ] 6 new `ApiConstants_*` tests added to `test_constants.cpp`
> - [ ] `CacheTtl_UsesConstant` test added to `test_exchange_rate_controller.cpp`
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 2 looks good, proceed to 2.1"_

---

## Phase 2.1 · Rename `kApi*` → `kFrankFurterApi*`

> **Naming clarity fix** · `kApi` is too generic — `kFrankFurterApi` makes clear these constants
> are specific to the Frankfurter exchange rate API, not just any API.
> Affects: `src/util/constants.hpp`, `src/controller/exchange_rate_controller.cpp`,
> `tests/util/test_constants.cpp`, `tests/controller/test_exchange_rate_controller.cpp`

### Step 1 — Implementation

Rename all 6 constants in `src/util/constants.hpp`:

| Old name | New name |
|---|---|
| `kApiBaseUrl` | `kFrankFurterApiBaseUrl` |
| `kApiCacheTtlSeconds` | `kFrankFurterApiCacheTtlSeconds` |
| `kApiTimeoutMs` | `kFrankFurterApiTimeoutMs` |
| `kApiHttpStatusOk` | `kFrankFurterApiHttpStatusOk` |
| `kApiRateJsonKey` | `kFrankFurterApiRateJsonKey` |
| `kApiInvalidRateSentinel` | `kFrankFurterApiInvalidRateSentinel` |

Update all call sites in:
- `src/controller/exchange_rate_controller.cpp`
- `tests/util/test_constants.cpp`
- `tests/controller/test_exchange_rate_controller.cpp`

### Step 2 — No new test cases

This is a pure rename — existing `ApiConstants_*` tests already cover the values.
All test names are updated to use the `FrankFurterApi` prefix for consistency.

### Step 3 — Testing & Validation

```bash
# Build
cmake --build build/

# Run constants + controller tests — all must still pass
./build/run_tests "[constants],[exchange_rate_controller]"

# Confirm no kApi* identifiers survive anywhere in the codebase
grep -rn 'kApi[A-Z]' src/ tests/
# Expected: no matches
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Phase 3 begins.
>
> Checklist:
> - [ ] All 6 constants renamed from `kApi*` → `kFrankFurterApi*` in `constants.hpp`
> - [ ] All call sites updated in `exchange_rate_controller.cpp`
> - [ ] All test names and constant references updated in `test_constants.cpp` and `test_exchange_rate_controller.cpp`
> - [ ] No `kApi[A-Z]` identifiers survive anywhere in `src/` or `tests/`
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 2.1 looks good, proceed to 3"_

---

## Phase 3 · `kDb*` — SQLite Table & Column Names

> **9 constants** · Affects: `model/exchange_rate.cpp`, `model/history_repository.cpp`

### Step 1 — Implementation

**Add to `src/util/constants.hpp`:**
```cpp
// ── Database: Exchange Rate Table ───────────────────────────────────────────
constexpr std::string_view kDbExchangeTable          = "exchange_rates";
constexpr std::string_view kDbExchangeColBase        = "base_currency";
constexpr std::string_view kDbExchangeColQuote       = "quote_currency";
constexpr std::string_view kDbExchangeColRate        = "rate";
constexpr std::string_view kDbExchangeColLastUpdated = "last_updated";

// ── Database: History Table ─────────────────────────────────────────────────
constexpr std::string_view kDbHistoryTable           = "history";
constexpr std::string_view kDbHistoryColExpression   = "expression";
constexpr std::string_view kDbHistoryColResult       = "result";
constexpr std::string_view kDbHistoryColTimestamp    = "timestamp";
```

**Update `src/model/exchange_rate.cpp`:**
- Add `#include "util/constants.hpp"`
- Replace 5 string literals across all SQL statements (CREATE TABLE, INSERT, SELECT)
  with `calc_cli::kDbExchange*` equivalents

**Update `src/model/history_repository.cpp`:**
- Add `#include "util/constants.hpp"`
- Replace 4 string literals across all SQL statements
  with `calc_cli::kDbHistory*` equivalents

### Step 2 — New Test Cases

**File:** `tests/util/test_constants.cpp` *(append)*

| Test | What it asserts |
|---|---|
| `DbConstants_ExchangeTable` | `kDbExchangeTable == "exchange_rates"` |
| `DbConstants_ExchangeColumns` | All 4 exchange column name constants are non-empty and distinct |
| `DbConstants_HistoryTable` | `kDbHistoryTable == "history"` |
| `DbConstants_HistoryColumns` | All 3 history column name constants are non-empty and distinct |
| `DbConstants_NoColumnNameCollisions` | Exchange and history column names are all unique across both tables |

**File:** `tests/model/test_exchange_rate.cpp` *(extend existing)*

| Test | What it asserts |
|---|---|
| `ExchangeRate_TableSchemaMatchesConstants` | After `CreateTable()`, query `sqlite_master` and confirm table name matches `kDbExchangeTable` |

**File:** `tests/model/test_history_repository.cpp` *(extend existing)*

| Test | What it asserts |
|---|---|
| `HistoryRepo_TableSchemaMatchesConstants` | After `CreateTable()`, query `sqlite_master` and confirm table name matches `kDbHistoryTable` |

### Step 3 — Testing & Validation

```bash
# Build
cmake --build build/

# Run constants + model tests
./build/tests/test_constants
./build/tests/test_exchange_rate
./build/tests/test_history_repository

# Confirm no raw literals remain
grep -n '"exchange_rates"\|"base_currency"\|"quote_currency"\|"last_updated"' src/model/exchange_rate.cpp
grep -n '"history"\|"expression"\|"result"\|"timestamp"' src/model/history_repository.cpp
# Expected: no matches
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Phase 3.1 begins.
>
> Checklist:
> - [ ] 9 `kDb*` constants added to `constants.hpp` (5 exchange + 4 history)
> - [ ] `exchange_rate.cpp` — all SQL string literals replaced
> - [ ] `history_repository.cpp` — all SQL string literals replaced
> - [ ] 5 new `DbConstants_*` tests in `test_constants.cpp`
> - [ ] Schema-matching tests added to `test_exchange_rate.cpp` and `test_history_repository.cpp`
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 3 looks good, proceed to 3.1"_

---

## Phase 3.1 · `std::format` & C++20 Upgrade

> **C++20 Modernization** · Refactor SQL string concatenation using `std::format`
> Affects: `CMakeLists.txt`, `src/model/exchange_rate.cpp`, `src/model/history_repository.cpp`

### Step 1 — Implementation

**Update `CMakeLists.txt`:**
- Change C++ standard from C++17 to C++20:
  ```cmake
  set(CMAKE_CXX_STANDARD 20)
  ```

**Update `src/model/exchange_rate.cpp`:**
- Add `#include <format>`
- Refactor the SQL query strings to use `std::format` instead of `+` concatenation.
  For example, `insert_sql` becomes:
  ```cpp
  std::string insert_sql = std::format(
      "INSERT OR REPLACE INTO {} ({}, {}, {}, {}) VALUES (?, ?, ?, ?);",
      calc_cli::kDbExchangeTable,
      calc_cli::kDbExchangeColBase,
      calc_cli::kDbExchangeColQuote,
      calc_cli::kDbExchangeColRate,
      calc_cli::kDbExchangeColLastUpdated
  );
  ```
  And similarly update the `CREATE TABLE` and `SELECT` queries.

**Update `src/model/history_repository.cpp`:**
- Add `#include <format>`
- Refactor the SQL query strings (`CREATE TABLE`, `SELECT`, `INSERT`, `DELETE`) to use `std::format`.

### Step 2 — No new test cases

Existing unit tests will verify that formatting does not break any database functionality.

### Step 3 — Testing & Validation

```bash
# Build (compiles as C++20)
cmake --build build/

# Run tests
./build/run_tests
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Phase 4 begins.
>
> Checklist:
> - [ ] `CMakeLists.txt` standard upgraded to 20
> - [ ] `src/model/exchange_rate.cpp` uses `std::format`
> - [ ] `src/model/history_repository.cpp` uses `std::format`
> - [ ] Build succeeds with C++20
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 3.1 looks good, proceed to 4"_

---

## Phase 4 · `kParser*` — Exchange Keyword

> **2 constants** · Affects: `model/parser.cpp`, `util/formatting.cpp`

### Step 1 — Implementation

**Add to `src/util/constants.hpp`:**
```cpp
// ── Parser ──────────────────────────────────────────────────────────────────
constexpr std::string_view kParserExchangeKeyword    = "exchange";
constexpr std::size_t      kParserExchangeKeywordLen = kParserExchangeKeyword.size();
```

**Update `src/model/parser.cpp`:**
- Add `#include "util/constants.hpp"`
- Replace all 6+ occurrences of `"exchange"` and magic `8` with
  `calc_cli::kParserExchangeKeyword` / `calc_cli::kParserExchangeKeywordLen`

**Update `src/util/formatting.cpp`:**
- Add `#include "util/constants.hpp"`
- Replace `"exchange"` and `8` in the formatting logic with the same constants

### Step 2 — New Test Cases

**File:** `tests/util/test_constants.cpp` *(append)*

| Test | What it asserts |
|---|---|
| `ParserConstants_KeywordValue` | `kParserExchangeKeyword == "exchange"` |
| `ParserConstants_KeywordLenDerived` | `kParserExchangeKeywordLen == kParserExchangeKeyword.size()` |
| `ParserConstants_KeywordLenValue` | `kParserExchangeKeywordLen == 8` |

**File:** `tests/model/test_parser.cpp` *(extend existing)*

| Test | What it asserts |
|---|---|
| `Parser_ExchangeKeywordRecognised` | `parse("exchange(AUD, USD)")` succeeds — confirms keyword constant is correctly wired |
| `Parser_PartialKeywordRejected` | `parse("exchang(AUD, USD)")` fails — confirms the full keyword is required |

**File:** `tests/util/test_formatting.cpp` *(extend existing)*

| Test | What it asserts |
|---|---|
| `Formatting_ExchangeKeywordLen` | Any formatting path using keyword length produces the same result before and after constant substitution |

### Step 3 — Testing & Validation

```bash
# Build
cmake --build build/

# Run full test suite
./build/tests/test_constants
./build/tests/test_parser
./build/tests/test_formatting

# Confirm no raw literals remain
grep -n '"exchange"\| 8\b' src/model/parser.cpp src/util/formatting.cpp
# Expected: no matches for hardcoded keyword/length
```

### Step 4 — Human Review

> [!IMPORTANT]
> **Stop here.** The engineer must review all changes before Final Integration begins.
>
> Checklist:
> - [ ] 2 `kParser*` constants added to `constants.hpp`, `kParserExchangeKeywordLen` derived from `kParserExchangeKeyword.size()`
> - [ ] `parser.cpp` — all `"exchange"` literals and magic `8` replaced
> - [ ] `formatting.cpp` — all `"exchange"` literals and magic `8` replaced
> - [ ] 3 new `ParserConstants_*` tests in `test_constants.cpp`
> - [ ] 2 new tests added to `test_parser.cpp`, 1 to `test_formatting.cpp`
> - [ ] All tests pass locally
>
> **Confirm with:** _"Phase 4 looks good, proceed to Final Integration"_

---

## Final Integration

After all 4 phases are complete:

```bash
# Full clean build
cmake --build build/ --clean-first

# Run entire test suite
ctest --test-dir build/ --output-on-failure

# Final grep across all src/ — confirm no surviving raw literals
grep -rn '"HOME"\|"USERPROFILE"\|"/.calc_cli"\|86400\|"https://api.frankfurter\|"exchange_rates"\|"exchange"' src/
# Expected: only comments and the constants.hpp definitions themselves
```

---

## Summary Table

| Phase | Constants | Affected files | New tests |
|---|---|---|---|
| Pre-flight | — | `src/util/constants.hpp` (create) | — |
| 1 · `kFileSys*` | 6 | `main.cpp` | 4 new in `test_constants.cpp` |
| 1.1 · Rename `kFs*` → `kFileSys*` | — | `constants.hpp`, `main.cpp`, `test_constants.cpp` | No new tests (pure rename) |
| 2 · `kFrankFurterApi*` | 6 | `exchange_rate_controller.cpp` | 6 new in `test_constants.cpp`, 1 in `test_exchange_rate_controller.cpp` |
| 2.1 · Rename `kApi*` → `kFrankFurterApi*` | — | `constants.hpp`, `exchange_rate_controller.cpp`, `test_constants.cpp`, `test_exchange_rate_controller.cpp` | No new tests (pure rename) |
| 3 · `kDb*` | 9 | `exchange_rate.cpp`, `history_repository.cpp` | 5 new in `test_constants.cpp`, 1 each in `test_exchange_rate.cpp` and `test_history_repository.cpp` |
| 3.1 · `std::format` & C++20 Upgrade | — | `CMakeLists.txt`, `exchange_rate.cpp`, `history_repository.cpp` | No new tests |
| 4 · `kParser*` | 2 | `parser.cpp`, `formatting.cpp` | 3 new in `test_constants.cpp`, 2 in `test_parser.cpp`, 1 in `test_formatting.cpp` |
| **Total** | **23** | **8 files updated, 1 created** | **~23 new test cases** |
