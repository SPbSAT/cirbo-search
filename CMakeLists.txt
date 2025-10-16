cmake_minimum_required(VERSION 3.28)
project(
        cirbo_search
        VERSION 0.1.0
        DESCRIPTION "Header-only library for boolean circuits synthesis and simplification."
        HOMEPAGE_URL "https://github.com/SPbSAT/cirbo-search"
        LANGUAGES CXX)

# Build options.
option(CIRBO_SEARCH_BUILD_APP "Build CLI app" ON)
option(CIRBO_SEARCH_BUILD_TESTS "Build tests (Catch2 required)" OFF)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compilation database is needed for static linter.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json for clang-tidy" FORCE)

# ------------------------------------------------------------------------------
# third_party (vendored dependencies)
# ------------------------------------------------------------------------------
add_subdirectory(third_party/cli11)
add_subdirectory(third_party/catch2 EXCLUDE_FROM_ALL)

# ------------------------------------------------------------------------------
# Header-only library: INTERFACE-target
# ------------------------------------------------------------------------------
add_library(cirbo_search INTERFACE)
add_library(cirbo_search::cirbo_search ALIAS cirbo_search)

target_include_directories(cirbo_search
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>)

file(GLOB_RECURSE CIRBO_SEARCH_HEADERS
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)
target_sources(cirbo_search INTERFACE
        FILE_SET HEADERS
        BASE_DIRS src
        FILES ${CIRBO_SEARCH_HEADERS}
)

# ------------------------------------------------------------------------------
# CLI application (see distinct CMakeLists.txt)
# ------------------------------------------------------------------------------
if (CIRBO_SEARCH_BUILD_APP)
    add_subdirectory(app)
endif ()

# ------------------------------------------------------------------------------
# Tests (optional)
# ------------------------------------------------------------------------------
if (CIRBO_SEARCH_BUILD_TESTS)
    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(tests)
    endif ()
endif ()
