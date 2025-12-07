#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "core/operators.hpp"
#include "core/types.hpp"
#include "logger.hpp"
#include "utils/cast.hpp"

namespace
{

void assertAssociativeBinaryOperatorTripleN(
    cirbo::GateType type,
    cirbo::GateState fst,
    cirbo::GateState snd,
    cirbo::GateState trd)
{
    cirbo::op::Operator oper2  = cirbo::op::getOperator(type);
    cirbo::op::OperatorN operN = cirbo::op::getOperatorN(type);

    [[maybe_unused]]
    cirbo::GateState la = oper2(oper2(fst, snd, cirbo::GateState::UNDEFINED), trd, cirbo::GateState::UNDEFINED);
    [[maybe_unused]]
    cirbo::GateState ra = operN(cirbo::GateStateContainer{fst, snd, trd});

    cirbo::log::getGlobalLogger().info("Answer is ", cirbo::utils::gateStateToString(ra));
    cirbo::log::getGlobalLogger().info("Expected ", cirbo::utils::gateStateToString(la));
    REQUIRE(la == ra);
}

using ArgContainer = std::vector<std::vector<cirbo::GateState>>;

ArgContainer getAllNplets(std::size_t n)
{
    static std::vector<cirbo::GateState> states{
        cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED};

    if (n == 1)
    {
        return {
            {cirbo::GateState::FALSE},
            {cirbo::GateState::TRUE},
            {cirbo::GateState::UNDEFINED},
        };
    }

    ArgContainer out;
    for (auto const& x : getAllNplets(n - 1))
    {
        for (auto const& s : states)
        {
            out.push_back(std::vector<cirbo::GateState>(x));
            out.back().push_back(s);
        }
    }
    return out;
}

void LOG_TRIPLET(std::string const& op_name, cirbo::GateStateContainer arg)
{
    using cirbo::utils::gateStateToString;
    cirbo::log::getGlobalLogger().info(
        "Testing ",
        op_name,
        " on: {",
        gateStateToString(arg.at(0)),
        ", ",
        gateStateToString(arg.at(1)),
        ", ",
        gateStateToString(arg.at(2)),
        ", ",
        "}.");
}

TEST_CASE("OperatorsN NOT", "[operatorsN]")
{
    REQUIRE(cirbo::op::NOT(cirbo::GateStateContainer{cirbo::GateState::FALSE}) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::NOT(cirbo::GateStateContainer{cirbo::GateState::TRUE}) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::NOT(cirbo::GateStateContainer{cirbo::GateState::UNDEFINED}) == cirbo::GateState::UNDEFINED);
}

TEST_CASE("OperatorsN IFF", "[operatorsN]")
{
    REQUIRE(cirbo::op::IFF(cirbo::GateStateContainer{cirbo::GateState::FALSE}) == cirbo::GateState::FALSE);
    REQUIRE(cirbo::op::IFF(cirbo::GateStateContainer{cirbo::GateState::TRUE}) == cirbo::GateState::TRUE);
    REQUIRE(cirbo::op::IFF(cirbo::GateStateContainer{cirbo::GateState::UNDEFINED}) == cirbo::GateState::UNDEFINED);
}

TEST_CASE("OperatorsN AND", "[operatorsN]")
{
    for (auto const& arg : getAllNplets(3))
    {
        LOG_TRIPLET("AND", arg);
        assertAssociativeBinaryOperatorTripleN(cirbo::GateType::AND, arg.at(0), arg.at(1), arg.at(2));
    }
}

TEST_CASE("OperatorsN OR", "[operatorsN]")
{
    for (auto const& arg : getAllNplets(3))
    {
        LOG_TRIPLET("OR", arg);
        assertAssociativeBinaryOperatorTripleN(cirbo::GateType::OR, arg.at(0), arg.at(1), arg.at(2));
    }
}

TEST_CASE("OperatorsN XOR", "[operatorsN]")
{
    for (auto const& arg : getAllNplets(3))
    {
        LOG_TRIPLET("XOR", arg);
        assertAssociativeBinaryOperatorTripleN(cirbo::GateType::XOR, arg.at(0), arg.at(1), arg.at(2));
    }
}

TEST_CASE("OperatorsN NAND", "[operatorsN]")
{
    for (auto const& arg : getAllNplets(3))
    {
        LOG_TRIPLET("NAND", arg);
        cirbo::log::getGlobalLogger().info(
            "Result is ", cirbo::utils::gateStateToString(cirbo::op::NAND({arg.at(0), arg.at(1), arg.at(2)})));
        cirbo::log::getGlobalLogger().info(
            "Expected ",
            cirbo::utils::gateStateToString(cirbo::op::NOT(cirbo::op::AND({arg.at(0), arg.at(1), arg.at(2)}))));
        REQUIRE(
            cirbo::op::NAND({arg.at(0), arg.at(1), arg.at(2)}) ==
            cirbo::op::NOT(cirbo::op::AND({arg.at(0), arg.at(1), arg.at(2)})));
    }
}

TEST_CASE("OperatorsN NOR", "[operatorsN]")
{
    for (auto const& arg : getAllNplets(3))
    {
        LOG_TRIPLET("NOR", arg);
        cirbo::log::getGlobalLogger().info(
            "Answer is ", cirbo::utils::gateStateToString(cirbo::op::OR({arg.at(0), arg.at(1), arg.at(2)})));
        cirbo::log::getGlobalLogger().info(
            "Expected ",
            cirbo::utils::gateStateToString(cirbo::op::NOT(cirbo::op::OR({arg.at(0), arg.at(1), arg.at(2)}))));
        REQUIRE(
            cirbo::op::NOR({arg.at(0), arg.at(1), arg.at(2)}) ==
            cirbo::op::NOT(cirbo::op::OR({arg.at(0), arg.at(1), arg.at(2)})));
    }
}

TEST_CASE("OperatorsN NXOR", "[operatorsN]")
{
    for (auto const& arg : getAllNplets(3))
    {
        LOG_TRIPLET("NXOR", arg);
        cirbo::log::getGlobalLogger().info(
            "Answer is ", cirbo::utils::gateStateToString(cirbo::op::NXOR({arg.at(0), arg.at(1), arg.at(2)})));
        cirbo::log::getGlobalLogger().info(
            "Expected ",
            cirbo::utils::gateStateToString(cirbo::op::NOT(cirbo::op::XOR({arg.at(0), arg.at(1), arg.at(2)}))));
        REQUIRE(
            cirbo::op::NXOR({arg.at(0), arg.at(1), arg.at(2)}) ==
            cirbo::op::NOT(cirbo::op::XOR({arg.at(0), arg.at(1), arg.at(2)})));
    }
}

TEST_CASE("OperatorsN MUX_F", "[operatorsN][mux]")
{
    for (auto first_op : {cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED})
    {
        for (auto second_op : {cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED})
        {
            REQUIRE(cirbo::op::MUX({cirbo::GateState::FALSE, first_op, second_op}) == first_op);
        }
    }
}

TEST_CASE("OperatorsN MUX_T", "[operatorsN][mux]")
{
    for (auto first_op : {cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED})
    {
        for (auto second_op : {cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED})
        {
            REQUIRE(cirbo::op::MUX({cirbo::GateState::TRUE, first_op, second_op}) == second_op);
        }
    }
}

TEST_CASE("OperatorsN MUX_U", "[operatorsN][mux]")
{
    for (auto first_op : {cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED})
    {
        for (auto second_op : {cirbo::GateState::FALSE, cirbo::GateState::TRUE, cirbo::GateState::UNDEFINED})
        {
            REQUIRE(cirbo::op::MUX({cirbo::GateState::UNDEFINED, first_op, second_op}) == cirbo::GateState::UNDEFINED);
        }
    }
}

TEST_CASE("OperatorsN ConstFalse", "[operatorsN][const]")
{
    REQUIRE(cirbo::op::CONST_FALSE(cirbo::GateStateContainer{}) == cirbo::GateState::FALSE);
}

TEST_CASE("OperatorsN ConstTrue", "[operatorsN][const]")
{
    REQUIRE(cirbo::op::CONST_TRUE(cirbo::GateStateContainer{}) == cirbo::GateState::TRUE);
}

TEST_CASE("OperatorsN GetOperator", "[operatorsN][dispatch]")
{
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::NOT) == (cirbo::op::OperatorN)&cirbo::op::NOT);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::AND) == (cirbo::op::OperatorN)&cirbo::op::AND);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::NAND) == (cirbo::op::OperatorN)&cirbo::op::NAND);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::OR) == (cirbo::op::OperatorN)&cirbo::op::OR);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::NOR) == (cirbo::op::OperatorN)&cirbo::op::NOR);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::XOR) == (cirbo::op::OperatorN)&cirbo::op::XOR);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::NXOR) == (cirbo::op::OperatorN)&cirbo::op::NXOR);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::MUX) == (cirbo::op::OperatorN)&cirbo::op::MUX);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::IFF) == (cirbo::op::OperatorN)&cirbo::op::IFF);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::CONST_FALSE) == (cirbo::op::OperatorN)&cirbo::op::CONST_FALSE);
    REQUIRE(cirbo::op::getOperatorN(cirbo::GateType::CONST_TRUE) == (cirbo::op::OperatorN)&cirbo::op::CONST_TRUE);
}

}  // anonymous namespace
