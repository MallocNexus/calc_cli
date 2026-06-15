#include "app.hpp"
#include "calculator.hpp"

#include <cstdio>
#include <cmath>
#include <string>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// ---------------------------------------------------------------------------
// Helper: format a double without unnecessary trailing zeros.
// ---------------------------------------------------------------------------
namespace {
std::string FormatResult(double value) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return std::string(buf);
}
}  // namespace

// ---------------------------------------------------------------------------
// App constructor — builds the full FTXUI component tree.
// ---------------------------------------------------------------------------
App::App(AppState& state, Calculator& calc, std::function<void()> on_quit)
    : state_(state), calc_(calc), on_quit_(on_quit) {

    // ------------------------------------------------------------------
    // Top-level horizontal animated menu: File | Edit | Help
    // ------------------------------------------------------------------
    auto top_menu = Menu(&top_menu_entries_, &top_menu_selected_,
                         MenuOption::HorizontalAnimated());

    // ------------------------------------------------------------------
    // Expression input (single-line)
    // Return  -> evaluate expression
    // Escape  -> clear input and result
    // ------------------------------------------------------------------
    InputOption input_option = InputOption::Default();
    input_option.content = &state_.expression_input;
    input_option.placeholder = "Enter expression, e.g.  3 + 4 * (2 - 1)";
    input_option.cursor_position = &state_.cursor_position;
    input_option.multiline = false;
    input_option.transform = [](InputState s) {
        if (s.is_placeholder) {
            s.element |= dim;
        }
        s.element |= (s.focused ? color(Color::Green) : color(Color::White));
        return s.element;
    };

    auto expr_input_base = Input(input_option);
    auto expr_input = CatchEvent(expr_input_base, [this](Event event) -> bool {
        if (event == Event::Return) {
            EvaluationResult res = calc_.Evaluate(state_.expression_input);
            if (res.ok) {
                state_.result_display = "= " + FormatResult(res.value);
                state_.error_state = false;
            } else {
                state_.result_display = "Error: " + res.error;
                state_.error_state = true;
            }
            return true;
        }
        if (event == Event::Escape) {
            state_.expression_input.clear();
            state_.cursor_position = 0;
            state_.result_display.clear();
            state_.error_state = false;
            return true;
        }
        return false;
    });

    // ------------------------------------------------------------------
    // File sub-menu: Quit
    // ------------------------------------------------------------------
    MenuOption file_option = MenuOption::HorizontalAnimated();
    file_option.on_enter = [this] {
        if (file_selected_ == FILE_QUIT) {
            on_quit_();
        }
    };
    auto file_menu = Menu(&file_entries_, &file_selected_, file_option);

    // ------------------------------------------------------------------
    // Edit sub-menu: Clear | History
    // ------------------------------------------------------------------
    MenuOption edit_option = MenuOption::HorizontalAnimated();
    edit_option.on_enter = [this] {
        if (edit_selected_ == EDIT_CLEAR) {
            state_.expression_input.clear();
            state_.cursor_position = 0;
            state_.result_display.clear();
            state_.error_state = false;
        } else if (edit_selected_ == EDIT_HISTORY) {
            state_.show_history_modal = true;
        }
    };
    auto edit_menu = Menu(&edit_entries_, &edit_selected_, edit_option);

    // ------------------------------------------------------------------
    // Help sub-menu: Version
    // ------------------------------------------------------------------
    MenuOption help_option = MenuOption::HorizontalAnimated();
    help_option.on_enter = [this] {
        if (help_selected_ == HELP_VERSION) {
            state_.show_version_modal = true;
        }
    };
    auto help_menu = Menu(&help_entries_, &help_selected_, help_option);

    // ------------------------------------------------------------------
    // Compose layout
    // ------------------------------------------------------------------
    auto tab_container =
        Container::Tab({file_menu, edit_menu, help_menu}, &top_menu_selected_);

    auto main_container =
        Container::Vertical({top_menu, tab_container, expr_input});

    auto main_renderer =
        Renderer(main_container, [this, top_menu, tab_container, expr_input] {
            Element result_elem;
            if (state_.result_display.empty()) {
                result_elem = text("") | dim;
            } else if (state_.error_state) {
                result_elem = text(state_.result_display) | color(Color::Red);
            } else {
                result_elem = text(state_.result_display) | color(Color::Green) | bold;
            }

            return vbox({
                       top_menu->Render(),
                       separator(),
                       tab_container->Render(),
                       separator(),
                       hbox({text(" > "), expr_input->Render() | flex}),
                       separator(),
                       hbox({text("   "), result_elem}),
                   }) |
                   border;
        });

    // ------------------------------------------------------------------
    // Version modal (Help -> Version)
    // ------------------------------------------------------------------
    auto version_close =
        Button("Close", [this] { state_.show_version_modal = false; });

    auto version_modal = Renderer(version_close, [this, version_close] {
        return vbox({
                   text("calc-cli  v1.0.0") | bold | center,
                   separator(),
                   text("Terminal calculator built with FTXUI") | dim | center,
                   separator(),
                   version_close->Render() | center,
               }) |
               size(WIDTH, GREATER_THAN, 44) | border | clear_under | center;
    });

    // ------------------------------------------------------------------
    // History modal (Edit -> History)
    // ------------------------------------------------------------------
    auto history_close =
        Button("Close", [this] { state_.show_history_modal = false; });

    auto history_modal =
        Renderer(history_close, [this, history_close] {
            const auto& hist = calc_.GetHistory();
            Elements rows;
            if (hist.empty()) {
                rows.push_back(text("No history yet.") | dim | center);
            } else {
                for (const auto& [expr, res] : hist) {
                    rows.push_back(text("  " + expr + "  =  " + res));
                }
            }
            rows.push_back(separator());
            rows.push_back(history_close->Render() | center);
            return vbox(std::move(rows)) | size(WIDTH, GREATER_THAN, 50) |
                   border | clear_under | center;
        });

    // ------------------------------------------------------------------
    // Stack modals on top of the main view
    // ------------------------------------------------------------------
    auto combined = Modal(main_renderer, version_modal, &state_.show_version_modal);
    component_ = Modal(combined, history_modal, &state_.show_history_modal);
}

ftxui::Component App::GetComponent() { return component_; }
