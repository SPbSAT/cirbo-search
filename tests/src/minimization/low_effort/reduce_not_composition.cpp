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

TEST_CASE("ReduceNotComposition SimpleTest", "[reduce_not_composition]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(9)\n"
        "6 = NOT(0)\n"
        "7 = NOT(6)\n"
        "8 = NOT(7)\n"
        "9 = AND(8, 1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, cirbo::minimization::ReduceNotComposition<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({1, 3}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("ReduceNotComposition UseMiddleNot", "[reduce_not_composition]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(7)\n"
        "2 = NOT(0)\n"
        "3 = NOT(2)\n"
        "4 = NOT(3)\n"
        "5 = NOT(4)\n"
        "6 = MUX(4, 0, 1)\n"
        "7 = AND(5, 6)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, cirbo::minimization::ReduceNotComposition<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 4}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(4) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({3, 0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("ReduceNotComposition NotIsOutput", "[reduce_not_composition]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(8)\n"
        "OUTPUT(9)\n"
        "6 = NOT(0)\n"
        "7 = NOT(6)\n"
        "8 = NOT(7)\n"
        "9 = AND(7, 1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, cirbo::minimization::ReduceNotComposition<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2, 3}));
}

TEST_CASE("ReduceNotComposition NoChanges", "[reduce_not_composition]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(9)\n"
        "6 = NOT(0)\n"
        "7 = NOR(6, 0)\n"
        "8 = NOT(7)\n"
        "9 = AND(8, 1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] =
        Composition<DAG, cirbo::minimization::ReduceNotComposition<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({1, 5}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(4) == GateType::NOR);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 3}));
    REQUIRE(circuit->getGateType(5) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({4}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}
