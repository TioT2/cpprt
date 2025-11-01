cmake_minimum_required(VERSION 3.14)

project(cpprt)

find_package(SDL3 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE "source" CONFIGURE_DEPENDS src/*.cpp)

add_executable(cpprt ${source})
# target_compile_options(cpprt PRIVATE -fno-rtti -fno-exceptions)
target_include_directories(cpprt PRIVATE src/)

target_link_libraries(cpprt PRIVATE SDL3::SDL3)
