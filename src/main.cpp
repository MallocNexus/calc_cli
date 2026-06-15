#include "app.hpp"
#include "calculator.hpp"

#include <cstdlib>

#include <ftxui/component/screen_interactive.hpp>

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();
    AppState state;
    Calculator calc;
    App app(state, calc, [&screen] { screen.ExitLoopClosure()(); });

    screen.Loop(app.GetComponent());
    return EXIT_SUCCESS;
}
