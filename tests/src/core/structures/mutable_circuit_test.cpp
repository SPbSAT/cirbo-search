#include "core/structures/mutable_circuit.hpp"

#include <catch2/catch_test_macros.hpp>

#include "core/structures/gate_info.hpp"
#include "core/types.hpp"

using namespace cirbo;

TEST_CASE("MutableCircuit SimpleConstruction", "[mutable_circuit]")
{
    MutableCircuit circuit(
        {
            {GateType::INPUT, {}    }, // 0
            {GateType::INPUT, {}    }, // 1
            {GateType::AND,   {0, 1}}  // 2
    },
        {0, 1},
        {2});

    REQUIRE(circuit.getActualNumberOfGates() == 3);
    REQUIRE(circuit.getInputGates() == GateIdContainer({0, 1}));
    REQUIRE(circuit.getOutputGates() == GateIdContainer({2}));

    REQUIRE(circuit.getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(circuit.getGateUsers(1) == GateIdContainer({2}));
    REQUIRE(circuit.getGateUsers(2).empty());
}

TEST_CASE("MutableCircuit AddGate", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});         // 0
    circuit.addGate(GateType::INPUT, {});         // 1
    circuit.addGate(GateType::OR, {0, 1}, true);  // 2

    REQUIRE(circuit.getActualNumberOfGates() == 3);
    REQUIRE(circuit.getInputGates() == GateIdContainer({0, 1}));
    REQUIRE(circuit.getOutputGates() == GateIdContainer({2}));

    REQUIRE(circuit.getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(circuit.getGateUsers(1) == GateIdContainer({2}));
}

TEST_CASE("MutableCircuit ConnectGates", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0
    circuit.addGate(GateType::INPUT, {});  // 1
    circuit.addGate(GateType::AND, {});    // 2

    circuit.connectGates(0, 2);
    circuit.connectGates(1, 2);

    REQUIRE(circuit.getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(circuit.getGateUsers(1) == GateIdContainer({2}));
}

TEST_CASE("MutableCircuit ChangeGateType", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});          // 0
    circuit.addGate(GateType::NOT, {0});           // 1
    circuit.addGate(GateType::INPUT, {});          // 2
    circuit.addGate(GateType::AND, {1, 2}, true);  // 3
    circuit.changeGateType(1, GateType::CONST_TRUE);

    REQUIRE(circuit.getGateType(1) == GateType::CONST_TRUE);
}

TEST_CASE("MutableCircuit RemoveInputGate", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0
    circuit.addGate(GateType::INPUT, {});  // 1

    REQUIRE(circuit.getInputGates() == GateIdContainer({0, 1}));

    circuit.removeGate(1);

    REQUIRE(circuit.getActualNumberOfGates() == 1);
    REQUIRE(circuit.getInputGates() == GateIdContainer({0}));
}

TEST_CASE("MutableCircuit RemoveGateWithUsersThrows", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::INPUT, {});    // 1
    circuit.addGate(GateType::AND, {0, 1});  // 2

    REQUIRE_THROWS_AS(circuit.removeGate(0), std::logic_error);
}

TEST_CASE("MutableCircuit RemoveOutputGateThrows", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});         // 0
    circuit.addGate(GateType::INPUT, {});         // 1
    circuit.addGate(GateType::OR, {0, 1}, true);  // 2

    REQUIRE_THROWS_AS(circuit.removeGate(2), std::logic_error);
}

TEST_CASE("MutableCircuit InvalidOperandThrows", "[mutable_circuit]")
{
    MutableCircuit circuit;

    REQUIRE_THROWS_AS(circuit.addGate(GateType::AND, {42}), std::logic_error);
}

TEST_CASE("MutableCircuit DuplicateInputsInConstructorThrows", "[mutable_circuit]")
{
    REQUIRE_THROWS_AS(
        MutableCircuit(
            {
                {GateType::INPUT, {}},
                {GateType::INPUT, {}}
    },
            {0, 0},
            {}),
        std::logic_error);
}

TEST_CASE("MutableCircuit RemoveGateCorrectly", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::INPUT, {});    // 1
    circuit.addGate(GateType::AND, {0, 1});  // 2

    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(circuit.getGateUsers(1) == GateIdContainer({2}));
    REQUIRE(circuit.getGateOperands(2) == GateIdContainer({0, 1}));

    circuit.removeGate(2);

    REQUIRE(circuit.getActualNumberOfGates() == 2);

    REQUIRE(circuit.getGateUsers(0).empty());
    REQUIRE(circuit.getGateUsers(1).empty());

    REQUIRE_THROWS_AS(circuit.getGateType(2), std::out_of_range);
}

TEST_CASE("MutableCircuit IsOutputGateThrows", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0

    REQUIRE_THROWS_AS(circuit.isOutputGate(42), std::out_of_range);
}

TEST_CASE("MutableCircuit GetNumberOfGatesWithoutInputs", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::INPUT, {});    // 1
    circuit.addGate(GateType::AND, {0, 1});  // 2
    circuit.addGate(GateType::OR, {0, 1});   // 3

    REQUIRE(circuit.getActualNumberOfGates() == 4);
    REQUIRE(circuit.getNumberOfGatesWithoutInputs() == 2);
}

TEST_CASE("MutableCircuit ConnectGatesDuplicates", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0
    circuit.addGate(GateType::AND, {});    // 1

    circuit.connectGates(0, 1);
    circuit.connectGates(0, 1);

    REQUIRE(circuit.getGateOperands(1) == GateIdContainer({0, 0}));
    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({1, 1}));
}

TEST_CASE("MutableCircuit AddGateWithUsers", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});       // 0
    circuit.addGate(GateType::CONST_TRUE, {});  // 1
    circuit.addGate(GateType::NOT, GateIdContainer{0}, GateIdContainer{1},
                    false);  // 2

    REQUIRE(circuit.getGateOperands(2) == GateIdContainer({0}));
    REQUIRE(circuit.getGateUsers(2) == GateIdContainer({1}));
    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(circuit.getGateOperands(1) == GateIdContainer({2}));
}

TEST_CASE("MutableCircuit RemoveFromInputsThrows", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0
    circuit.addGate(GateType::AND, {});    // 1

    REQUIRE_THROWS_AS(circuit.removeFromInputs(1), std::out_of_range);

    REQUIRE_THROWS_AS(circuit.removeFromInputs(42), std::out_of_range);
}

TEST_CASE("MutableCircuit RemoveFromInputs", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0
    circuit.addGate(GateType::INPUT, {});  // 1

    REQUIRE(circuit.getInputGates() == GateIdContainer({0, 1}));

    circuit.removeFromInputs(0);

    REQUIRE(circuit.getInputGates() == GateIdContainer({1}));
}

TEST_CASE("MutableCircuit IsGateHasUsers", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::INPUT, {});    // 1
    circuit.addGate(GateType::XOR, {0, 1});  // 2

    REQUIRE(circuit.isGateHasUsers(0));
    REQUIRE(circuit.isGateHasUsers(1));
    REQUIRE_FALSE(circuit.isGateHasUsers(2));
}

TEST_CASE("MutableCircuit MultipleOutputs", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});          // 0
    circuit.addGate(GateType::INPUT, {});          // 1
    circuit.addGate(GateType::OR, {0, 1}, true);   // 2
    circuit.addGate(GateType::AND, {0, 1}, true);  // 3

    REQUIRE(circuit.getOutputGates() == GateIdContainer({2, 3}));
    REQUIRE(circuit.isOutputGate(2));
    REQUIRE(circuit.isOutputGate(3));
    REQUIRE_FALSE(circuit.isOutputGate(0));
}

TEST_CASE("MutableCircuit DuplicateOperandsAllowed", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::AND, {0, 0});  // 1

    REQUIRE(circuit.getGateOperands(1) == GateIdContainer({0, 0}));
    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({1, 1}));
}

TEST_CASE("MutableCircuit MoveConstructorBasic", "[mutable_circuit][move]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});          // 0
    circuit.addGate(GateType::INPUT, {});          // 1
    circuit.addGate(GateType::AND, {0, 1}, true);  // 2

    REQUIRE(circuit.getActualNumberOfGates() == 3);

    MutableCircuit moved(std::move(circuit));

    REQUIRE(moved.getActualNumberOfGates() == 3);
    REQUIRE(moved.getInputGates() == GateIdContainer({0, 1}));
    REQUIRE(moved.getOutputGates() == GateIdContainer({2}));
    REQUIRE(moved.getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(moved.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(moved.getGateUsers(1) == GateIdContainer({2}));

    REQUIRE(circuit.getActualNumberOfGates() == 0);
    REQUIRE(circuit.getInputGates().empty());
    REQUIRE(circuit.getOutputGates().empty());
}

TEST_CASE("MutableCircuit MoveConstructorReuseMovedFrom", "[mutable_circuit][move]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});  // 0
    circuit.addGate(GateType::INPUT, {});  // 1

    MutableCircuit moved(std::move(circuit));

    circuit.addGate(GateType::INPUT, {});  // 0

    REQUIRE(circuit.getActualNumberOfGates() == 1);
    REQUIRE(circuit.getInputGates() == GateIdContainer({0}));
}

TEST_CASE("MutableCircuit MoveAssignment", "[mutable_circuit][move]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});         // 0
    circuit.addGate(GateType::INPUT, {});         // 1
    circuit.addGate(GateType::OR, {0, 1}, true);  // 2

    MutableCircuit moved;
    moved.addGate(GateType::INPUT, {});

    moved = std::move(circuit);

    REQUIRE(moved.getActualNumberOfGates() == 3);
    REQUIRE(moved.getInputGates() == GateIdContainer({0, 1}));
    REQUIRE(moved.getOutputGates() == GateIdContainer({2}));

    REQUIRE(moved.getGateOperands(2) == GateIdContainer({0, 1}));
}

TEST_CASE("MutableCircuit CopyConstructor", "[mutable_circuit][copy]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});          // 0
    circuit.addGate(GateType::INPUT, {});          // 1
    circuit.addGate(GateType::AND, {0, 1}, true);  // 2

    MutableCircuit copy(circuit);

    REQUIRE(copy.getActualNumberOfGates() == 3);
    REQUIRE(copy.getInputGates() == GateIdContainer({0, 1}));
    REQUIRE(copy.getOutputGates() == GateIdContainer({2}));

    REQUIRE(copy.getGateOperands(2) == GateIdContainer({0, 1}));
    REQUIRE(copy.getGateUsers(0) == GateIdContainer({2}));
    REQUIRE(copy.getGateUsers(1) == GateIdContainer({2}));

    REQUIRE(circuit.getActualNumberOfGates() == 3);
}

TEST_CASE("MutableCircuit CopyAssignment", "[mutable_circuit][copy]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});         // 0
    circuit.addGate(GateType::OR, {0, 0}, true);  // 1

    MutableCircuit copy;
    copy.addGate(GateType::INPUT, {});

    copy = circuit;

    REQUIRE(copy.getActualNumberOfGates() == 2);
    REQUIRE(copy.getInputGates() == GateIdContainer({0}));
    REQUIRE(copy.getOutputGates() == GateIdContainer({1}));

    REQUIRE(copy.getGateOperands(1) == GateIdContainer({0, 0}));
}

TEST_CASE("MutableCircuit NextGateIdAfterRemoval", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::INPUT, {});    // 1
    circuit.addGate(GateType::AND, {0, 1});  // 2

    circuit.removeGate(2);

    circuit.addGate(GateType::OR, {0, 1});  // 3

    REQUIRE(circuit.getActualNumberOfGates() == 3);
    REQUIRE(circuit.getGateOperands(3) == GateIdContainer({0, 1}));

    REQUIRE_THROWS_AS(circuit.getGateType(2), std::out_of_range);
}

TEST_CASE("MutableCircuit RemoveGateWithDuplicateOperands", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::AND, {0, 0});  // 1

    REQUIRE(circuit.getGateUsers(0) == GateIdContainer({1, 1}));

    circuit.removeGate(1);

    REQUIRE(circuit.getGateUsers(0).empty());
    REQUIRE(circuit.getActualNumberOfGates() == 1);
}

TEST_CASE("MutableCircuit GettersThrowOnInvalidId", "[mutable_circuit][exceptions]")
{
    MutableCircuit circuit;
    circuit.addGate(GateType::INPUT, {});  // 0

    REQUIRE_THROWS_AS(circuit.getGateType(1), std::out_of_range);
    REQUIRE_THROWS_AS(circuit.getGateOperands(1), std::out_of_range);
    REQUIRE_THROWS_AS(circuit.getGateUsers(1), std::out_of_range);
}

TEST_CASE("MutableCircuit ConnectGatesThrowsOnInvalidId", "[mutable_circuit][exceptions]")
{
    MutableCircuit circuit;
    circuit.addGate(GateType::INPUT, {});  // 0

    REQUIRE_THROWS_AS(circuit.connectGates(0, 1), std::out_of_range);

    REQUIRE_THROWS_AS(circuit.connectGates(1, 0), std::out_of_range);
}

TEST_CASE("MutableCircuit ChangeGateTypeThrowsOnInvalidId", "[mutable_circuit][exceptions]")
{
    MutableCircuit circuit;

    REQUIRE_THROWS_AS(circuit.changeGateType(0, GateType::AND), std::out_of_range);
}

TEST_CASE("MutableCircuit IsGateHasUsersThrowsOnInvalidId", "[mutable_circuit][exceptions]")
{
    MutableCircuit circuit;

    REQUIRE_THROWS_AS(circuit.isGateHasUsers(0), std::out_of_range);
}

TEST_CASE("MutableCircuit RemoveGateThrowsOnInvalidId", "[mutable_circuit][exceptions]")
{
    MutableCircuit circuit;

    REQUIRE_THROWS_AS(circuit.removeGate(42), std::out_of_range);
}

TEST_CASE("MutableCircuit GetActualNumberOfGatesWithoutNot", "[mutable_circuit]")
{
    MutableCircuit circuit;

    circuit.addGate(GateType::INPUT, {});    // 0
    circuit.addGate(GateType::INPUT, {});    // 1
    circuit.addGate(GateType::NOT, {0});     // 2
    circuit.addGate(GateType::AND, {1, 0});  // 3

    REQUIRE(circuit.getActualNumberOfGates() == 4);
    REQUIRE(circuit.getActualNumberOfGatesWithoutNot() == 3);

    circuit.changeGateType(2, GateType::CONST_TRUE);
    REQUIRE(circuit.getActualNumberOfGatesWithoutNot() == 4);

    circuit.changeGateType(2, GateType::NOT);
    REQUIRE(circuit.getActualNumberOfGatesWithoutNot() == 3);

    circuit.removeGate(2);
    REQUIRE(circuit.getActualNumberOfGatesWithoutNot() == 3);

    circuit.addGate(GateType::NOT, {0});
    REQUIRE(circuit.getActualNumberOfGatesWithoutNot() == 3);
}