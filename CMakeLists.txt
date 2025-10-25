cmake_minimum_required(VERSION 3.23)

# Project meta
project(schack
    VERSION 0.1.0
    DESCRIPTION "A modular C++ chess engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options (handy while developing)
option(CH_ENABLE_SANITIZERS "Enable Address/Undefined sanitizers in Debug" ON)
option(CH_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Library with your bitboard implementation
add_library(chess_core
    src/core/ch_bitboard.cpp
    src/core/ch_board.cpp
    src/core/ch_state.cpp
    src/pieces/ch_piece.cpp
    src/analysis/ch_attack.cpp
    src/analysis/ch_pins.cpp
    src/analysis/ch_legality.cpp
    src/gen/ch_legal_masks.cpp
    src/gen/ch_legalize.cpp
    src/gen/ch_movegen.cpp
    src/gen/ch_king_legal.cpp)
target_include_directories(chess_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Warnings
if (MSVC)
    target_compile_options(chess_core PRIVATE /W4)
else()
    target_compile_options(chess_core PRIVATE -Wall -Wextra -Wpedantic)
    if (CH_WARNINGS_AS_ERRORS)
        target_compile_options(chess_core PRIVATE -Werror)
    endif()
endif()

# ---- Smoke test executable ---
add_executable(ch_bb_smoke tests/ch_pins_king_legal.cpp)
target_link_libraries(ch_bb_smoke PRIVATE chess_core)

# --- GUI Build ---
find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window System)

add_executable(chess_gui gui/chess_gui.cpp)

target_include_directories(chess_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(chess_gui PRIVATE
    chess_core
    SFML::Graphics
    SFML::Window
    SFML::System)
target_compile_features(chess_gui PRIVATE cxx_std_20)

# A handy target to run the smoke test from your build
add_custom_target(run_bb_smoke
    COMMAND ch_bb_smoke
    DEPENDS ch_bb_smoke
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )