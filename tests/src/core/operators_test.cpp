#include "core/operators.hpp"

#include <catch2/catch_test_macros.hpp>

#include "core/types.hpp"

TEST_CASE("Operators NOT", "[operators]")
{
    REQUIRE(cirbo::op::NOT(cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NOT(cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NOT(cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
}

TEST_CASE("Operators AND", "[operators]")
{
    REQUIRE(cirbo::op::AND(cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::AND(cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::AND(cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::AND(cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::AND(cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::AND(cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::AND(cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::AND(cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::AND(cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
}

TEST_CASE("Operators OR", "[operators]")
{
    REQUIRE(cirbo::op::OR(cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::OR(cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::OR(cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::OR(cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::OR(cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::OR(cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::OR(cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::OR(cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::OR(cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
}

TEST_CASE("Operators XOR", "[operators]")
{
    REQUIRE(cirbo::op::XOR(cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::XOR(cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
}

TEST_CASE("Operators NAND", "[operators]")
{
    REQUIRE(cirbo::op::NAND(cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NAND(cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
}

TEST_CASE("Operators NOR", "[operators]")
{
    REQUIRE(cirbo::op::NOR(cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NOR(cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
}

TEST_CASE("Operators NXOR", "[operators]")
{
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NXOR(cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
}

TEST_CASE("Operators IFF", "[operators]")
{
    REQUIRE(cirbo::op::IFF(cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::IFF(cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::IFF(cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
}

TEST_CASE("Operators MUX_F", "[operators][mux]")
{
    cirbo::GateState logic = cirbo::GateState::FALSE;
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::TRUE);
    REQUIRE(
        cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
}

TEST_CASE("Operators MUX_T", "[operators][mux]")
{
    cirbo::GateState logic = cirbo::GateState::TRUE;
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(
        cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
}

TEST_CASE("Operators MUX_U", "[operators][mux]")
{
    cirbo::GateState logic = cirbo::GateState::UNDEFINED;
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::FALSE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::TRUE) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::FALSE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(cirbo::op::MUX(logic, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
    REQUIRE(
        cirbo::op::MUX(logic, cirbo::GateState::UNDEFINED, cirbo::GateState::UNDEFINED) == cirbo::GateState::UNDEFINED);
}

TEST_CASE("Operators CONST_FALSE", "[operators][const]")
{
    REQUIRE(cirbo::op::CONST_FALSE() == cirbo::GateState::FALSE);
}

TEST_CASE("Operators CONST_TRUE", "[operators][const]") { REQUIRE(cirbo::op::CONST_TRUE() == cirbo::GateState::TRUE); }

TEST_CASE("Operators getOperator", "[operators][dispatch]")
{
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::NOT) == (cirbo::op::Operator)&cirbo::op::NOT);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::AND) == (cirbo::op::Operator)&cirbo::op::AND);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::NAND) == (cirbo::op::Operator)&cirbo::op::NAND);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::OR) == (cirbo::op::Operator)&cirbo::op::OR);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::NOR) == (cirbo::op::Operator)&cirbo::op::NOR);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::XOR) == (cirbo::op::Operator)&cirbo::op::XOR);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::NXOR) == (cirbo::op::Operator)&cirbo::op::NXOR);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::IFF) == (cirbo::op::Operator)&cirbo::op::IFF);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::MUX) == (cirbo::op::Operator)&cirbo::op::MUX);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::CONST_FALSE) == (cirbo::op::Operator)&cirbo::op::CONST_FALSE);
    REQUIRE(cirbo::op::getOperator(cirbo::GateType::CONST_TRUE) == (cirbo::op::Operator)&cirbo::op::CONST_TRUE);
}
