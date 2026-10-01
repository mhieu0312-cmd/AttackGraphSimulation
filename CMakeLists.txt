cmake_minimum_required(VERSION 3.10)
project(AttackGraphDefender)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Khai báo 2 file .cpp cần biên dịch
add_executable(AttackGraphDefender main.cpp Defender.cpp)