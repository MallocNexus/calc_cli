#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "app.hpp"
#include "calculator.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

using namespace ftxui;
using Catch::Matchers::WithinRel;

// ===========================================================================
// Unit Tests — Calculator (calc_lib only, no FTXUI involved)
// ===========================================================================

TEST_CASE("Calculator — basic arithmetic", "[calculator]") {
    Calculator calc;

    SECTION("Addition") {
        auto res = calc.Evaluate("2 + 3");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(5.0, 1e-9));
    }

    SECTION("Subtraction") {
        auto res = calc.Evaluate("10 - 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(6.0, 1e-9));
    }

    SECTION("Multiplication") {
        auto res = calc.Evaluate("3 * 7");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(21.0, 1e-9));
    }

    SECTION("Division") {
        auto res = calc.Evaluate("15 / 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(3.75, 1e-9));
    }
}

TEST_CASE("Calculator — operator precedence", "[calculator]") {
    Calculator calc;

    SECTION("Multiplication before addition") {
        auto res = calc.Evaluate("2 + 3 * 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(14.0, 1e-9));
    }

    SECTION("Parentheses override precedence") {
        auto res = calc.Evaluate("(2 + 3) * 4");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(20.0, 1e-9));
    }

    SECTION("Nested parentheses") {
        auto res = calc.Evaluate("3 + 4 * (2 - 1)");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(7.0, 1e-9));
    }
}

TEST_CASE("Calculator — unary minus and floats", "[calculator]") {
    Calculator calc;

    SECTION("Unary minus") {
        auto res = calc.Evaluate("-3 + 1");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(-2.0, 1e-9));
    }

    SECTION("Floating point input") {
        auto res = calc.Evaluate("1.5 + 2.5");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(4.0, 1e-9));
    }

    SECTION("Negative result") {
        auto res = calc.Evaluate("3 - 10");
        REQUIRE(res.ok);
        REQUIRE_THAT(res.value, WithinRel(-7.0, 1e-9));
    }
}

TEST_CASE("Calculator — error cases", "[calculator]") {
    Calculator calc;

    SECTION("Division by zero") {
        auto res = calc.Evaluate("5 / 0");
        REQUIRE_FALSE(res.ok);
        REQUIRE_FALSE(res.error.empty());
    }

    SECTION("Empty expression") {
        auto res = calc.Evaluate("");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Invalid characters") {
        auto res = calc.Evaluate("abc");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Trailing garbage") {
        auto res = calc.Evaluate("3 + 4 abc");
        REQUIRE_FALSE(res.ok);
    }

    SECTION("Mismatched parenthesis") {
        auto res = calc.Evaluate("(3 + 4");
        REQUIRE_FALSE(res.ok);
    }
}

TEST_CASE("Calculator — history", "[calculator]") {
    Calculator calc;

    SECTION("History is empty initially") {
        REQUIRE(calc.GetHistory().empty());
    }

    SECTION("Successful evaluations are recorded") {
        calc.Evaluate("1 + 1");
        calc.Evaluate("2 * 3");
        REQUIRE(calc.GetHistory().size() == 2);
        REQUIRE(calc.GetHistory()[0].first == "1 + 1");
        REQUIRE(calc.GetHistory()[1].first == "2 * 3");
    }

    SECTION("Failed evaluations are NOT recorded") {
        calc.Evaluate("bad input");
        REQUIRE(calc.GetHistory().empty());
    }

    SECTION("ClearHistory empties the record") {
        calc.Evaluate("1 + 1");
        calc.ClearHistory();
        REQUIRE(calc.GetHistory().empty());
    }
}

// ===========================================================================
// Integration Tests — App UI (headless FTXUI event simulation)
//
// Focus order in Container::Vertical({top_menu, tab_container, expr_input}):
//   Tab once  -> moves focus from top_menu  to tab_container (sub-menu)
//   Tab twice -> moves focus from sub-menu  to expr_input
// ===========================================================================

// Helper: navigate focus from top_menu down to the expression input.
//
// Horizontal menus do NOT consume ArrowDown, so it bubbles up to
// Container::Vertical which moves focus to the next child:
//   ArrowDown 1: top_menu   -> tab_container (sub-menu)
//   ArrowDown 2: tab_container -> expr_input
static void FocusExprInput(ftxui::Component& comp) {
    comp->OnEvent(Event::ArrowDown);  // top_menu -> tab_container
    comp->OnEvent(Event::ArrowDown);  // tab_container -> expr_input
}

TEST_CASE("App — evaluate expression", "[app]") {
    AppState state;
    Calculator calc;
    bool quit_called = false;
    App app(state, calc, [&] { quit_called = true; });
    auto comp = app.GetComponent();

    FocusExprInput(comp);
    state.expression_input = "3 + 4";

    comp->OnEvent(Event::Return);

    REQUIRE(state.result_display == "= 7");
    REQUIRE_FALSE(state.error_state);
}

TEST_CASE("App — error display on bad expression", "[app]") {
    AppState state;
    Calculator calc;
    App app(state, calc, [] {});
    auto comp = app.GetComponent();

    FocusExprInput(comp);
    state.expression_input = "1 / 0";

    comp->OnEvent(Event::Return);

    REQUIRE(state.error_state);
    REQUIRE(state.result_display.rfind("Error:", 0) == 0);
}

TEST_CASE("App — Escape clears input and result", "[app]") {
    AppState state;
    Calculator calc;
    App app(state, calc, [] {});
    auto comp = app.GetComponent();

    FocusExprInput(comp);
    state.expression_input = "42";
    state.result_display = "= 42";

    comp->OnEvent(Event::Escape);

    REQUIRE(state.expression_input.empty());
    REQUIRE(state.result_display.empty());
    REQUIRE_FALSE(state.error_state);
}

TEST_CASE("App — Edit -> Clear clears input", "[app]") {
    AppState state;
    Calculator calc;
    App app(state, calc, [] {});
    auto comp = app.GetComponent();

    state.expression_input = "some expression";

    // Navigate top menu to "Edit" (index 1, one ArrowRight from "File")
    comp->OnEvent(Event::ArrowRight);   // File -> Edit
    // Move into sub-menu
    comp->OnEvent(Event::ArrowDown);    // focus into Edit sub-menu ("Clear" focused)
    // Trigger "Clear"
    comp->OnEvent(Event::Return);

    REQUIRE(state.expression_input.empty());
    REQUIRE(state.result_display.empty());
}

TEST_CASE("App — Edit -> History opens history modal", "[app]") {
    AppState state;
    Calculator calc;
    App app(state, calc, [] {});
    auto comp = app.GetComponent();

    // Navigate to Edit sub-menu
    comp->OnEvent(Event::ArrowRight);   // File -> Edit
    comp->OnEvent(Event::ArrowDown);    // focus into Edit sub-menu
    // Move to "History" (index 1, one ArrowRight from "Clear")
    comp->OnEvent(Event::ArrowRight);
    // Trigger "History"
    comp->OnEvent(Event::Return);

    REQUIRE(state.show_history_modal);
}

TEST_CASE("App — Help -> Version opens version modal", "[app]") {
    AppState state;
    Calculator calc;
    App app(state, calc, [] {});
    auto comp = app.GetComponent();

    // Navigate to Help (index 2, two ArrowRights from File)
    comp->OnEvent(Event::ArrowRight);   // File -> Edit
    comp->OnEvent(Event::ArrowRight);   // Edit -> Help
    comp->OnEvent(Event::ArrowDown);    // focus into Help sub-menu
    comp->OnEvent(Event::Return);       // trigger "Version"

    REQUIRE(state.show_version_modal);
}

TEST_CASE("App — File -> Quit calls on_quit", "[app]") {
    AppState state;
    Calculator calc;
    bool quit_called = false;
    App app(state, calc, [&] { quit_called = true; });
    auto comp = app.GetComponent();

    // "File" is already selected (index 0). Move into sub-menu.
    comp->OnEvent(Event::ArrowDown);    // focus into File sub-menu ("Quit" focused)
    comp->OnEvent(Event::Return);       // trigger "Quit"

    REQUIRE(quit_called);
}
