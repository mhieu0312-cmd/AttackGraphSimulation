cmake_minimum_required(VERSION 3.16)

project(AttackGraphSimulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# =========================
# CORE LIBRARY
# =========================

add_library(attackgraph_core
        src/Node.cpp
        src/Edge.cpp
        src/Graph.cpp
        src/LoadDataset.cpp
        src/Dijkstra.cpp
        src/MinCut.cpp
        src/Defender.cpp
        src/Simulation.cpp
)

target_include_directories(
        attackgraph_core
        PUBLIC include
)

target_include_directories(
        attackgraph_core
        SYSTEM PRIVATE third_party
)

if(MSVC)
    target_compile_options(
            attackgraph_core
            PRIVATE /W4 /utf-8
    )
else()
    target_compile_options(
            attackgraph_core
            PRIVATE -Wall -Wextra -Wpedantic
    )
endif()


# =========================
# MAIN PROGRAM
# =========================

add_executable(
        AttackGraph
        src/main.cpp
)

target_link_libraries(
        AttackGraph
        PRIVATE attackgraph_core
)

target_compile_definitions(
        AttackGraph
        PRIVATE PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)


# =========================
# RAYLIB DESKTOP INTERFACE
# =========================

option(BUILD_GUI "Build the raylib desktop interface" ON)
set(RAYLIB_SOURCE_DIR "" CACHE PATH "Optional local raylib 6.0 source folder for offline builds")

if(BUILD_GUI)
    if(CMAKE_VERSION VERSION_LESS 3.25)
        message(FATAL_ERROR "raylib 6.0 requires CMake 3.25 or newer. Use BUILD_GUI=OFF for CLI only.")
    endif()
    enable_language(C)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)

    if(RAYLIB_SOURCE_DIR)
        add_subdirectory("${RAYLIB_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/raylib")
    else()
        include(FetchContent)
        FetchContent_Declare(raylib
            URL https://codeload.github.com/raysan5/raylib/tar.gz/refs/tags/6.0
            URL_HASH SHA256=2b3ee1e2120c7a0796b33062c7e9a694dd8a8caa56a96319ac8c8ecf54a90d0b
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(raylib)
    endif()

    add_executable(AttackGraphGUI
        gui/main.cpp
        gui/SimulationUI.cpp
        gui/SimulationUI.h
        gui/GraphView.cpp
        gui/GraphView.h
    )
    target_link_libraries(AttackGraphGUI PRIVATE attackgraph_core raylib)
    target_compile_definitions(AttackGraphGUI PRIVATE PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
    if(MSVC)
        target_compile_options(AttackGraphGUI PRIVATE /W4 /utf-8)
    else()
        target_compile_options(AttackGraphGUI PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
