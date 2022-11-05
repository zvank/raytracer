project(raytracer)

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CPP_COMPILER g++)

find_package(PNG)

include_directories(tools/util)
include_directories(${PNG_INCLUDE_DIRS})

add_executable(raytracer raytracer/main.cpp)
target_include_directories(raytracer PUBLIC ${PNG_INCLUDE_DIRS})
target_link_libraries(raytracer png)
