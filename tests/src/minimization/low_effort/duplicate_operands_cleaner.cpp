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
using namespace cirbo;
using namespace cirbo::minimization;

TEST_CASE("DuplicateOperandsCleaner KnownAnswer1", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "1 = NOT(0)\n"
        "2 = AND(1, 0)\n"
        "3 = AND(2, 1)\n"
        "4 = AND(3, 2)\n"
        "OUTPUT(4)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("DuplicateOperandsCleaner Bamboo", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "3 = OR(0, 0)\n"
        "4 = OR(3, 3)\n"
        "5 = AND(4, 0)\n"
        "6 = AND(5, 1)\n"
        "OUTPUT(6)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("DuplicateOperandsCleaner CreateNOT", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "2 = NAND(0, 0)\n"
        "3 = AND(2, 2)\n"
        "4 = AND(3, 1)\n"
        "OUTPUT(4)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({1}));
    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3}));
}

TEST_CASE("DuplicateOperandsCleaner MaxReductions", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "INPUT(3)\n"
        "4 = NAND(0, 0)\n"
        "5 = AND(4, 4)\n"
        "6 = AND(5, 1)\n"
        "7 = NAND(6, 2)\n"
        "8 = NOR(7, 7)\n"
        "9 = AND(8, 3)\n"
        "OUTPUT(9)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 9);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::INPUT);
    REQUIRE(circuit->getGateType(4) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(4) == GateIdContainer({3}));
    REQUIRE(circuit->getGateType(5) == GateType::AND);
    REQUIRE(circuit->getGateOperands(5) == GateIdContainer({2, 4}));
    REQUIRE(circuit->getGateType(6) == GateType::NAND);
    REQUIRE(circuit->getGateOperands(6) == GateIdContainer({1, 5}));
    REQUIRE(circuit->getGateType(7) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(7) == GateIdContainer({6}));
    REQUIRE(circuit->getGateType(8) == GateType::AND);
    REQUIRE(circuit->getGateOperands(8) == GateIdContainer({0, 7}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({8}));
}

TEST_CASE("DuplicateOperandsCleaner ChangeOutput1", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "2 = NOT(0)\n"
        "3 = AND(2, 0)\n"
        "4 = NOT(3)\n"
        "5 = XOR(4, 1)\n"
        "OUTPUT(5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, encoder_] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 2);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(encoder_->decodeGate(0) == "1");
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({1}));
}

TEST_CASE("DuplicateOperandsCleaner ChangeOutputsType", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "INPUT(1)\n"
        "INPUT(2)\n"
        "3 = NOT(0)\n"
        "4 = AND(3, 0)\n"
        "5 = NOT(4)\n"
        "6 = XOR(5, 1, 2)\n"
        "OUTPUT(6)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, encoder_] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(encoder_->decodeGate(0) == "1");
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(encoder_->decodeGate(1) == "2");
    REQUIRE(circuit->getGateType(2) == GateType::NXOR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("DuplicateOperandsCleaner KnownAnswer2", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "1 = XOR(0, 0)\n"
        "OUTPUT(1)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(2) == GateType::AND);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("DuplicateOperandsCleaner ChangeOutput2", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "1 = NAND(0, 0)\n"
        "2 = NOT(1)\n"
        "3 = XOR(0, 1, 2)\n"
        "OUTPUT(3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 2);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({1}));
}

TEST_CASE("DuplicateOperandsCleaner InputIsOutput", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "1 = NAND(0, 0)\n"
        "2 = NOT(1)\n"
        "3 = NAND(0, 0)\n"
        "4 = NOT(3)\n"
        "5 = XOR(0, 1, 2, 3, 4)\n"
        "OUTPUT(5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 1);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getOutputGates() == GateIdContainer({0}));
}

TEST_CASE("DuplicateOperandsCleaner SeveralOutputs", "[duplicate_operands]")
{
    std::string const dag =
        "INPUT(0)\n"
        "1 = NAND(0, 0)\n"
        "2 = NOT(0)\n"
        "3 = NAND(2, 2)\n"
        "4 = XOR(0, 1, 2, 3)\n"
        "5 = AND(0, 0)\n"
        "OUTPUT(4)\n"
        "OUTPUT(5)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::DuplicateOperandsCleaner<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(2) == GateType::NXOR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2, 0}));
}

TEST_CASE("DuplicateOperandsCleaner SaveCONST", "[duplicate_operands][const]")
{
    std::string const dag =
        "INPUT(0)\n"
        "OUTPUT(2)\n"
        "OUTPUT(3)\n"
        "1 = CONST(1)\n"
        "2 = MUX(1, 4, 0)\n"
        "3 = MUX(0, 1, 2)\n"
        "4 = CONST(1)";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    // Note: this test uses ConstantGateReducer, as in the original code.
    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::ConstantGateReducer<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::CONST_TRUE);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({}));
    REQUIRE(circuit->getGateType(2) == GateType::MUX);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1, 0}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({0, 2}));
}
