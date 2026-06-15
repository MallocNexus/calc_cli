#include <catch2/catch_test_macros.hpp>
#include "model/app_state.hpp"
#include "model/calculator.hpp"
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
    f->state.expression_input = "100 ";
    f->state.cursor_position = 4;

    f->comp->OnEvent(Event::ArrowRight);   // File -> Edit
    f->comp->OnEvent(Event::ArrowRight);   // Edit -> Exchange
    f->comp->OnEvent(Event::ArrowDown);    // focus into Exchange sub-menu (AUD -> USD)
    f->comp->OnEvent(Event::Return);       // trigger insert

    REQUIRE(f->state.expression_input == "100 exchange(AUD, USD)");
}
