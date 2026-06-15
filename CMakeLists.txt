cmake_minimum_required(VERSION 3.14)
project(CalcCli VERSION 1.0.0)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG        v6.1.9
)
FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

find_package(SQLite3 REQUIRED)

# ---------------------------------------------------------------------------
# Utility library (no dependencies)
# ---------------------------------------------------------------------------
add_library(util_lib src/util/formatting.cpp)
target_include_directories(util_lib PUBLIC src/)

# ---------------------------------------------------------------------------
# Layer 1: Model  (no FTXUI dependency)
# Headers accessed as: #include "model/calculator.hpp"
#                       #include "model/app_state.hpp"
# ---------------------------------------------------------------------------
add_library(calc_lib
    src/model/calculator.cpp
    src/model/parser.cpp
    src/model/history_repository.cpp
)
target_include_directories(calc_lib PUBLIC src/)
target_link_libraries(calc_lib PUBLIC util_lib SQLite3::SQLite3)

# ---------------------------------------------------------------------------
# Layer 2: Controller  (no FTXUI dependency)
# Headers accessed as: #include "controller/app_controller.hpp"
# ---------------------------------------------------------------------------
add_library(controller_lib
    src/controller/app_controller.cpp
    src/controller/history_controller.cpp
)
target_include_directories(controller_lib PUBLIC src/)
target_link_libraries(controller_lib PUBLIC calc_lib util_lib)

# ---------------------------------------------------------------------------
# Layer 3: View  (depends on FTXUI + controller_lib)
# Headers accessed as: #include "view/app.hpp"
# ---------------------------------------------------------------------------
add_library(app_lib src/view/app.cpp)
target_include_directories(app_lib PUBLIC src/)
target_link_libraries(app_lib PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
    controller_lib
)

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(calc_cli src/main.cpp)
target_link_libraries(calc_cli PRIVATE app_lib)

# ---------------------------------------------------------------------------
# Test executable — can link any layer independently to test in isolation
# ---------------------------------------------------------------------------
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests PRIVATE
    Catch2::Catch2WithMain
    app_lib
    controller_lib
    calc_lib
)
