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

TEST_CASE("MergeNotWithOthers SimpleTest", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(3)\n"
                            "2 = AND(0, 1)\n"
                            "3 = NOT(2)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NAND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("MergeNotWithOthers ChangeOutput", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(3)\n"
                            "2 = NOR(0, 1)\n"
                            "3 = NOT(2)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("MergeNotWithOthers NoChanges", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "OUTPUT(2)\n"
                            "1 = NOT(0)\n"
                            "2 = NOT(1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({2}));
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({1}));
}

TEST_CASE("MergeNotWithOthers ChangeLink", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "OUTPUT(6)\n"
                            "3 = NOR(0, 1)\n"
                            "4 = NOT(3)\n"
                            "5 = AND(2, 3)\n"
                            "6 = AND(4, 5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 7);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({5, 6}));
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({5}));
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(6) == GateType::AND);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({2, 4}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3}));
}

TEST_CASE("MergeNotWithOthers SeveralOutputs1", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "OUTPUT(4)\n"
                            "OUTPUT(6)\n"
                            "3 = NOT(1)\n"
                            "4 = NOR(0, 3)\n"
                            "5 = XOR(1, 2)\n"
                            "6 = NOT(5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::NOR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 5}));
    REQUIRE(circuit->getGateType(4) == GateType::NXOR);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({1, 2}));
    REQUIRE(circuit->getGateType(5) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3, 4}));
}

TEST_CASE("MergeNotWithOthers SeveralOutputs2", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(2)\n"
                            "OUTPUT(3)\n"
                            "2 = NOR(0, 1)\n"
                            "3 = NOT(2)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(3) == GateType::OR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2, 3}));
}

TEST_CASE("MergeNotWithOthers NewGateMUX", "[merge_not_with_others]")
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(4)\n"
                            "2 = CONST(1)\n"
                            "3 = MUX(0, 1, 2)\n"
                            "4 = NOT(3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        MergeNotWithOthers<DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(3) == GateType::CONST_TRUE);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({}));
    REQUIRE(circuit->getGateType(4) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1, 3}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}
