cmake_minimum_required(VERSION 3.0)

project(Blackhorse)

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES src/main.cpp src/board.cpp src/move.cpp src/eval.cpp src/uci.cpp)
set(HEADERS src/board.h src/move.h src/eval.h src/uci.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES})

if(MINGW)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
find_package(Threads REQUIRED)  
target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif(MINGW)
