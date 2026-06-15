cmake_minimum_required(VERSION 3.14)
project(CalcCli VERSION 1.0.0)

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

# Pure logic library — NO FTXUI dependency (keeps it unit-testable in isolation)
add_library(calc_lib src/calculator.cpp)
target_include_directories(calc_lib PUBLIC src/)

# UI library — depends on FTXUI and calc_lib
add_library(app_lib src/app.cpp)
target_include_directories(app_lib PUBLIC src/)
target_link_libraries(app_lib PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
    calc_lib
)

# Main executable
add_executable(calc_cli src/main.cpp)
target_link_libraries(calc_cli PRIVATE app_lib)

# Test executable
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests PRIVATE Catch2::Catch2WithMain app_lib calc_lib)
