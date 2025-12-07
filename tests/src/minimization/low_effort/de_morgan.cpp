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

TEST_CASE("DeMorgan SimpleTest", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(2)\n"
        "2 = NOR(0, 1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 5);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({3, 4}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("DeMorgan Rehang", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(4)\n"
        "2 = NOR(0, 1)\n"
        "3 = NAND(0, 2)\n"
        "4 = NAND(2, 3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(3) == GateType::OR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({4, 5}));
    REQUIRE(circuit->getGateType(4) == GateType::OR);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3}));
}

TEST_CASE("DeMorgan UseCurrentNot", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "OUTPUT(4)\n"
        "2 = NOT(0)\n"
        "3 = NOR(0, 1)\n"
        "4 = AND(2, 3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 6);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({1, 5}));
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({1, 3}));
    REQUIRE(circuit->getGateType(5) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4}));
}

TEST_CASE("DeMorgan NotComposition", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "OUTPUT(6)\n"
        "3 = OR(0, 1)\n"
        "4 = NOT(3)\n"
        "5 = OR(2, 3)\n"
        "6 = NAND(4, 5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 8);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({2}));
    REQUIRE(circuit->getGateType(4) == GateType::INPUT);
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({2, 6}));
    REQUIRE(circuit->getGateType(6) == GateType::AND);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({3, 7}));
    REQUIRE(circuit->getGateType(7) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({4}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({5}));
}

TEST_CASE("DeMorgan DontMergeNot1", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "OUTPUT(6)\n"
        "3 = OR(0, 1)\n"
        "4 = NOT(3)\n"
        "5 = OR(2, 3)\n"
        "6 = AND(4, 5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 7);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({2}));
    REQUIRE(circuit->getGateType(4) == GateType::INPUT);
    REQUIRE(circuit->getGateType(5) == GateType::OR);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({2, 4}));
    REQUIRE(circuit->getGateType(6) == GateType::AND);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({3, 5}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({6}));
}

TEST_CASE("DeMorgan DontMergeNot2", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "OUTPUT(6)\n"
        "3 = OR(0, 1)\n"
        "4 = NOT(3)\n"
        "5 = NOR(2, 3)\n"
        "6 = AND(4, 5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 8);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getGateType(3) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({2}));
    REQUIRE(circuit->getGateType(4) == GateType::INPUT);
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({3, 7}));
    REQUIRE(circuit->getGateType(6) == GateType::AND);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({3, 5}));
    REQUIRE(circuit->getGateType(7) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({4}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({6}));
}

TEST_CASE("DeMorgan LongChain", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "OUTPUT(6)\n"
        "4 = NOR(0, 1)\n"
        "5 = AND(2, 4)\n"
        "6 = NOR(3, 5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 9);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({5, 6}));
    REQUIRE(circuit->getGateType(5) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(6) == GateType::OR);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({7, 8}));
    REQUIRE(circuit->getGateType(7) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({1}));
    REQUIRE(circuit->getGateType(8) == GateType::OR);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({2, 3}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({4}));
}

TEST_CASE("DeMorgan Cycle1", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "INPUT(4)\n"
        "INPUT(5)\n"
        "OUTPUT(12)\n"
        "6 = AND(0, 1)\n"
        "7 = AND(2, 6)\n"
        "8 = AND(3, 6)\n"
        "9 = OR(4, 5)\n"
        "10 = NOR(8, 9)\n"
        "11 = NAND(7, 8)\n"
        "12 = AND(10, 11)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 19);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::INPUT);
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({9, 13}));
    REQUIRE(circuit->getGateType(6) == GateType::INPUT);
    REQUIRE(circuit->getGateType(7) == GateType::OR);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({9, 10}));
    REQUIRE(circuit->getGateType(8) == GateType::AND);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({5, 7}));
    REQUIRE(circuit->getGateType(9) == GateType::OR);
    REQUIRE(circuit->getGateOperands(9) == GateIdContainer({11, 16}));
    REQUIRE(circuit->getGateType(10) == GateType::OR);
    REQUIRE(circuit->getGateOperands(10) == GateIdContainer({11, 12}));
    REQUIRE(circuit->getGateType(11) == GateType::OR);
    REQUIRE(circuit->getGateOperands(11) == GateIdContainer({17, 18}));
    REQUIRE(circuit->getGateType(12) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(12) == GateIdContainer({6}));
    REQUIRE(circuit->getGateType(13) == GateType::AND);
    REQUIRE(circuit->getGateOperands(13) == GateIdContainer({14, 15}));
    REQUIRE(circuit->getGateType(14) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(14) == GateIdContainer({3}));
    REQUIRE(circuit->getGateType(15) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(15) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(16) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(16) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(17) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(17) == GateIdContainer({1}));
    REQUIRE(circuit->getGateType(18) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(18) == GateIdContainer({2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({8}));
}

TEST_CASE("DeMorgan CycleplusSeveralOutputs", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "INPUT(4)\n"
        "INPUT(5)\n"
        "OUTPUT(10)\n"
        "OUTPUT(11)\n"
        "6 = AND(0, 1)\n"
        "7 = AND(2, 6)\n"
        "8 = AND(3, 6)\n"
        "9 = OR(4, 5)\n"
        "10 = OR(8, 9)\n"
        "11 = NAND(7, 8)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 15);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({1, 2}));
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 3}));
    REQUIRE(circuit->getGateType(5) == GateType::INPUT);
    REQUIRE(circuit->getGateType(6) == GateType::INPUT);
    REQUIRE(circuit->getGateType(7) == GateType::OR);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({5, 6}));
    REQUIRE(circuit->getGateType(8) == GateType::OR);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({4, 7}));
    REQUIRE(circuit->getGateType(9) == GateType::INPUT);
    REQUIRE(circuit->getGateType(10) == GateType::OR);
    REQUIRE(circuit->getGateOperands(10) == GateIdContainer({11, 12}));
    REQUIRE(circuit->getGateType(11) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(11) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(12) == GateType::OR);
    REQUIRE(circuit->getGateOperands(12) == GateIdContainer({13, 14}));
    REQUIRE(circuit->getGateType(13) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(13) == GateIdContainer({3}));
    REQUIRE(circuit->getGateType(14) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(14) == GateIdContainer({9}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({8, 10}));
}

TEST_CASE("DeMorgan StopDeMorgan", "[demorgan]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "INPUT(4)\n"
        "INPUT(5)\n"
        "OUTPUT(12)\n"
        "OUTPUT(13)\n"
        "6 = AND(0, 1)\n"
        "7 = AND(2, 6)\n"
        "8 = AND(3, 6)\n"
        "9 = OR(4, 5)\n"
        "10 = NOR(8, 9)\n"
        "11 = NAND(7, 8)\n"
        "12 = AND(10, 11)\n"
        "13 = MUX(8, 14, 15)\n"
        "14 = CONST(0)\n"
        "15 = CONST(1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder         = parser.getEncoder();

    auto [circuit, _] = Composition<DAG, cirbo::minimization::DeMorgan<cirbo::DAG> >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 21);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({1, 2}));
    REQUIRE(circuit->getGateType(4) == GateType::AND);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({0, 3}));
    REQUIRE(circuit->getGateType(5) == GateType::INPUT);
    REQUIRE(circuit->getGateType(6) == GateType::INPUT);
    REQUIRE(circuit->getGateType(7) == GateType::AND);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({14, 18}));
    REQUIRE(circuit->getGateType(8) == GateType::INPUT);
    REQUIRE(circuit->getGateType(9) == GateType::OR);
    REQUIRE(circuit->getGateOperands(9) == GateIdContainer({14, 15}));
    REQUIRE(circuit->getGateType(10) == GateType::AND);
    REQUIRE(circuit->getGateOperands(10) == GateIdContainer({7, 9}));
    REQUIRE(circuit->getGateType(11) == GateType::CONST_FALSE);
    REQUIRE(circuit->getGateOperands(11) == GateIdContainer({}));
    REQUIRE(circuit->getGateType(12) == GateType::CONST_TRUE);
    REQUIRE(circuit->getGateOperands(12) == GateIdContainer({}));
    REQUIRE(circuit->getGateType(13) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(13) == GateIdContainer({4, 11, 12}));
    REQUIRE(circuit->getGateType(14) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(14) == GateIdContainer({4}));
    REQUIRE(circuit->getGateType(15) == GateType::OR);
    REQUIRE(circuit->getGateOperands(15) == GateIdContainer({16, 17}));
    REQUIRE(circuit->getGateType(16) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(16) == GateIdContainer({3}));
    REQUIRE(circuit->getGateType(17) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(17) == GateIdContainer({8}));
    REQUIRE(circuit->getGateType(18) == GateType::AND);
    REQUIRE(circuit->getGateOperands(18) == GateIdContainer({19, 20}));
    REQUIRE(circuit->getGateType(19) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(19) == GateIdContainer({5}));
    REQUIRE(circuit->getGateType(20) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(20) == GateIdContainer({6}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({10, 13}));
}
