#include "core/structures/dag.hpp"
#include "core/algo.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace cirbo;

TEST_CASE("TopSort SimpleCircuit", "[topsort]")
{
    auto dag = DAG(
        {
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::AND, {0, 1}}
        },
        {2}
    );

    GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    REQUIRE(gateSorting.at(0) == 2);
    REQUIRE(gateSorting.at(1) == 1);
    REQUIRE(gateSorting.at(2) == 0);
}

TEST_CASE("TopSort MediumCircuit", "[topsort]")
{
    auto dag = DAG(
        {
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::AND, {0, 1}},
            {GateType::AND, {1, 2}},
            {GateType::AND, {0, 1}},
            {GateType::OR, {3, 4, 5}}
        },
        {6}
    );

    GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    REQUIRE(gateSorting.at(0) == 6);
    REQUIRE(gateSorting.at(1) == 5);
    REQUIRE(gateSorting.at(2) == 4);
    REQUIRE(gateSorting.at(3) == 2);
    REQUIRE(gateSorting.at(4) == 3);
    REQUIRE(gateSorting.at(5) == 1);
    REQUIRE(gateSorting.at(6) == 0);
}

TEST_CASE("TopSort MultiOutputCircuit", "[topsort]")
{
    auto dag = DAG(
        {
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::AND, {0, 1}},
            {GateType::AND, {1, 2}},
            {GateType::AND, {0, 1}},
            {GateType::OR, {3, 5}},
            {GateType::AND, {4, 5}}
        },
        {6, 7}
    );

    GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    REQUIRE(gateSorting.at(0) == 7);
    REQUIRE(gateSorting.at(1) == 4);
    REQUIRE(gateSorting.at(2) == 2);
    REQUIRE(gateSorting.at(3) == 6);
    REQUIRE(gateSorting.at(4) == 5);
    REQUIRE(gateSorting.at(5) == 3);
    REQUIRE(gateSorting.at(6) == 1);
    REQUIRE(gateSorting.at(7) == 0);

    auto dag2 = DAG(
        {
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::AND, {0, 1}},
            {GateType::AND, {2, 1}},
        },
        {2, 3}
    );

    gateSorting = algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag2);
    REQUIRE(gateSorting.at(0) == 3);
    REQUIRE(gateSorting.at(1) == 2);
    REQUIRE(gateSorting.at(2) == 0);
    REQUIRE(gateSorting.at(3) == 1);
}

TEST_CASE("TopSort DisconnectedGraphCircuit", "[topsort]")
{
    auto dag = DAG(
        {
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::AND, {0, 1}},
            {GateType::INPUT, {}},
            {GateType::OR, {4}}
        },
        {3}
    );

    GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    REQUIRE(gateSorting.at(0) == 5);
    REQUIRE(gateSorting.at(1) == 4);
    REQUIRE(gateSorting.at(2) == 3);
    REQUIRE(gateSorting.at(3) == 1);
    REQUIRE(gateSorting.at(4) == 0);
    REQUIRE(gateSorting.at(5) == 2);
}

TEST_CASE("TopSort GatesWithoutUsers", "[topsort]")
{
    auto dag = DAG(
        {
            {GateType::AND, {1, 3}},
            {GateType::NOT, {3}},
            {GateType::NOT, {1}},
            {GateType::INPUT, {}}
        },
        {3}
    );

    auto gateSorting = algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag);
    REQUIRE(gateSorting.size() == 4);
}