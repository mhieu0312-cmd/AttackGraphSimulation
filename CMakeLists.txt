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
