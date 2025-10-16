# Supposed to be used through `add_subdirectory(app)` from root.

# Executable file.
add_executable(cirbo_search_cli cirbo_search_cli.cpp)

target_link_libraries(cirbo_search_cli
        PRIVATE
        cirbo_search::cirbo_search
        CLI11::CLI11
)

target_compile_features(cirbo_search_cli PRIVATE cxx_std_20)

# ---- CLI Profile -----------------------------------------------------------------
option(CIRBO_SEARCH_APP_DEBUG "Build CLI with debug flags (-O0 -g, no NDEBUG, no IPO)" OFF)
option(CIRBO_SEARCH_APP_SANITIZE "Enable Address+UB sanitizers for CLI (gcc/clang)" OFF)
option(CIRBO_SEARCH_APP_NATIVE "Enable -march=native (non-portable binary)" OFF)

# Optimization options
option(CIRBO_SEARCH_APP_OFAST "Use -Ofast instead of -O3 (implies fast-math on gcc/clang)" OFF)
option(CIRBO_SEARCH_APP_UNROLL "Force loop unrolling flags" OFF)

# Specific architecture (MSVC: /arch:AVX2; gcc/clang: -march=<val>)
set(CIRBO_SEARCH_APP_ARCH "" CACHE STRING "Override CPU arch (e.g. native, x86-64-v3, znver3, skylake; MSVC: AVX2)")

if (CIRBO_SEARCH_APP_DEBUG)
    # --- Debug profile ------------------------------------------------------------
    if (MSVC)
        target_compile_options(cirbo_search_cli PRIVATE /Od /Zi)
        target_link_options(cirbo_search_cli PRIVATE /DEBUG)
        target_compile_definitions(cirbo_search_cli PRIVATE _DEBUG NDEBUG=0)
    else ()
        target_compile_options(cirbo_search_cli PRIVATE -O0 -g3)
        target_link_options(cirbo_search_cli PRIVATE -g3)
        target_compile_definitions(cirbo_search_cli PRIVATE NDEBUG=0)
    endif ()
    set_property(TARGET cirbo_search_cli PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

else ()
    # --- Release profile ------------------------------------------------
    if (MSVC)
        target_compile_options(cirbo_search_cli PRIVATE /O2 /DNDEBUG /Oi)

        if (CIRBO_SEARCH_APP_OFAST)
            target_compile_options(cirbo_search_cli PRIVATE /fp:fast)
        endif ()

        if (CIRBO_SEARCH_APP_ARCH MATCHES "^[Aa][Vv][Xx]2$")
            target_compile_options(cirbo_search_cli PRIVATE /arch:AVX2)
        elseif (CIRBO_SEARCH_APP_NATIVE)
            # MSVC has no «native».
        endif ()
    else ()
        # -Ofast (if enabled) or -O3
        if (CIRBO_SEARCH_APP_OFAST)
            target_compile_options(cirbo_search_cli PRIVATE -Ofast -DNDEBUG)
        else ()
            target_compile_options(cirbo_search_cli PRIVATE -O3 -DNDEBUG)
        endif ()

        # CPU architecture.
        if (CIRBO_SEARCH_APP_ARCH STREQUAL "")
            if (CIRBO_SEARCH_APP_NATIVE)
                target_compile_options(cirbo_search_cli PRIVATE -march=native)
            endif ()
        else ()
            target_compile_options(cirbo_search_cli PRIVATE -march=${CIRBO_SEARCH_APP_ARCH})
        endif ()

        # Loop unrolling and inlining.
        if (CIRBO_SEARCH_APP_UNROLL)
            target_compile_options(cirbo_search_cli PRIVATE -funroll-loops -finline-functions -finline-functions-called-once)
        endif ()

        # Useful micro optimizations.
        target_compile_options(cirbo_search_cli PRIVATE -fstrict-aliasing -fno-plt)
    endif ()

    # IPO/LTO if possible
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _ipo_ok OUTPUT _ipo_msg)
    if (_ipo_ok)
        set_property(TARGET cirbo_search_cli PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
endif ()

# --- Sanitizers (gcc/clang) -------------------------------------------------------
if (CIRBO_SEARCH_APP_SANITIZE AND NOT MSVC)
    target_compile_options(cirbo_search_cli PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(cirbo_search_cli PRIVATE -fsanitize=address,undefined)
    # Sanitizers sometimes are incompatible with IPO.
    set_property(TARGET cirbo_search_cli PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
endif ()
