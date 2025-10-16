cmake_minimum_required(VERSION 3.28)
project(cirbo_search)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Compilation database is needed for static linter.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json for clang-tidy" FORCE)

include_directories(src)

add_executable(cirbo_search_cli app/cirbo_search_cli.cpp)
