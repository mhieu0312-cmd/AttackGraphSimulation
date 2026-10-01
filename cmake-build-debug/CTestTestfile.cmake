# CMake generated Testfile for 
# Source directory: C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation
# Build directory: C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(graph "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/attackgraph_tests.exe" "graph")
set_tests_properties(graph PROPERTIES  WORKING_DIRECTORY "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;121;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(loader "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/attackgraph_tests.exe" "loader")
set_tests_properties(loader PROPERTIES  WORKING_DIRECTORY "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;121;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(attack "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/attackgraph_tests.exe" "attack")
set_tests_properties(attack PROPERTIES  WORKING_DIRECTORY "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;121;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(defender "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/attackgraph_tests.exe" "defender")
set_tests_properties(defender PROPERTIES  WORKING_DIRECTORY "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;121;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(integration "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/attackgraph_tests.exe" "integration")
set_tests_properties(integration PROPERTIES  WORKING_DIRECTORY "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;121;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(regression "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/attackgraph_tests.exe" "regression")
set_tests_properties(regression PROPERTIES  WORKING_DIRECTORY "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;121;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(cli_demo "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/AttackGraph.exe" "--demo")
set_tests_properties(cli_demo PROPERTIES  PASS_REGULAR_EXPRESSION "NO_PATH: khong con duong di" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;135;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(cli_manual_budget "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/AttackGraph.exe" "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/tests/fixtures/p2_demo.json" "0" "3" "15" "--patch" "2")
set_tests_properties(cli_manual_budget PROPERTIES  PASS_REGULAR_EXPRESSION "OVER_BUDGET: van co duong" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;147;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
add_test(cli_missing_dataset "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/cmake-build-debug/AttackGraph.exe" "missing.json" "0" "3" "15")
set_tests_properties(cli_missing_dataset PROPERTIES  WILL_FAIL "TRUE" _BACKTRACE_TRIPLES "C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;161;add_test;C:/Users/mhieu/OneDrive/Desktop/AttackGraphSimulation/CMakeLists.txt;0;")
