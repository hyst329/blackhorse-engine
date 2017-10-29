cmake_minimum_required(VERSION 3.0)

project(Blackhorse)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES src/main.cpp src/board.cpp src/move.cpp src/eval.cpp src/uci.cpp)
set(HEADERS src/board.h src/move.h src/eval.h src/uci.h src/magic.h)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME})

configure_file(data/blackhorse_rattacks.dat blackhorse_rattacks.dat COPYONLY)
configure_file(data/blackhorse_battacks.dat blackhorse_battacks.dat COPYONLY)

if(MINGW)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif(MINGW)
