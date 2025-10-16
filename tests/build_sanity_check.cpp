#include <catch2/catch_test_macros.hpp>

TEST_CASE("x + y = z", "[sum]") {
    int x = 2, y = 3;
    int z = x + y;
    REQUIRE(z == 5);
}
