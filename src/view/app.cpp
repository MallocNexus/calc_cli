#include "view/app.hpp"

#include <fstream>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "controller/app_controller.hpp"
#include "model/app_state.hpp"
#include "util/constants.hpp"
#include "view/custom_exchange.hpp"

using namespace ftxui;

// ---------------------------------------------------------------------------
// App constructor — pure view: builds layout, wires events to controller.
// No if/else business logic lives here; every action is a controller call.
// ---------------------------------------------------------------------------
App::App(AppState& state, AppController& controller) : state_(state), controller_(controller) {
    // ------------------------------------------------------------------
    // Top-level horizontal animated menu: File | Edit | Exchange | Help
    // ------------------------------------------------------------------
    auto top_menu = Menu(&top_menu_entries_, &top_menu_selected_, MenuOption::HorizontalAnimated());

    // ------------------------------------------------------------------
    // Expression input (single-line)
    // Return -> controller_.OnEvaluate()
    // Escape -> controller_.OnClear()
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
            controller_.OnEvaluate();
            return true;
        }
        if (event == Event::Escape) {
            controller_.OnClear();
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
            controller_.OnQuit();
        }
    };
    auto file_menu = Menu(&file_entries_, &file_selected_, file_option);

    // ------------------------------------------------------------------
    // Edit sub-menu: Clear | History
    // ------------------------------------------------------------------
    MenuOption edit_option = MenuOption::HorizontalAnimated();
    edit_option.on_enter = [this] {
        if (edit_selected_ == EDIT_CLEAR_INPUT) {
            controller_.OnClear();
        } else if (edit_selected_ == EDIT_CLEAR_HISTORY) {
            controller_.OnClearHistory();
        }
    };
    auto edit_menu = Menu(&edit_entries_, &edit_selected_, edit_option);

    // ------------------------------------------------------------------
    // Exchange sub-menu: AUD -> USD | Custom
    // ------------------------------------------------------------------
    MenuOption exchange_option = MenuOption::HorizontalAnimated();
    exchange_option.on_enter = [this] {
        if (exchange_selected_ == EXCHANGE_AUD_USD) {
            std::string inserted = "exchange(AUD, USD)";
            state_.expression_input.insert(state_.cursor_position, inserted);
            state_.cursor_position += inserted.size();
        } else if (exchange_selected_ == EXCHANGE_CUSTOM) {
            state_.show_custom_modal = true;
        }
    };
    auto exchange_menu = Menu(&exchange_entries_, &exchange_selected_, exchange_option);

    // ------------------------------------------------------------------
    // Help sub-menu: Version
    // ------------------------------------------------------------------
    MenuOption help_option = MenuOption::HorizontalAnimated();
    help_option.on_enter = [this] {
        if (help_selected_ == HELP_VERSION) {
            controller_.OnOpenVersion();
        }
    };
    auto help_menu = Menu(&help_entries_, &help_selected_, help_option);

    // ------------------------------------------------------------------
    // Compose layout
    // ------------------------------------------------------------------
    auto tab_container =
        Container::Tab({file_menu, edit_menu, exchange_menu, help_menu}, &top_menu_selected_);

    auto history_menu_option = MenuOption::Vertical();
    history_menu_option.on_enter = [this, expr_input_base] {
        controller_.OnUseSelectedHistory();
        expr_input_base->TakeFocus();
    };
    history_menu_option.entries_option.transform = [](EntryState s) {
        Element element = text(s.label);
        if (s.focused) {
            element |= color(Color::Green) | bold;
        } else if (s.active) {
            element |= color(Color::Green);
        }
        return element;
    };

    auto history_menu = Menu(&state_.history_menu_entries, &state_.selected_history_idx, history_menu_option);

    auto main_container = Container::Vertical({top_menu, tab_container, expr_input, history_menu});

    auto main_renderer = Renderer(main_container, [this, top_menu, tab_container, expr_input, history_menu] {
        Element result_elem;
        if (state_.result_display.empty()) {
            result_elem = text("") | dim;
        } else if (state_.error_state) {
            result_elem = text(state_.result_display) | color(Color::Red);
        } else {
            result_elem = text(state_.result_display) | color(Color::Green) | bold;
        }

        Element history_elem;
        if (state_.history_menu_entries.empty()) {
            history_elem = text("  No history yet") | dim;
        } else {
            history_elem = history_menu->Render() | yframe | flex;
        }

        return vbox({
                   top_menu->Render(),
                   separator(),
                   tab_container->Render(),
                   separator(),
                   hbox({text(" > "), expr_input->Render()}),
                   separator(),
                   hbox({text("   "), result_elem}),
                   separator(),
                   text(" History:") | dim,
                   separator(),
                   history_elem,
               }) |
               border;
    });

    // ------------------------------------------------------------------
    // Version modal (Help -> Version)
    // ------------------------------------------------------------------
    auto version_close = Button("Close", [this] { controller_.OnCloseVersion(); });

    auto version_modal = Renderer(version_close, [this, version_close] {
        return vbox({
                   text("calc-cli  v" + std::string(calc_cli::kAppVersion)) | bold | center,
                   separator(),
                   text(std::string(calc_cli::kAppTitle)) | dim | center,
                   separator(),
                   version_close->Render() | center,
               }) |
               size(WIDTH, GREATER_THAN, 44) | border | clear_under | center;
    });

    // ------------------------------------------------------------------
    // Custom Exchange Modal (Exchange -> Custom)
    // ------------------------------------------------------------------
    custom_exchange_ = std::make_shared<view::CustomExchange>(state_);
    auto custom_modal = custom_exchange_->GetComponent();

    // ------------------------------------------------------------------
    // Stack modals on top of the main view
    // ------------------------------------------------------------------
    auto component_temp = Modal(main_renderer, version_modal, &state_.show_version_modal);
    component_ = Modal(component_temp, custom_modal, &state_.show_custom_modal);
}

ftxui::Component App::GetComponent() { return component_; }
