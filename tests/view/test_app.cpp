#include <catch2/catch_test_macros.hpp>
#include "model/app_state.hpp"
#include "service/calculator.hpp"
#include "model/history_repository.hpp"
#include "model/exchange_rate.hpp"
#include "controller/app_controller.hpp"
#include "controller/history_controller.hpp"
#include "controller/exchange_rate_controller.hpp"
#include "view/app.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <memory>

using namespace ftxui;

static void FocusExprInput(ftxui::Component& comp) {
    comp->OnEvent(Event::ArrowDown);  // top_menu -> tab_container
    comp->OnEvent(Event::ArrowDown);  // tab_container -> expr_input
}

static auto MakeApp(bool* quit_called = nullptr) {
    struct Fixture {
        AppState state;
        Calculator calc;
        std::unique_ptr<HistoryRepository> history_repo;
        std::unique_ptr<HistoryController> history_ctrl;
        std::unique_ptr<ExchangeRate> exch_repo;
        std::unique_ptr<ExchangeRateController> exch_ctrl;
        std::unique_ptr<AppController> controller;
        std::unique_ptr<App> app;
        ftxui::Component comp;
    };
    auto f = std::make_shared<Fixture>();
    f->history_repo = std::make_unique<HistoryRepository>(":memory:");
    f->history_repo->Initialize();
    f->history_ctrl = std::make_unique<HistoryController>(*f->history_repo);
    
    f->exch_repo = std::make_unique<ExchangeRate>(":memory:");
    f->exch_repo->Initialize();
    f->exch_ctrl = std::make_unique<ExchangeRateController>(*f->exch_repo);

    bool* qc = quit_called;
    f->controller = std::make_unique<AppController>(
        f->state, f->calc, *f->history_ctrl, *f->exch_ctrl, [qc] {
            if (qc) *qc = true;
        });
    f->app = std::make_unique<App>(f->state, *f->controller);
    f->comp = f->app->GetComponent();
    return f;
}

TEST_CASE("App — evaluate expression via keypress", "[app]") {
    auto f = MakeApp();
    FocusExprInput(f->comp);
    f->state.expression_input = "3 + 4";
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.result_display == "= 7");
    REQUIRE_FALSE(f->state.error_state);
}

TEST_CASE("App — error display on bad expression", "[app]") {
    auto f = MakeApp();
    FocusExprInput(f->comp);
    f->state.expression_input = "1 / 0";
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.error_state);
    REQUIRE(f->state.result_display.rfind("Error:", 0) == 0);
}

TEST_CASE("App — Escape clears input and result", "[app]") {
    auto f = MakeApp();
    FocusExprInput(f->comp);
    f->state.expression_input = "42";
    f->state.result_display = "= 42";
    f->comp->OnEvent(Event::Escape);
    REQUIRE(f->state.expression_input.empty());
    REQUIRE(f->state.result_display.empty());
    REQUIRE_FALSE(f->state.error_state);
}

TEST_CASE("App — Edit -> Clear via menu", "[app]") {
    auto f = MakeApp();
    f->state.expression_input = "some expression";
    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowDown);    // focus into Edit sub-menu
    f->comp->OnEvent(Event::Return);       // trigger Clear
    REQUIRE(f->state.expression_input.empty());
    REQUIRE(f->state.result_display.empty());
}

TEST_CASE("App — Edit -> Clear History via menu", "[app]") {
    auto f = MakeApp();
    f->history_ctrl->OnSaveHistory("1 + 1", "2");
    REQUIRE_FALSE(f->controller->GetHistory().empty());

    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowDown);    // focus into Edit sub-menu
    f->comp->OnEvent(Event::ArrowRight);   // Clear Input -> Clear History
    f->comp->OnEvent(Event::Return);       // trigger Clear History

    REQUIRE(f->controller->GetHistory().empty());
}

TEST_CASE("App — Help -> Version opens modal", "[app]") {
    auto f = MakeApp();
    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
    f->comp->OnEvent(Event::ArrowRight);   // Exchange -> Help
    f->comp->OnEvent(Event::ArrowDown);    // focus into Help sub-menu
    f->comp->OnEvent(Event::Return);       // trigger Version
    REQUIRE(f->state.show_version_modal);
}

TEST_CASE("App — File -> Quit calls on_quit", "[app]") {
    bool quit_called = false;
    auto f = MakeApp(&quit_called);
    f->comp->OnEvent(Event::ArrowDown);    // focus into File sub-menu
    f->comp->OnEvent(Event::Return);       // trigger Quit
    REQUIRE(quit_called);
}

TEST_CASE("App — Exchange -> AUD -> USD inserts shorthand", "[app]") {
    auto f = MakeApp();
    f->state.expression_input = "100";
    f->state.cursor_position = 0; // Cursor is at the beginning

    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
    f->comp->OnEvent(Event::ArrowDown);    // focus into Exchange sub-menu (AUD -> USD)
    f->comp->OnEvent(Event::Return);       // trigger insert

    REQUIRE(f->state.expression_input == "100 exchange(AUD, USD)");
    REQUIRE(f->state.cursor_position == 22); // Cursor should be at the end of the new string

    // Verify focus returned to expr_input by typing a character '1'
    f->comp->OnEvent(Event::Character('1'));
    REQUIRE(f->state.expression_input == "100 exchange(AUD, USD)1");
}

TEST_CASE("App — Exchange -> AUD -> USD appends to end after typing", "[app]") {
    auto f = MakeApp();
    f->state.expression_input = "600";
    f->state.cursor_position = 3;

    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
    f->comp->OnEvent(Event::ArrowDown);    // focus into Exchange sub-menu (AUD -> USD)
    f->comp->OnEvent(Event::Return);       // trigger insert

    REQUIRE(f->state.expression_input == "600 exchange(AUD, USD)");
    REQUIRE(f->state.cursor_position == 22);

    // Verify focus returned to expr_input by typing a character '+'
    f->comp->OnEvent(Event::Character('+'));
    REQUIRE(f->state.expression_input == "600 exchange(AUD, USD)+");
}

TEST_CASE("App — History Menu navigation and recall", "[app][history_menu]") {
    auto f = MakeApp();
    f->history_ctrl->OnSaveHistory("5 + 5", "10");
    f->history_ctrl->OnSaveHistory("3 * 3", "9");
    
    // Re-create controller/app to synchronize startup database entries
    f->controller = std::make_unique<AppController>(
        f->state, f->calc, *f->history_ctrl, *f->exch_ctrl, [] {});
    f->app = std::make_unique<App>(f->state, *f->controller);
    f->comp = f->app->GetComponent();

    REQUIRE(f->state.history_menu_entries.size() == 2);
    REQUIRE(f->state.selected_history_idx == 0); // Top item initially

    // Navigate focus down: top_menu -> tab_container -> expr_input -> history_menu
    f->comp->OnEvent(Event::ArrowDown); // tab_container
    f->comp->OnEvent(Event::ArrowDown); // expr_input
    f->comp->OnEvent(Event::ArrowDown); // history_menu

    // Navigate down in the history list (from 0 -> 1)
    f->comp->OnEvent(Event::ArrowDown);
    REQUIRE(f->state.selected_history_idx == 1);

    // Confirm choice via Return
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.expression_input == "3 * 3");
}

TEST_CASE("App — History Menu bounds validation", "[app][history_menu]") {
    auto f = MakeApp();
    for (int i = 0; i < 20; ++i) {
        f->history_ctrl->OnSaveHistory("1 + " + std::to_string(i), std::to_string(1 + i));
    }
    f->controller = std::make_unique<AppController>(
        f->state, f->calc, *f->history_ctrl, *f->exch_ctrl, [] {});
    f->app = std::make_unique<App>(f->state, *f->controller);
    f->comp = f->app->GetComponent();

    REQUIRE(f->state.history_menu_entries.size() == 20);
    REQUIRE(f->state.selected_history_idx == 0); // Top item initially

    // Navigate focus down: top_menu -> tab_container -> expr_input -> history_menu
    f->comp->OnEvent(Event::ArrowDown); // tab_container
    f->comp->OnEvent(Event::ArrowDown); // expr_input
    f->comp->OnEvent(Event::ArrowDown); // history_menu

    // ArrowUp at bounds should be blocked
    f->comp->OnEvent(Event::ArrowUp);
    REQUIRE(f->state.selected_history_idx == 0);

    // Refocus history_menu (since ArrowUp at index 0 shifted focus back to expr_input)
    f->comp->OnEvent(Event::ArrowDown);

    // Navigate Down 10 times
    for (int i = 0; i < 10; ++i) {
        f->comp->OnEvent(Event::ArrowDown);
    }
    REQUIRE(f->state.selected_history_idx == 10);

    // Confirm choice at index 10 (which is "1 + 10 = 11")
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.expression_input == "1 + 10");
}

TEST_CASE("App — History Menu recall and re-evaluation with exchange rate", "[app][history_menu_bug]") {
    auto f = MakeApp();
    f->exch_repo->SaveRate("AUD", "USD", 0.65);
    f->history_ctrl->OnSaveHistory("1 + 1", "2");
    f->history_ctrl->OnSaveHistory("100 exchange(AUD, USD)", "65");

    f->controller = std::make_unique<AppController>(
        f->state, f->calc, *f->history_ctrl, *f->exch_ctrl, [] {});
    f->app = std::make_unique<App>(f->state, *f->controller);
    f->comp = f->app->GetComponent();

    REQUIRE(f->state.history_menu_entries.size() == 2);
    REQUIRE(f->state.selected_history_idx == 0);
    REQUIRE(f->state.focused_history_idx == 0);

    // Focus history menu (top_menu -> tab_container -> expr_input -> history_menu)
    f->comp->OnEvent(Event::ArrowDown); // tab_container
    f->comp->OnEvent(Event::ArrowDown); // expr_input
    f->comp->OnEvent(Event::ArrowDown); // history_menu

    // Go down to index 1 ("100 exchange(AUD, USD) = 65")
    f->comp->OnEvent(Event::ArrowDown);
    REQUIRE(f->state.selected_history_idx == 1);
    REQUIRE(f->state.focused_history_idx == 1);

    // Let's recall index 1 ("100 exchange(AUD, USD) = 65").
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.expression_input == "100 exchange(AUD, USD)");
    // Should reset immediately on recall
    REQUIRE(f->state.selected_history_idx == 0);
    REQUIRE(f->state.focused_history_idx == 0);

    // Press Enter to evaluate (since we are focused on expr_input)
    f->comp->OnEvent(Event::Return);

    // Verify it evaluates and appends to history
    REQUIRE(f->state.history_menu_entries.size() == 3);
    REQUIRE(f->state.selected_history_idx == 0);
    REQUIRE(f->state.focused_history_idx == 0);

    // Press Down to focus history menu.
    f->comp->OnEvent(Event::ArrowDown);
    REQUIRE(f->state.selected_history_idx == 0);
    REQUIRE(f->state.focused_history_idx == 0);
}

TEST_CASE("App — History Menu recall and evaluation failure resets index", "[app][history_menu_bug]") {
    auto f = MakeApp();
    f->history_ctrl->OnSaveHistory("1 + 1", "2");
    f->history_ctrl->OnSaveHistory("5 + 5", "10");

    f->controller = std::make_unique<AppController>(
        f->state, f->calc, *f->history_ctrl, *f->exch_ctrl, [] {});
    f->app = std::make_unique<App>(f->state, *f->controller);
    f->comp = f->app->GetComponent();

    // Focus history menu
    f->comp->OnEvent(Event::ArrowDown); // tab_container
    f->comp->OnEvent(Event::ArrowDown); // expr_input
    f->comp->OnEvent(Event::ArrowDown); // history_menu

    // Go down to index 1 ("5 + 5 = 10")
    f->comp->OnEvent(Event::ArrowDown);
    REQUIRE(f->state.selected_history_idx == 1);
    REQUIRE(f->state.focused_history_idx == 1);

    // Recall item
    f->comp->OnEvent(Event::Return);
    REQUIRE(f->state.expression_input == "5 + 5");
    // Verify it is reset to 0 immediately
    REQUIRE(f->state.selected_history_idx == 0);
    REQUIRE(f->state.focused_history_idx == 0);

    // Make expression invalid so evaluation fails
    f->state.expression_input = "5 + 5 +";
    f->comp->OnEvent(Event::Return); // trigger evaluation (fails)

    REQUIRE(f->state.error_state);
    
    // Press Down to focus history menu again
    f->comp->OnEvent(Event::ArrowDown);
    REQUIRE(f->state.selected_history_idx == 0);
    REQUIRE(f->state.focused_history_idx == 0);
}



