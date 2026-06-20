#include <catch2/catch_test_macros.hpp>
#include "model/app_state.hpp"
#include "view/custom_exchange.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

using namespace ftxui;

TEST_CASE("CustomExchange View — interaction", "[custom_exchange]") {
    AppState state;
    bool submit_called = false;
    view::CustomExchange custom_exchange(state, [&submit_called] { submit_called = true; });
    auto comp = custom_exchange.GetComponent();

    SECTION("Escape cancels and closes the modal") {
        state.show_custom_modal = true;
        state.custom_source_input = "AUD";
        state.custom_target_input = "USD";
        state.expression_input = "100 ";
        state.cursor_position = 4;

        // Dispatch Escape event directly to custom_exchange component
        comp->OnEvent(Event::Escape);

        REQUIRE_FALSE(state.show_custom_modal);
        REQUIRE(state.custom_source_input.empty());
        REQUIRE(state.custom_target_input.empty());
        REQUIRE(state.expression_input == "100 ");
        REQUIRE_FALSE(submit_called);
    }

    SECTION("Return with non-empty inputs saves and closes modal") {
        state.show_custom_modal = true;
        state.custom_source_input = "EUR";
        state.custom_target_input = "GBP";
        state.expression_input = "100";
        state.cursor_position = 0; // cursor at the beginning

        // Dispatch Return event directly to custom_exchange component
        comp->OnEvent(Event::Return);

        REQUIRE_FALSE(state.show_custom_modal);
        REQUIRE(state.custom_source_input.empty());
        REQUIRE(state.custom_target_input.empty());
        REQUIRE(state.expression_input == "100 exchange(EUR, GBP)");
        REQUIRE(state.cursor_position == 22);
        REQUIRE(submit_called);
    }

    SECTION("Return with empty inputs does not insert or close modal") {
        state.show_custom_modal = true;
        state.custom_source_input = "";
        state.custom_target_input = "USD";
        state.expression_input = "100 ";
        state.cursor_position = 4;

        comp->OnEvent(Event::Return);

        REQUIRE(state.show_custom_modal);
        REQUIRE(state.expression_input == "100 ");
    }
}
