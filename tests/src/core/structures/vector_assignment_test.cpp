#include "core/structures/vector_assignment.hpp"

#include <catch2/catch_test_macros.hpp>

#include "core/types.hpp"

TEST_CASE("VectorAssignment Set", "[vector_assignment]")
{
    cirbo::VectorAssignment<> assignment{};
    assignment.assign(1, cirbo::GateState::TRUE);
    assignment.assign(2, cirbo::GateState::FALSE);
    assignment.assign(3, cirbo::GateState::UNDEFINED);
    assignment.assign(10, cirbo::GateState::FALSE);

    REQUIRE(assignment.getGateState(1) == cirbo::GateState::TRUE);
    REQUIRE(assignment.getGateState(2) == cirbo::GateState::FALSE);
    REQUIRE(assignment.getGateState(3) == cirbo::GateState::UNDEFINED);
    REQUIRE(assignment.getGateState(10) == cirbo::GateState::FALSE);
}

TEST_CASE("VectorAssignment Basic", "[vector_assignment]")
{
    cirbo::VectorAssignment<> asmt{};
    asmt.assign(2, cirbo::GateState::TRUE);
    REQUIRE(asmt.getGateState(2) == cirbo::GateState::TRUE);

    asmt.assign(1, cirbo::GateState::FALSE);
    REQUIRE(asmt.getGateState(1) == cirbo::GateState::FALSE);

    asmt.assign(2, cirbo::GateState::FALSE);
    REQUIRE(asmt.getGateState(2) == cirbo::GateState::FALSE);
    REQUIRE(asmt.getGateState(3) == cirbo::GateState::UNDEFINED);
}

TEST_CASE("VectorAssignment Clear", "[vector_assignment]")
{
    cirbo::VectorAssignment<> assignment{};
    assignment.assign(1, cirbo::GateState::TRUE);
    assignment.assign(2, cirbo::GateState::FALSE);
    assignment.assign(3, cirbo::GateState::UNDEFINED);

    REQUIRE(assignment.getGateState(1) == cirbo::GateState::TRUE);
    REQUIRE(assignment.getGateState(2) == cirbo::GateState::FALSE);
    REQUIRE(assignment.getGateState(3) == cirbo::GateState::UNDEFINED);

    assignment.clear();

    REQUIRE(assignment.getGateState(1) == cirbo::GateState::UNDEFINED);
    REQUIRE(assignment.getGateState(2) == cirbo::GateState::UNDEFINED);
    REQUIRE(assignment.getGateState(3) == cirbo::GateState::UNDEFINED);
}
