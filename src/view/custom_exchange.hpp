#pragma once

#include <ftxui/component/component.hpp>
#include "model/app_state.hpp"

namespace view {

// ---------------------------------------------------------------------------
// CustomExchange
// Encapsulates the modal dialog UI for custom currency exchange entry.
// ---------------------------------------------------------------------------
class CustomExchange {
  public:
    explicit CustomExchange(AppState& state);
    ftxui::Component GetComponent();

  private:
    AppState& state_;
    ftxui::Component component_;
};

} // namespace view
