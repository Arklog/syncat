cmake_minimum_required(VERSION 3.31)
project(syncat)

include(FetchContent)

FetchContent_Declare(
    indicators
    GIT_REPOSITORY https://github.com/p-ranav/indicators
    GIT_TAG v2.3
)
FetchContent_MakeAvailable(indicators)

set(CMAKE_CXX_STANDARD 26)

add_executable(syncat main.cpp)
target_link_libraries(syncat PRIVATE indicators)