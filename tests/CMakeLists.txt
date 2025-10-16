file(GLOB_RECURSE CIRBO_SEARCH_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)


add_executable(cirbo_search_tests
        ${CIRBO_SEARCH_TEST_SOURCES}
)

target_compile_features(cirbo_search_tests PRIVATE cxx_std_20)

target_link_libraries(cirbo_search_tests PRIVATE
        cirbo_search::cirbo_search
        Catch2::Catch2WithMain
)

if (MSVC)
    target_compile_options(cirbo_search_tests PRIVATE /fsanitize=address)
    target_link_options(cirbo_search_tests PRIVATE /fsanitize=address)

    set_property(TARGET cirbo_search_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
else ()
    target_compile_options(cirbo_search_tests PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
    )
    target_link_options(cirbo_search_tests PRIVATE
            -fsanitize=address,undefined
    )

    set_property(TARGET cirbo_search_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
endif ()

# Test registration.
add_test(NAME cirbo_search_tests COMMAND cirbo_search_tests)