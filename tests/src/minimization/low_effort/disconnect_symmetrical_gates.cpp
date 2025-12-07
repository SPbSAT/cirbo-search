#include "core/types.hpp"
#include "core/structures/dag.hpp"
#include "io/parsers/bench_to_circuit.hpp"

#include "minimization/composition.hpp"
#include "minimization/strategy.hpp"

#include <string>
#include <sstream>
#include <memory>

#include <catch2/catch_test_macros.hpp>

using namespace cirbo;
using namespace cirbo::minimization;

TEST_CASE("DisconnectSymmetricalGates SimpleTest", "[disconnect_symmetrical_gates]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "INPUT(3)\n"
                            "INPUT(4)\n"
                            "INPUT(5)\n"
                            "INPUT(6)\n"
                            "\n"
                            "OUTPUT(8)\n"
                            "\n"
                            "7 = AND(0, 1, 2, 3, 4, 5)\n"
                            "8 = XOR(6, 7)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DisconnectSymmetricalGates<cirbo::DAG, 3, true, true, true>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 11);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::INPUT);
    REQUIRE(circuit->getGateType(5) == GateType::INPUT);
    REQUIRE(circuit->getGateType(6) == GateType::INPUT);
    REQUIRE(circuit->getGateType(7) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({6, 8}));
    REQUIRE(circuit->getGateType(8) == GateType::AND);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({5, 10}));
    REQUIRE(circuit->getGateType(9) == GateType::AND);
    REQUIRE(circuit->getGateOperands(9) == GateIdContainer({0, 1, 2}));
    REQUIRE(circuit->getGateType(10) == GateType::AND);
    REQUIRE(circuit->getGateOperands(10) == GateIdContainer({3, 4, 9}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({7}));
}

TEST_CASE("DisconnectSymmetricalGates SeveralOutputs", "[disconnect_symmetrical_gates]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "INPUT(3)\n"
                            "INPUT(4)\n"
                            "INPUT(5)\n"
                            "INPUT(6)\n"
                            "OUTPUT(7)\n"
                            "OUTPUT(10)\n"
                            "7 = AND(0, 1, 2, 3)\n"
                            "8 = XOR(0, 1, 2)\n"
                            "9 = CONST(0)\n"
                            "10 = MUX(0, 8, 9)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DisconnectSymmetricalGates<cirbo::DAG, 2, true, true, true>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 11);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({3, 9}));
    REQUIRE(circuit->getGateType(5) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 6, 7}));
    REQUIRE(circuit->getGateType(6) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({2, 10}));
    REQUIRE(circuit->getGateType(7) == GateType::CONST_FALSE);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({}));
    REQUIRE(circuit->getGateType(8) == GateType::AND);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(9) == GateType::AND);
    REQUIRE(circuit->getGateOperands(9) == GateIdContainer({2, 8}));
    REQUIRE(circuit->getGateType(10) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(10) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4, 5}));
}

TEST_CASE("DisconnectSymmetricalGates NoChanges", "[disconnect_symmetrical_gates]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "OUTPUT(3)\n"
                            "3 = XOR(0, 1, 2)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DisconnectSymmetricalGates<cirbo::DAG, 2, false, false, false>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3}));
}

TEST_CASE("DisconnectSymmetricalGates PartChanges", "[disconnect_symmetrical_gates]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "OUTPUT(3)\n"
                            "OUTPUT(4)\n"
                            "3 = XOR(0, 1, 2)\n"
                            "4 = AND(0, 1, 2)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DisconnectSymmetricalGates<cirbo::DAG, 2, true, true, false>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1, 2}));
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({2, 5}));
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3, 4}));
}
