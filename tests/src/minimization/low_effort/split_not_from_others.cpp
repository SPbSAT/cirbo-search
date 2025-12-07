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

TEST_CASE("SplitNotFromOthers ChangeOutput", "[split_not_from_others]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(2)\n"
        "2 = NAND(0, 1)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        SplitNotFromOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({3}));
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("SplitNotFromOthers DifferentOperators", "[split_not_from_others]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(4)\n"
        "2 = NAND(0, 1)\n"
        "3 = NOR(2, 1)\n"
        "4 = NXOR(3, 0)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        SplitNotFromOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 8);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({7})); // XOR
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({5})); // AND
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({6})); // OR
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(6) == GateType::OR);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({1, 3}));
    REQUIRE(circuit->getGateType(7) == GateType::XOR);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({0, 4}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("SplitNotFromOthers UnusedGates", "[split_not_from_others]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(2)\n"
        "2 = NAND(0, 1)\n"
        "3 = NOR(2, 0)";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        SplitNotFromOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({5}));
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("SplitNotFromOthers NotBesideInput", "[split_not_from_others]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(4)\n"
        "2 = NOT(0)\n"
        "3 = NOR(2, 6)\n"
        "4 = NOT(3)\n"
        "5 = CONST(1)\n"
        "6 = MUX(2, 5, 1)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        SplitNotFromOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 8);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({7}));
    REQUIRE(circuit->getGateType(5) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({3, 6, 1}));
    REQUIRE(circuit->getGateType(6) == GateType::CONST_TRUE);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({}));
    REQUIRE(circuit->getGateType(7) == GateType::OR);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({3, 5}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("SplitNotFromOthers NoChanges", "[split_not_from_others]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(3)\n"
        "2 = NOT(0)\n"
        "3 = NOT(2)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        SplitNotFromOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({3}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("SplitNotFromOthers SeveralOutputs", "[split_not_from_others]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(3)\n"
        "OUTPUT(4)\n"
        "2 = NOT(0)\n"
        "3 = NOR(2)\n"
        "4 = NAND(0, 1)\n";

    std::istringstream stream(dag);
    io::parsers::BenchToCircuit<DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<DAG> csat_instance = parser.instantiate();
    utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        SplitNotFromOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 7);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({5}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({6}));
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(6) == GateType::AND);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2, 3}));
}
