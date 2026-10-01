cmake_minimum_required(VERSION 3.16)

project(AttackGraph)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(AttackGraph
        src/main.cpp
        src/Node.cpp
        src/Edge.cpp
        src/Graph.cpp
        src/Dijkstra.cpp
        include/Dijkstra.h
        src/MinCut.cpp
        include/MinCut.h
)

target_include_directories(AttackGraph PRIVATE
        include
)