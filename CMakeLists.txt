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


# =========================
# COPY DEMO FIXTURE
# =========================

add_custom_command(
        TARGET AttackGraph
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:AttackGraph>/fixtures"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/p2_demo.json"
        "$<TARGET_FILE_DIR:AttackGraph>/fixtures/p2_demo.json"
)


# =========================
# TESTING
# =========================

include(CTest)

if(BUILD_TESTING)

    add_executable(
            attackgraph_tests
            tests/test_main.cpp
    )

    target_link_libraries(
            attackgraph_tests
            PRIVATE attackgraph_core
    )

    foreach(group graph loader attack defender integration regression)

        add_test(
                NAME ${group}
                COMMAND attackgraph_tests ${group}
        )

        set_tests_properties(
                ${group}
                PROPERTIES
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )

    endforeach()


    add_test(
            NAME cli_demo
            COMMAND AttackGraph --demo
    )

    set_tests_properties(
            cli_demo
            PROPERTIES
            PASS_REGULAR_EXPRESSION "NO_PATH: khong con duong di"
    )


    add_test(
            NAME cli_manual_budget
            COMMAND AttackGraph
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/p2_demo.json"
            0 3 15 --patch 2
    )

    set_tests_properties(
            cli_manual_budget
            PROPERTIES
            PASS_REGULAR_EXPRESSION "OVER_BUDGET: van co duong"
    )


    add_test(
            NAME cli_missing_dataset
            COMMAND AttackGraph missing.json 0 3 15
    )

    set_tests_properties(
            cli_missing_dataset
            PROPERTIES
            WILL_FAIL TRUE
    )

endif()