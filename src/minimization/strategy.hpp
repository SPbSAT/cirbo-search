#ifndef CIRBO_SEARCH_MINIMIZATION_STRATEGY_HPP
#define CIRBO_SEARCH_MINIMIZATION_STRATEGY_HPP

#include "core/structures/dag.hpp"
#include "core/structures/icircuit.hpp"
#include "minimization/composition.hpp"
#include "minimization/low_effort/connect_symmetrical_gates.hpp"
#include "minimization/low_effort/constant_gate_reducer.hpp"
#include "minimization/low_effort/de_morgan.hpp"
#include "minimization/low_effort/disconnect_symmetrical_gates.hpp"
#include "minimization/low_effort/duplicate_gates_cleaner.hpp"
#include "minimization/low_effort/duplicate_operands_cleaner.hpp"
#include "minimization/low_effort/merge_not_with_others.hpp"
#include "minimization/low_effort/reduce_not_composition.hpp"
#include "minimization/low_effort/redundant_gates_cleaner.hpp"
#include "minimization/low_effort/split_not_from_others.hpp"
#include "minimization/transformer_base.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that cleans circuit from all
 * gates that are not reachable from outputs.
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using RedundantGatesCleaner = Composition<CircuitT, RedundantGatesCleaner_<DAG> >;

/**
 * Transformer, that cleans circuit from duplicate gates.
 * Duplicates are gates with the same operands and operator
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using DuplicateGatesCleaner =
    Composition<CircuitT, RedundantGatesCleaner_<CircuitT>, DuplicateGatesCleaner_<CircuitT> >;

/**
 * Transformer, that cleans the circuit from unnecessary gates NOT.
 * For example: NOT(NOT(x)) => x
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using ReduceNotComposition = Composition<CircuitT, ReduceNotComposition_<DAG>, RedundantGatesCleaner_<DAG> >;

/**
 * Transformer, that cleans the circuit from constant gates ( like AND(x, NOT(x)) = false )
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(1)
 * INPUT(1)             |       INPUT(2)
 * INPUT(2)             |       5 = OR(1, 2)
 * 3 = NOT(0)           |       OUTPUT(5)
 * 4 = AND(0, 3)        |
 * 5 = OR(1, 2, 4)      |
 * OUTPUT(5)            |
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using ConstantGateReducer = Composition<
    CircuitT,
    ConstantGateReducer_<DAG>,
    ReduceNotComposition_<DAG>,
    RedundantGatesCleaner_<DAG>,
    DuplicateGatesCleaner_<DAG> >;

/**
 * Transformer, that cleans the circuit from gates with the same operands. For example:
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(0)
 * INPUT(1)             |       INPUT(1)
 * INPUT(2)             |       INPUT(2)
 * 3 = NOT(0)           |       3 = NOT(0)
 * 4 = AND(3, 3)        |       5 = AND(0, 3)
 * 5 = AND(0, 4)        |       6 = OR(1, 2, 5)
 * 6 = OR(1, 2, 5)      |       OUTPUT(6)
 * OUTPUT(6)            |
 *
 * And after cleans the circuit from constant gates ( like AND(x, NOT(x)) = false )
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(1)
 * INPUT(1)             |       INPUT(2)
 * INPUT(2)             |       6 = OR(1, 2)
 * 3 = NOT(0)           |       OUTPUT(6)
 * 5 = AND(0, 3)        |
 * 6 = OR(1, 2, 5)      |
 * OUTPUT(6)            |
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using DuplicateOperandsCleaner = Composition<
    CircuitT,
    RedundantGatesCleaner_<DAG>,
    DuplicateOperandsCleaner_<DAG>,
    RedundantGatesCleaner_<DAG, true>,  // true == save at least one input
    ConstantGateReducer_<DAG>,
    ReduceNotComposition_<DAG>,
    RedundantGatesCleaner_<DAG>,
    DuplicateGatesCleaner_<DAG> >;

/**
 * Transformer, that join NOT with other operators. For example:
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(0)
 * INPUT(1)             |       INPUT(1)
 * 2 = AND(0, 1)        |       2 = NAND(0, 1)
 * 3 = NOT(3)           |       OUTPUT(2)
 * OUTPUT(3)            |
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using MergeNotWithOthers = Composition<CircuitT, MergeNotWithOthers_<DAG>, RedundantGatesCleaner_<DAG> >;

/**
 * Transformer, that remove nesting of symmetrical pates (AND, OR and XOR) if they are specified in the params.
 * For example: AND(AND(0, 1), 2) => AND(0, 1, 2)
 *
 * @tparam CircuitT
 * @param EnableAND
 * @param EnableOR
 * @param EnableXOR
 */
template<
    class CircuitT,
    bool EnableAND = false,
    bool EnableOR  = false,
    bool EnableXOR = false,
    typename       = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using ConnectSymmetricalGates = Composition<
    CircuitT,
    RedundantGatesCleaner_<DAG>,
    ConnectSymmetricalGates_<DAG, EnableAND, EnableOR, EnableXOR>,
    RedundantGatesCleaner_<DAG> >;

/**
 * Transformer, that separates symmetrical gates (AND and OR) into gates of the specified arity using nesting.
 * For example: arity = 2; AND(0, 1, 2) => AND(AND(0, 1), 2)
 *
 * @tparam CircuitT
 * @param arity
 */
template<
    class CircuitT,
    int arity      = 2,
    bool EnableAND = false,
    bool EnableOR  = false,
    bool EnableXOR = false,
    typename       = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using DisconnectSymmetricalGates = Composition<
    CircuitT,
    RedundantGatesCleaner_<DAG>,
    DisconnectSymmetricalGates_<DAG, arity, EnableAND, EnableOR, EnableXOR> >;

/**
 * Transformer, that moves NOT closer to INPUT using de Morgan's low: NOT(AND(1, 2)) = OR(NOT(1), NOT(2));
 *                                                                    NOT(OR(1, 2)) = AND(NOT(1), NOT(2))
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using DeMorgan = Composition<
    CircuitT,
    RedundantGatesCleaner_<DAG>,
    DuplicateGatesCleaner_<DAG>,
    MergeNotWithOthers_<DAG>,
    RedundantGatesCleaner_<DAG>,
    DeMorgan_<DAG>,
    ReduceNotComposition_<DAG>,
    RedundantGatesCleaner_<DAG> >;

/**
 * Transformer, that split NOT with other operators. For example:
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(0)
 * INPUT(1)             |       INPUT(1)
 * 2 = NAND(0, 1)       |       2 = AND(0, 1)
 * OUTPUT(2)            |       3 = NOT(2)
 *                      |       OUTPUT(3)
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
using SplitNotFromOthers = Composition<CircuitT, SplitNotFromOthers_<DAG> >;

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_STRATEGY_HPP