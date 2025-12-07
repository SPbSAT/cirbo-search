#include "logger.hpp"
#include "io/parsers/bench_to_circuit.hpp"
#include "core/structures/dag.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

namespace
{

using var_map   = std::unordered_map<std::string, size_t>;
using gate_names = std::vector<std::string>;

void testBasicParse(
    std::string const& str,
    [[maybe_unused]] var_map map,
    gate_names const& gates)
{
    std::istringstream stream(str);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);
    auto circuit = parser.instantiate();
    (void)circuit; // not used directly here, but ensures instantiation

    for (auto const& g : gates)
    {
        cirbo::log::getGlobalLogger().info("<PARSER TEST> Checking gate encoding: ", g, "==", parser.encoder.encodeGate(g));
        REQUIRE(map.at(g) == parser.encoder.encodeGate(g));
    }
}

TEST_CASE("BenchParser One", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "\n"
        "INPUT(X)\n"
        "INPUT(Y)\n"
        "\n"
        "OUTPUT(Z)\n"
        "INPUT(F)\n";
    var_map const test_case_vm{{"X", 0}, {"Y", 1}, {"Z", 2}, {"F", 3}};
    gate_names gates{"X", "Y", "Z", "F"};
    testBasicParse(test_case, test_case_vm, gates);
}

TEST_CASE("BenchParser Two", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "\n"
        "INPUT(X)\n"
        "INPUT(Y)\n"
        "\n"
        "OUTPUT(Z)\n"
        "Z = AND(X, Y)\n";
    var_map const test_case_vm{{"X", 0}, {"Y", 1}, {"Z", 2}};
    gate_names gates{"X", "Y", "Z"};
    testBasicParse(test_case, test_case_vm, gates);
}

TEST_CASE("BenchParser LongNames", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "\n"
        "INPUT(INPUT)\n"
        "INPUT(ABCDEF)\n"
        "\n"
        "OUTPUT(OUTPUT)\n";
    var_map const test_case_vm{{"INPUT", 0}, {"ABCDEF", 1}, {"OUTPUT", 2}};
    gate_names gates{"INPUT", "ABCDEF", "OUTPUT"};
    testBasicParse(test_case, test_case_vm, gates);
}

TEST_CASE("BenchParser Spaces", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "   \n"
        "INPUT(     XXX)\n"
        "INPUT(YY     )\n"
        "\n"
        "OUTPUT(   ZZZ   )\n";
    var_map const test_case_vm{{"XXX", 0}, {"YY", 1}, {"ZZZ", 2}};
    gate_names gates{"XXX", "YY", "ZZZ"};
    testBasicParse(test_case, test_case_vm, gates);
}

TEST_CASE("BenchParser SpacesMux", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "   \n"
        " INPUT(     XXX)  \n"
        "INPUT(YY     )  \n"
        "  INPUT(  ZZZZ     )\n"
        "\n"
        "OUTPUT(   ABC   )\n"
        "   ABC  =   MUX(XXX  , YY,  ZZZZ  ) \n";
    var_map const test_case_vm{{"XXX", 0}, {"YY", 1}, {"ZZZZ", 2}, {"ABC", 3}};
    gate_names gates{"XXX", "YY", "ZZZZ", "ABC"};
    testBasicParse(test_case, test_case_vm, gates);
}

TEST_CASE("BenchParser ExternalConst", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "   \n"
        " INPUT(     XXX)  \n"
        "INPUT(YY     )  \n"
        " ZZZZ  = CONST(0)\n"
        "\n"
        "OUTPUT(   ABC   )\n"
        "   ABC  =   MUX(XXX  , YY,  ZZZZ  ) \n";
    var_map const test_case_vm{{"XXX", 0}, {"YY", 1}, {"ZZZZ", 2}, {"ABC", 3}};
    gate_names gates{"XXX", "YY", "ZZZZ", "ABC"};
    testBasicParse(test_case, test_case_vm, gates);
}

TEST_CASE("BenchParser ExternalConstGateType", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "   \n"
        " INPUT(     XXX)  \n"
        "INPUT(YY     )  \n"
        " ZZZZ  = CONST(0)\n"
        " FFFFF  = CONST(1)\n"
        "\n"
        "OUTPUT(   ABC   )\n"
        "   ABC  =   MUX(XXX  , YY,  ZZZZ  ) \n";
    
    std::istringstream stream(test_case);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);
    auto circuit = parser.instantiate();
    
    REQUIRE(parser.encoder.encodeGate("XXX")   == 0);
    REQUIRE(parser.encoder.encodeGate("YY")    == 1);
    REQUIRE(parser.encoder.encodeGate("ZZZZ")  == 2);
    REQUIRE(parser.encoder.encodeGate("FFFFF") == 3);
    REQUIRE(parser.encoder.encodeGate("ABC")   == 4);
    
    REQUIRE(circuit->getGateType(0) == cirbo::GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == cirbo::GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == cirbo::GateType::CONST_FALSE);
    REQUIRE(circuit->getGateType(3) == cirbo::GateType::CONST_TRUE);
    REQUIRE(circuit->getGateType(4) == cirbo::GateType::MUX);
}

TEST_CASE("BenchParser VDD", "[bench_parser]")
{
    std::string const test_case =
        "# Comment Line\n"
        "#\n"
        "   \n"
        " INPUT(     XXX)  \n"
        " ZZZZ  = vdd\n"
        " FFFFF  =      vdd    \n"
        "\n"
        "OUTPUT(   ABC   )\n"
        "   ABC  =   MUX(XXX  , FFFFF,  ZZZZ  ) \n";
    
    std::istringstream stream(test_case);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);
    auto circuit = parser.instantiate();
    
    REQUIRE(parser.encoder.encodeGate("XXX")   == 0);
    REQUIRE(parser.encoder.encodeGate("ZZZZ")  == 1);
    REQUIRE(parser.encoder.encodeGate("FFFFF") == 2);
    REQUIRE(parser.encoder.encodeGate("ABC")   == 3);
    
    REQUIRE(circuit->getGateType(0) == cirbo::GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == cirbo::GateType::CONST_TRUE);
    REQUIRE(circuit->getGateType(2) == cirbo::GateType::CONST_TRUE);
    REQUIRE(circuit->getGateType(3) == cirbo::GateType::MUX);
}

} // namespace
