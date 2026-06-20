# Future Refactoring Ideas

This document captures intentional deferrals — improvements identified during planning but
excluded from their initial pass for scope or complexity reasons.
Each section notes the rationale and what a future implementation would look like.

---

## 1. Centralise UI / View Strings in `constants.hpp`

**Deferred from:** `src/util/constants.hpp` initial pass (high-priority constants only)

**Rationale for deferral:**
Menu labels and modal strings are currently stored in `std::vector<std::string>` inside
`view/app.hpp`. Wiring `constexpr std::string_view` constants into these vectors requires
explicit `.data()` or `std::string(...)` conversions at each call site — a mechanical but
non-trivial change that touches the view layer heavily. Deferred to avoid scope creep.

**What the refactor involves:**
- Add a `// ── UI / View ─────────` section to `constants.hpp` under `namespace calc_cli`
- Replace every hardcoded string in `view/app.cpp`, `view/app.hpp`, `view/custom_exchange.cpp`
  with the named constant
- Where `std::vector<std::string>` initialisation is needed, wrap with `std::string(kFoo)`

**Constants to add (31 total):**

### App version info
| Constant | Value |
|---|---|
| `kUiAppVersionString` | `"calc-cli  v1.0.0"` |
| `kUiAppDescription` | `"Terminal calculator built with FTXUI"` |

### Input / result display
| Constant | Value | Source |
|---|---|---|
| `kUiInputPlaceholder` | `"Enter expression, e.g.  3 + 4 * (2 - 1)"` | `view/app.cpp:32` |
| `kUiInputPromptPrefix` | `" > "` | `view/app.cpp:140` |
| `kUiResultPrefix` | `"= "` | `app_controller.cpp:31` |
| `kUiResultIndent` | `"   "` | `view/app.cpp:142` |

### History display
| Constant | Value | Source |
|---|---|---|
| `kUiHistorySectionLabel` | `"History:"` | `view/app.cpp:129` |
| `kUiHistoryRowSeparator` | `"  =  "` | `view/app.cpp:131` |
| `kUiHistoryRowIndent` | `"  "` | `view/app.cpp:131` |

### Version modal
| Constant | Value | Source |
|---|---|---|
| `kUiVersionModalCloseLabel` | `"Close"` | `view/app.cpp:153` |
| `kUiVersionModalMinWidth` | `44` | `view/app.cpp:163` |

### Exchange snippet
| Constant | Value | Source |
|---|---|---|
| `kUiDefaultExchangeSnippet` | `"exchange(AUD, USD)"` | `view/app.cpp:86` |

### Custom exchange modal
| Constant | Value | Source |
|---|---|---|
| `kUiCustomExchangeModalTitle` | `"Custom Exchange"` | `custom_exchange.cpp:70` |
| `kUiCustomExchangeSrcLabel` | `"Source Currency: "` | `custom_exchange.cpp:73` |
| `kUiCustomExchangeDstLabel` | `"Target Currency: "` | `custom_exchange.cpp:77` |
| `kUiCustomExchangeLabelWidth` | `17` | `custom_exchange.cpp:73,77` |
| `kUiCustomExchangeInputWidth` | `15` | `custom_exchange.cpp:74,78` |
| `kUiCustomExchangeOkLabel` | `"  OK  "` | `custom_exchange.cpp:20` |
| `kUiCustomExchangeCancelLabel` | `"Cancel"` | `custom_exchange.cpp:31` |
| `kUiCustomExchangeButtonSpacer` | `"    "` | `custom_exchange.cpp:83` |
| `kUiCustomExchangeModalMinWidth` | `40` | `custom_exchange.cpp:87` |

### Menu labels
| Constant | Value | Source |
|---|---|---|
| `kUiMenuFile` | `"File"` | `view/app.hpp:32` |
| `kUiMenuEdit` | `"Edit"` | `view/app.hpp:32` |
| `kUiMenuExchange` | `"Exchange"` | `view/app.hpp:32` |
| `kUiMenuHelp` | `"Help"` | `view/app.hpp:32` |
| `kUiMenuFileQuit` | `"Quit"` | `view/app.hpp:36` |
| `kUiMenuEditClearInput` | `"Clear Input"` | `view/app.hpp:39` |
| `kUiMenuEditClearHistory` | `"Clear History"` | `view/app.hpp:39` |
| `kUiMenuExchangeAudUsd` | `"AUD->USD"` | `view/app.hpp:42` |
| `kUiMenuExchangeCustom` | `"Custom"` | `view/app.hpp:42` |
| `kUiMenuHelpVersion` | `"Version"` | `view/app.hpp:45` |

---

## 2. Centralise Error & Warning Message Strings in `constants.hpp`

**Deferred from:** `src/util/constants.hpp` initial pass

**Rationale for deferral:**
Several error strings are used as *prefixes* concatenated with dynamic content at the call site
(e.g. `kErrNetworkRequestFailed + std::to_string(status_code)`). This pattern works fine with
`std::string_view` but requires careful call-site review across 5 files. Deferred for a focused
error-handling pass.

**What the refactor involves:**
- Add a `// ── Errors & Warnings ─────────` section to `constants.hpp`
- Replace all hardcoded error strings across `parser.cpp`, `calculator.cpp`,
  `exchange_rate_controller.cpp`, `main.cpp`, `history_repository.cpp`, `exchange_rate.cpp`
- For concatenation cases, use `std::string(kErrFoo) + dynamic_part`

**Constants to add (17 total):**

### Prefixes (used across many files)
| Constant | Value | Files |
|---|---|---|
| `kErrErrorPrefix` | `"Error: "` | `app_controller.cpp`, `main.cpp`, `exchange_rate.cpp`, `history_repository.cpp` |
| `kErrWarningPrefix` | `"Warning: "` | `main.cpp:139,147` |

### Parser errors
| Constant | Value | Source |
|---|---|---|
| `kErrExpectedNumber` | `"Expected a number"` | `parser.cpp:42` |
| `kErrExpectedCurrency` | `"Expected currency identifier (e.g. AUD)"` | `parser.cpp:66` |
| `kErrExpectedCloseParen` | `"Expected closing ')'"` | `parser.cpp:79` |
| `kErrExpectedOpenParenAfterExchange` | `"Expected '(' after exchange"` | `parser.cpp:92,181` |
| `kErrExpectedComma` | `"Expected ',' after base currency"` | `parser.cpp:98,187` |
| `kErrExpectedCloseParenAfterQuote` | `"Expected ')' after quote currency"` | `parser.cpp:104,193` |
| `kErrRateResolverNotSet` | `"Rate resolver is not set"` | `parser.cpp:109,198` |
| `kErrDivisionByZero` | `"Division by zero"` | `parser.cpp:141` |

### Calculator errors
| Constant | Value | Source |
|---|---|---|
| `kErrEmptyExpression` | `"Empty expression"` | `calculator.cpp:16` |
| `kErrResultNotFinite` | `"Result is not finite"` | `calculator.cpp:37` |
| `kErrUnexpectedCharPrefix` | `"Unexpected character: '"` | `calculator.cpp:30` |

### API / network errors
| Constant | Value | Source |
|---|---|---|
| `kErrInvalidApiResponse` | `"Invalid API response format"` | `exchange_rate_controller.cpp:49` |
| `kErrNetworkRequestFailed` | `"Network request failed (status "` | `exchange_rate_controller.cpp:42` |
| `kErrParseExchangeRate` | `"Failed to parse exchange rate: "` | `exchange_rate_controller.cpp:53` |

---

## 3. Centralise CLI Argument Flags in `constants.hpp`

**Deferred from:** `src/util/constants.hpp` initial pass

**Rationale for deferral:**
CLI flags are only used in `main.cpp` and are simple string comparisons. Low duplication risk
and low impact relative to paths/API/DB constants. Straightforward to add in a future pass.

**What the refactor involves:**
- Add a `// ── CLI Arguments ─────────` section to `constants.hpp`
- Replace string literals in the argument parsing block of `main.cpp`

**Constants to add (6 total):**

| Constant | Value | Source |
|---|---|---|
| `kCliArgStdinFile` | `"--stdin-file"` | `main.cpp:106` |
| `kCliArgValue` | `"--value"` | `main.cpp:109` |
| `kCliArgExpr` | `"--expr"` | `main.cpp:111` |
| `kCliArgExchange` | `"--exchange"` | `main.cpp:114` |
| `kCliArgHelp` | `"--help"` | `main.cpp:117` |
| `kCliArgFlagPrefix` | `"--"` | `main.cpp:129` |

---

## 4. Centralise Formatting Constants in `constants.hpp`

**Deferred from:** `src/util/constants.hpp` initial pass

**Rationale for deferral:**
Only 2 constants, used only in `util/formatting.cpp`. Extremely low risk and low reward for
this pass. Natural to include when the formatting utility is next touched.

**What the refactor involves:**
- Add 2 constants to `constants.hpp`
- Update `util/formatting.cpp`

**Constants to add (2 total):**

| Constant | Value | Source |
|---|---|---|
| `kFmtDoubleBufSize` | `64` | `formatting.cpp:10` |
| `kFmtDoubleFormatSpec` | `"%g"` | `formatting.cpp:11` |
