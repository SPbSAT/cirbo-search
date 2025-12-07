#ifndef CIRBO_SEARCH_CORE_TYPES_HPP
#define CIRBO_SEARCH_CORE_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Main header that contains some logic constants and general types.
 */
namespace cirbo
{

// One place to choose what to use: `vector<char>` or `vector<bool>`.
// (`vector<bool>` is a specific template specialization).
using BoolVector = std::vector<bool>;

/** Minimum arity of a logical function. **/
enum class MinArity : uint8_t
{
    NULLARY = 0,
    UNARY   = 1,
    BINARY  = 2,
    TERNARY = 3
};

/** Possible states of gates. **/
enum class GateState : uint8_t
{
    // if changed, then cirbo::op must be changed as well
    FALSE     = 0,
    TRUE      = 1,
    UNDEFINED = 2
};

/** Number of possible Gate States **/
constexpr size_t GateStateNumber = 3;

/** Possible types of gates. **/
enum class GateType : uint8_t
{
    // if changed, then cirbo::op must be changed as well
    INPUT = 0,
    NOT   = 1,
    AND   = 2,
    NAND  = 3,
    OR    = 4,
    NOR   = 5,
    XOR   = 6,
    NXOR  = 7,
    IFF   = 8,
    MUX   = 9,
    // constant operator types
    CONST_FALSE = 10,
    CONST_TRUE  = 11,
    // special operator types
    BUFF      = UINT8_MAX - 1,
    UNDEFINED = UINT8_MAX
};
/** Number of supported Operator Gates **/
constexpr size_t SupportedOperatorNumber = 11;
/** Index of first Operator Gate in GateType enum **/
constexpr size_t FirstOperatorIdx = 1;

/**
 * @return index of gateType among all gate operator types.
 */
inline size_t getIndexByOperator(GateType gateType) { return static_cast<size_t>(gateType) - FirstOperatorIdx; }

/** At i'th position carries state of gate with Id = i. **/
using GateStateContainer = std::vector<GateState>;

/** Internal gate ids are numbers 0,1,2... **/
using GateId          = size_t;
using GateIdContainer = std::vector<GateId>;

// FIXME: maybe basis should guarantee strictness.
//
/** Type of circuit basis (set of allowed operators). **/
enum class Basis : uint8_t
{
    BENCH,
    AIG
};

}  // namespace cirbo

#endif  // CIRBO_SEARCH_CORE_TYPES_HPP