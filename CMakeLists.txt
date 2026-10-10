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
# COPY DATASET
# =========================

add_custom_command(
        TARGET AttackGraph
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:AttackGraph>/data"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json"
        "$<TARGET_FILE_DIR:AttackGraph>/data/graph.json"
)
# CLI regression checks use the current real dataset.
include(CTest)
if(BUILD_TESTING)
    add_test(NAME cli_default COMMAND AttackGraph)
    set_tests_properties(cli_default PROPERTIES PASS_REGULAR_EXPRESSION "Cost: 28")
    add_test(NAME cli_over_budget COMMAND AttackGraph
        "${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json" 36 3 27)
    set_tests_properties(cli_over_budget PROPERTIES PASS_REGULAR_EXPRESSION "OVER_BUDGET")
    add_test(NAME cli_manual_patch COMMAND AttackGraph
        "${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json" 36 3 30 --patch 2 6 7)
    set_tests_properties(cli_manual_patch PROPERTIES PASS_REGULAR_EXPRESSION "Reachable: 1 -> 0")
    add_test(NAME cli_invalid_node COMMAND AttackGraph
        "${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json" 999 3 30)
    set_tests_properties(cli_invalid_node PROPERTIES WILL_FAIL TRUE)
endif()

if(BUILD_TESTING)
    add_executable(graph_validation_dfs_tests tests/graph_validation_dfs_tests.cpp)
    target_link_libraries(graph_validation_dfs_tests PRIVATE attackgraph_core)
    target_include_directories(graph_validation_dfs_tests SYSTEM PRIVATE third_party)
    target_compile_definitions(graph_validation_dfs_tests PRIVATE
        DATASET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json")
    foreach(group dataset direct_errors json_errors warnings dfs)
        add_test(NAME graph_${group} COMMAND graph_validation_dfs_tests ${group})
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(mincut_tests tests/mincut_tests.cpp)
    target_link_libraries(mincut_tests PRIVATE attackgraph_core)
    target_compile_definitions(mincut_tests PRIVATE
        DATASET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json")
    foreach(group single multiple parallel cycles zero blocked invalid disconnected large reverse dataset exhaustive)
        add_test(NAME mincut_${group} COMMAND mincut_tests ${group})
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(simulation_tests tests/simulation_tests.cpp)
    target_link_libraries(simulation_tests PRIVATE attackgraph_core)
    target_compile_definitions(simulation_tests PRIVATE
        DATASET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json")
    foreach(group auto_multiple auto_single disconnected same_node manual budget atomic independent unchanged zero)
        add_test(NAME simulation_${group} COMMAND simulation_tests ${group})
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(hacker_tests tests/hacker_tests.cpp)
    target_link_libraries(hacker_tests PRIVATE attackgraph_core)
    target_compile_definitions(hacker_tests PRIVATE
        DATASET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/graph.json")
    foreach(group basic can_move budget edge_cases scenarios)
        add_test(NAME hacker_${group} COMMAND hacker_tests ${group})
    endforeach()
endif()

