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

TEST_CASE("ConstantGateReducer SimpleTest", "[constant_gate_reducer]")
{
    auto csat_instance = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::CONST_TRUE, {}},
            {cirbo::GateType::AND, {0, 3}},
            {cirbo::GateType::OR, {1, 2, 4}}
        },
        {5}
    );

    NameEncoder encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("1");
    encoder.encodeGate("2");
    encoder.encodeGate("5");
    encoder.encodeGate("3");
    encoder.encodeGate("4");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::INPUT);
    REQUIRE(circuit->getGateType(3) == GateType::OR);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1, 2}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({3}));
}

TEST_CASE("ConstantGateReducer ChangeOutput", "[constant_gate_reducer]")
{
    auto csat_instance = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::CONST_TRUE, {}},
            {cirbo::GateType::AND, {0, 1}}
        },
        {2}
    );

    NameEncoder encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("2");
    encoder.encodeGate("1");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 1);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getOutputGates() == GateIdContainer({0}));
}

TEST_CASE("ConstantGateReducer KnownAnswer", "[constant_gate_reducer]")
{
    auto csat_instance = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::CONST_TRUE, {}},
            {cirbo::GateType::OR, {0, 1}}
        },
        {2}
    );

    NameEncoder encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("2");
    encoder.encodeGate("1");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));
    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("ConstantGateReducer NoChanges", "[constant_gate_reducer]")
{
    auto csat_instance = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::OR, {0, 1}}
        },
        {2}
    );

    NameEncoder encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("1");
    encoder.encodeGate("2");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 3);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getGateType(1) == GateType::INPUT);
    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit->getOutputGates() == GateIdContainer({2}));
}

TEST_CASE("ConstantGateReducer UnusedGates", "[constant_gate_reducer]")
{
    auto csat_instance = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::CONST_TRUE, {}},
            {cirbo::GateType::OR, {0, 1}},
            {cirbo::GateType::CONST_TRUE, {}},
            {cirbo::GateType::AND, {0, 3}}
        },
        {4}
    );

    NameEncoder encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("1");
    encoder.encodeGate("2");
    encoder.encodeGate("3");
    encoder.encodeGate("4");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 1);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);
    REQUIRE(circuit->getOutputGates() == GateIdContainer({0}));
}

TEST_CASE("ConstantGateReducer SeveralOutputs", "[constant_gate_reducer]")
{
    std::string const dag =
        "INPUT(0)\n"
        "OUTPUT(2)\n"
        "OUTPUT(4)\n"
        "1 = CONST(1)\n"
        "2 = MUX(1, 0, 1)\n"
        "3 = NOT(1)\n"
        "4 = AND(1, 3)\n";

    std::istringstream stream(dag);
    cirbo::io::parsers::BenchToCircuit<cirbo::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<cirbo::DAG> csat_instance = parser.instantiate();
    cirbo::utils::NameEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        cirbo::minimization::ConstantGateReducer<cirbo::DAG>
    >().apply(*csat_instance, encoder);

    REQUIRE(circuit->getNumberOfGates() == 4);
    REQUIRE(circuit->getGateType(0) == GateType::INPUT);

    REQUIRE(circuit->getGateType(1) == GateType::NOT);
    REQUIRE(circuit->getGateOperands(1) == GateIdContainer({0}));

    REQUIRE(circuit->getGateType(2) == GateType::OR);
    REQUIRE(circuit->getGateOperands(2) == GateIdContainer({0, 1}));

    REQUIRE(circuit->getGateType(3) == GateType::AND);
    REQUIRE(circuit->getGateOperands(3) == GateIdContainer({0, 1}));

    REQUIRE(circuit->getOutputGates() == GateIdContainer({2, 3}));
}

TEST_CASE("ConstantGateReducer SaveCONST", "[constant_gate_reducer]")
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
