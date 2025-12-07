#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <sstream>
#include <string>

#include "core/structures/dag.hpp"
#include "core/types.hpp"
#include "io/parsers/bench_to_circuit.hpp"
#include "minimization/composition.hpp"
#include "minimization/strategy.hpp"

using namespace cirbo;
using namespace cirbo::minimization;

TEST_CASE("ConnectSymmetricalGates SimpleTest", "[connect_symmetrical]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "OUTPUT(7)\n"
        "4 = AND(0, 1)\n"
        "5 = AND(2, 4)\n"
        "6 = AND(3, 4)\n"
        "7 = AND(5, 6)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, ConnectSymmetricalGates<DAG, true, true, true> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1, 2, 3}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4}));
}

TEST_CASE("ConnectSymmetricalGates Cycle", "[connect_symmetrical]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "OUTPUT(7)\n"
        "4 = OR(0, 1)\n"
        "5 = OR(2, 4)\n"
        "6 = OR(3, 4)\n"
        "7 = OR(5, 6)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, ConnectSymmetricalGates<DAG, true, true, true> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::OR);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1, 2, 3}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4}));
}

TEST_CASE("ConnectSymmetricalGates UseMiddleGates", "[connect_symmetrical]")
{
    std::string const dag =
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "INPUT(4)\n"
        "INPUT(5)\n"
        "INPUT(6)\n"
        "INPUT(7)\n"
        "OUTPUT(15)\n"
        "8 = AND(1, 2)\n"
        "9 = AND(8, 10)\n"
        "10 = XOR(5, 6)\n"
        "11 = XOR(3, 10)\n"
        "12 = XOR(4, 10)\n"
        "13 = XOR(11, 12)\n"
        "14 = AND(6, 9)\n"
        "15 = XOR(13, 14)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, ConnectSymmetricalGates<DAG, true, true, true> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 9);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::INPUT);
    REQUIRE(circuit->getGateType(5) == GateType::INPUT);
    REQUIRE(circuit->getGateType(6) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({2, 3, 7, 7, 8}));
    REQUIRE(circuit->getGateType(7) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({4, 5}));
    REQUIRE(circuit->getGateType(8) == GateType::AND);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({0, 1, 5, 7}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({6}));
}

TEST_CASE("ConnectSymmetricalGates NoChanges", "[connect_symmetrical]")
{
    std::string const dag =
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "OUTPUT(5)\n"
        "4 = AND(1, 2)\n"
        "5 = OR(4, 3)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, ConnectSymmetricalGates<DAG, true, true, true> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::OR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({2, 4}));
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3}));
}

TEST_CASE("ConnectSymmetricalGates SeveralOutputs", "[connect_symmetrical]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "OUTPUT(7)\n"
        "OUTPUT(9)\n"
        "4 = OR(0, 1)\n"
        "5 = OR(2, 4)\n"
        "6 = OR(3, 4)\n"
        "7 = OR(5, 6)\n"
        "8 = AND(0, 1)\n"
        "9 = AND(2, 8)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, ConnectSymmetricalGates<DAG, true, true, true> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::OR);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1, 2, 3}));
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 1, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4, 5}));
}

TEST_CASE("ConnectSymmetricalGates NewGatesMUX", "[connect_symmetrical]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "OUTPUT(7)\n"
        "4 = OR(0, 1)\n"
        "5 = OR(2, 4)\n"
        "6 = MUX(3, 0, 4)\n"
        "7 = OR(5, 6)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, ConnectSymmetricalGates<DAG, true, true, true> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 7);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::OR);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({2, 5, 6}));
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(6) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({3, 0, 5}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4}));
}
