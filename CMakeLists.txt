cmake_minimum_required(VERSION 3.0)

project(Blackhorse)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES src/main.cpp src/board.cpp src/move.cpp src/eval.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})