#include "core/types.hpp"
#include "core/structures/dag.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("DAG SimpleConstruction", "[dag]")
{
    auto dag = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::AND, {0, 1}}
        },
        {2}
    );

    REQUIRE(dag.getNumberOfGates() == 3);
    REQUIRE(dag.getGateUsers(0) == cirbo::GateIdContainer({2}));
    REQUIRE(dag.getGateUsers(1) == cirbo::GateIdContainer({2}));
}

TEST_CASE("DAG Calculation", "[dag]")
{
    auto dag = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::OR, {0, 1}}
        },
        {2}
    );

    REQUIRE(dag.getGateUsers(0) == cirbo::GateIdContainer({2}));
    REQUIRE(dag.getGateUsers(1) == cirbo::GateIdContainer({2}));

    auto asmt = cirbo::VectorAssignment<>{};
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::UNDEFINED);

    asmt.assign(0, cirbo::GateState::TRUE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(1, cirbo::GateState::TRUE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(0, cirbo::GateState::FALSE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(1, cirbo::GateState::FALSE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::FALSE);
}

TEST_CASE("DAG CalculationMultipleOutputs", "[dag]")
{
    auto dag = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::INPUT, {}},
            {cirbo::GateType::OR,  {0, 1}},
            {cirbo::GateType::AND, {0, 1}},
            {cirbo::GateType::XOR, {0, 1}}
        },
        {2, 3, 4}
    );

    REQUIRE(dag.getGateUsers(0) == cirbo::GateIdContainer({2, 3, 4}));
    REQUIRE(dag.getGateUsers(1) == cirbo::GateIdContainer({2, 3, 4}));
    REQUIRE(dag.getGateUsers(2) == cirbo::GateIdContainer({}));
    REQUIRE(dag.getGateUsers(3) == cirbo::GateIdContainer({}));
    REQUIRE(dag.getGateUsers(4) == cirbo::GateIdContainer({}));

    auto asmt = cirbo::VectorAssignment<>{};
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::UNDEFINED);

    asmt.assign(0, cirbo::GateState::TRUE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(1, cirbo::GateState::TRUE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(0, cirbo::GateState::TRUE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(1, cirbo::GateState::FALSE);
    auto result_assignment = dag.evaluateCircuit(asmt);
    REQUIRE(result_assignment->getGateState(2) == cirbo::GateState::TRUE);
    REQUIRE(result_assignment->getGateState(3) == cirbo::GateState::FALSE);
    REQUIRE(result_assignment->getGateState(4) == cirbo::GateState::TRUE);
}

TEST_CASE("DAG CalculationComplex", "[dag]")
{
    auto dag = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},      // 0
            {cirbo::GateType::INPUT, {}},      // 1
            {cirbo::GateType::INPUT, {}},      // 2
            {cirbo::GateType::NOT, {1}},       // 3
            {cirbo::GateType::OR,  {0, 3}},    // 4
            {cirbo::GateType::AND, {3, 2}},    // 5
            {cirbo::GateType::AND, {4, 5}}     // 6
        },
        {6}
    );

    REQUIRE(dag.getGateUsers(0) == cirbo::GateIdContainer({4}));
    REQUIRE(dag.getGateUsers(1) == cirbo::GateIdContainer({3}));
    REQUIRE(dag.getGateUsers(2) == cirbo::GateIdContainer({5}));
    REQUIRE(dag.getGateUsers(3) == cirbo::GateIdContainer({4, 5}));
    REQUIRE(dag.getGateUsers(4) == cirbo::GateIdContainer({6}));
    REQUIRE(dag.getGateUsers(5) == cirbo::GateIdContainer({6}));
    REQUIRE(dag.getGateUsers(6) == cirbo::GateIdContainer({}));

    auto asmt = cirbo::VectorAssignment<>{};
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(6) == cirbo::GateState::UNDEFINED);

    asmt.assign(0, cirbo::GateState::TRUE);
    asmt.assign(1, cirbo::GateState::TRUE);
    asmt.assign(2, cirbo::GateState::TRUE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(6) == cirbo::GateState::FALSE);

    asmt.assign(1, cirbo::GateState::FALSE);
    REQUIRE(dag.evaluateCircuit(asmt)->getGateState(6) == cirbo::GateState::TRUE);
}

TEST_CASE("DAG CalculationWithConstGates", "[dag]")
{
    auto dag = cirbo::DAG(
        {
            {cirbo::GateType::CONST_FALSE, {}},   // 0
            {cirbo::GateType::CONST_TRUE,  {}},   // 1
            {cirbo::GateType::INPUT,       {}},   // 2
            {cirbo::GateType::NOT,  {1}},         // 3
            {cirbo::GateType::OR,   {0, 3}},      // 4
            {cirbo::GateType::OR,   {3, 2}},      // 5
            {cirbo::GateType::OR,   {4, 5}}       // 6
        },
        {6}
    );

    REQUIRE(dag.getGateUsers(0) == cirbo::GateIdContainer({4}));
    REQUIRE(dag.getGateUsers(1) == cirbo::GateIdContainer({3}));
    REQUIRE(dag.getGateUsers(2) == cirbo::GateIdContainer({5}));
    REQUIRE(dag.getGateUsers(3) == cirbo::GateIdContainer({4, 5}));
    REQUIRE(dag.getGateUsers(4) == cirbo::GateIdContainer({6}));
    REQUIRE(dag.getGateUsers(5) == cirbo::GateIdContainer({6}));
    REQUIRE(dag.getGateUsers(6) == cirbo::GateIdContainer({}));

    auto asmt   = cirbo::VectorAssignment<>{};
    auto asmt_1 = dag.evaluateCircuit(asmt);

    REQUIRE(asmt_1->getGateState(0) == cirbo::GateState::FALSE);
    REQUIRE(asmt_1->getGateState(1) == cirbo::GateState::TRUE);
    REQUIRE(asmt_1->getGateState(2) == cirbo::GateState::UNDEFINED);
    REQUIRE(asmt_1->getGateState(3) == cirbo::GateState::FALSE);
    REQUIRE(asmt_1->getGateState(4) == cirbo::GateState::FALSE);
    REQUIRE(asmt_1->getGateState(5) == cirbo::GateState::UNDEFINED);
    REQUIRE(asmt_1->getGateState(6) == cirbo::GateState::UNDEFINED);

    asmt.assign(2, cirbo::GateState::TRUE);
    auto asmt_2 = dag.evaluateCircuit(asmt);

    REQUIRE(asmt_2->getGateState(0) == cirbo::GateState::FALSE);
    REQUIRE(asmt_2->getGateState(1) == cirbo::GateState::TRUE);
    REQUIRE(asmt_2->getGateState(2) == cirbo::GateState::TRUE);
    REQUIRE(asmt_2->getGateState(3) == cirbo::GateState::FALSE);
    REQUIRE(asmt_2->getGateState(4) == cirbo::GateState::FALSE);
    REQUIRE(asmt_2->getGateState(5) == cirbo::GateState::TRUE);
    REQUIRE(asmt_2->getGateState(6) == cirbo::GateState::TRUE);

    asmt.assign(2, cirbo::GateState::FALSE);
    auto asmt_3 = dag.evaluateCircuit(asmt);

    REQUIRE(asmt_3->getGateState(0) == cirbo::GateState::FALSE);
    REQUIRE(asmt_3->getGateState(1) == cirbo::GateState::TRUE);
    REQUIRE(asmt_3->getGateState(2) == cirbo::GateState::FALSE);
    REQUIRE(asmt_3->getGateState(3) == cirbo::GateState::FALSE);
    REQUIRE(asmt_3->getGateState(4) == cirbo::GateState::FALSE);
    REQUIRE(asmt_3->getGateState(5) == cirbo::GateState::FALSE);
    REQUIRE(asmt_3->getGateState(6) == cirbo::GateState::FALSE);
}

TEST_CASE("DAG CalculationManyOperands", "[dag]")
{
    auto dag = cirbo::DAG(
        {
            {cirbo::GateType::INPUT, {}},           // 0
            {cirbo::GateType::INPUT, {}},           // 1
            {cirbo::GateType::INPUT, {}},           // 2
            {cirbo::GateType::AND, {0, 1, 2}},      // 3
            {cirbo::GateType::OR,  {0, 1, 2}}       // 4
        },
        {}
    );

    REQUIRE(dag.getGateUsers(0) == cirbo::GateIdContainer({3, 4}));
    REQUIRE(dag.getGateUsers(1) == cirbo::GateIdContainer({3, 4}));
    REQUIRE(dag.getGateUsers(2) == cirbo::GateIdContainer({3, 4}));
    REQUIRE(dag.getGateUsers(3) == cirbo::GateIdContainer({}));
    REQUIRE(dag.getGateUsers(4) == cirbo::GateIdContainer({}));
}
