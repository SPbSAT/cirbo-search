#include "logger.hpp"
#include "utils/encoder.hpp"

#include <string>

#include <catch2/catch_test_macros.hpp>

using namespace cirbo;
using namespace cirbo::utils;


TEST_CASE("Encoder EncodeDecode", "[encoder]")
{
    log::getGlobalLogger().info("Testing simple encoding/decoding");

    NameEncoder enc;
    enc.encodeGate("a");
    enc.encodeGate("b");
    enc.encodeGate("c");
    enc.encodeGate("d");
    enc.encodeGate("e");

    REQUIRE(enc.size() == 5);

    REQUIRE(enc.encodeGate("a") == 0);
    REQUIRE(enc.encodeGate("b") == 1);
    REQUIRE(enc.encodeGate("c") == 2);
    REQUIRE(enc.encodeGate("d") == 3);
    REQUIRE(enc.encodeGate("e") == 4);

    REQUIRE(enc.decodeGate(0) == "a");
    REQUIRE(enc.decodeGate(1) == "b");
    REQUIRE(enc.decodeGate(2) == "c");
    REQUIRE(enc.decodeGate(3) == "d");
    REQUIRE(enc.decodeGate(4) == "e");
}
