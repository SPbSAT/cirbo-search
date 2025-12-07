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

TEST_CASE("RedundantGatesCleaner SimpleTest", "[redundant_gates_cleaner]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "\n"
        "OUTPUT(4)\n"
        "\n"
        "4 = AND(0, 2)\n"
        "5 = OR(1, 3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, RedundantGatesCleaner<DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("RedundantGatesCleaner NotBreaksAnything", "[redundant_gates_cleaner]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "\n"
        "OUTPUT(3)\n"
        "OUTPUT(4)\n"
        "\n"
        "3 = AND(0, 1)\n"
        "4 = OR(0, 2)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, RedundantGatesCleaner<DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateType(4) == GateType::OR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3, 4}));
}

TEST_CASE("RedundantGatesCleaner NotMarkedOutput", "[redundant_gates_cleaner]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "\n"
        "OUTPUT(3)\n"
        "\n"
        "3 = AND(0, 1)\n"
        "4 = OR(0, 2, 3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, RedundantGatesCleaner<DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("RedundantGatesCleaner ConnectedOutputs", "[redundant_gates_cleaner]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "\n"
        "OUTPUT(3)\n"
        "OUTPUT(5)\n"
        "\n"
        "3 = AND(0, 1)\n"
        "4 = OR(3, 2)\n"
        "5 = NOT(4)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, RedundantGatesCleaner<DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({2, 3}));
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({5}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3, 4}));
}

TEST_CASE("RedundantGatesCleaner NewGatesMUXandCONST", "[redundant_gates_cleaner]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "\n"
        "OUTPUT(3)\n"
        "OUTPUT(4)\n"
        "\n"
        "3 = AND(0, 1)\n"
        "4 = MUX(0, 1, 5)\n"
        "5 = CONST(0)\n"
        "6 = MUX(3, 4, 2)\n"
        "7 = CONST(1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, RedundantGatesCleaner<DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateType(3) == GateType::MUX);
    REQUIRE(circuit->getGateType(4) == GateType::CONST_FALSE);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1, 4}));
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2, 3}));
}
