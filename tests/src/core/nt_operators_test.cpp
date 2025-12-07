#include <catch2/catch_test_macros.hpp>

#include "core/operators.hpp"
#include "core/types.hpp"

TEST_CASE("OperatorsNTTest GetOperator", "[operators][nt]")
{
    using cirbo::GateId;
    using cirbo::GateType;
    namespace op = cirbo::op;

    REQUIRE(op::getOperatorNT<GateId>(GateType::NOT) == static_cast<op::OperatorNT<GateId>>(&op::NOT<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::AND) == static_cast<op::OperatorNT<GateId>>(&op::AND<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::NAND) == static_cast<op::OperatorNT<GateId>>(&op::NAND<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::OR) == static_cast<op::OperatorNT<GateId>>(&op::OR<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::NOR) == static_cast<op::OperatorNT<GateId>>(&op::NOR<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::XOR) == static_cast<op::OperatorNT<GateId>>(&op::XOR<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::NXOR) == static_cast<op::OperatorNT<GateId>>(&op::NXOR<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::IFF) == static_cast<op::OperatorNT<GateId>>(&op::IFF<GateId>));
    REQUIRE(op::getOperatorNT<GateId>(GateType::MUX) == static_cast<op::OperatorNT<GateId>>(&op::MUX<GateId>));
    REQUIRE(
        op::getOperatorNT<GateId>(GateType::CONST_FALSE) ==
        static_cast<op::OperatorNT<GateId>>(&op::CONST_FALSE<GateId>));
    REQUIRE(
        op::getOperatorNT<GateId>(GateType::CONST_TRUE) ==
        static_cast<op::OperatorNT<GateId>>(&op::CONST_TRUE<GateId>));
}
