#include "view/custom_exchange.hpp"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace view {

using namespace ftxui;

CustomExchange::CustomExchange(AppState& state) : state_(state) {
    InputOption src_opt = InputOption::Default();
    src_opt.content = &state_.custom_source_input;
    src_opt.placeholder = "";
    auto src_input = Input(src_opt);

    InputOption dst_opt = InputOption::Default();
    dst_opt.content = &state_.custom_target_input;
    dst_opt.placeholder = "";
    auto dst_input = Input(dst_opt);

    auto custom_ok = Button("  OK  ", [this] {
        if (!state_.custom_source_input.empty() && !state_.custom_target_input.empty()) {
            std::string inserted = "exchange(" + state_.custom_source_input + ", " + state_.custom_target_input + ")";
            state_.expression_input.insert(state_.cursor_position, inserted);
            state_.cursor_position += inserted.size();
            state_.show_custom_modal = false;
            state_.custom_source_input.clear();
            state_.custom_target_input.clear();
        }
    });

    auto custom_cancel = Button("Cancel", [this] {
        state_.show_custom_modal = false;
        state_.custom_source_input.clear();
        state_.custom_target_input.clear();
    });

    auto modal_container = Container::Vertical({
        src_input,
        dst_input,
        Container::Horizontal({
            custom_ok,
            custom_cancel
        })
    });

    auto modal_event_handler = CatchEvent(modal_container, [this](Event event) -> bool {
        if (event == Event::Escape) {
            state_.show_custom_modal = false;
            state_.custom_source_input.clear();
            state_.custom_target_input.clear();
            return true;
        }
        if (event == Event::Return) {
            if (!state_.custom_source_input.empty() && !state_.custom_target_input.empty()) {
                std::string inserted = "exchange(" + state_.custom_source_input + ", " + state_.custom_target_input + ")";
                state_.expression_input.insert(state_.cursor_position, inserted);
                state_.cursor_position += inserted.size();
                state_.show_custom_modal = false;
                state_.custom_source_input.clear();
                state_.custom_target_input.clear();
                return true;
            }
            return true;
        }
        return false;
    });

    component_ = Renderer(modal_event_handler, [this, src_input, dst_input, custom_ok, custom_cancel] {
        return vbox({
                   text("Custom Exchange") | bold | center,
                   separator(),
                   hbox({
                       text("Source Currency: ") | size(WIDTH, EQUAL, 17),
                       src_input->Render() | border | size(WIDTH, EQUAL, 15)
                   }) | center,
                   hbox({
                       text("Target Currency: ") | size(WIDTH, EQUAL, 17),
                       dst_input->Render() | border | size(WIDTH, EQUAL, 15)
                   }) | center,
                   separator(),
                   hbox({
                       custom_ok->Render() | center,
                       text("    "),
                       custom_cancel->Render() | center
                   }) | center
               }) |
               size(WIDTH, GREATER_THAN, 40) | border | clear_under | center;
    });
}

ftxui::Component CustomExchange::GetComponent() { return component_; }

} // namespace view
