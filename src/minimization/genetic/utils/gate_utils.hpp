#pragma once

#include <vector>

#include "core/structures/mutable_circuit.hpp"

namespace cirbo::minimization::genetic
{

/**
 * @brief Validates operand count for a given gate type.
 * @param type Type of the gate.
 * @param operands List of operands for the gate.
 * @return true if operand count is valid.
 */
inline bool isValidArity(GateType type, std::vector<GateId> const& operands)
{
    if (type == GateType::NOT)
    {
        return operands.size() == 1;
    }

    if (type == GateType::AND || type == GateType::OR || type == GateType::XOR || type == GateType::NAND ||
        type == GateType::NOR || type == GateType::NXOR)
    {
        return operands.size() == 2;
    }

    return true;
}

}  // namespace cirbo::minimization::genetic