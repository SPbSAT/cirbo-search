#pragma once

#include <random>
#include <stack>
#include <unordered_set>
#include <vector>

#include "core/structures/mutable_circuit.hpp"
#include "utils/random.hpp"

namespace cirbo::minimization::genetic
{

/**
 * @brief Checks whether a node is reachable from another node.
 * @param circuit Circuit instance.
 * @param ancestor Starting gate in the traversal.
 * @param candidate Gate to check for reachability.
 * @return true if candidate is reachable from ancestor, false otherwise.
 *
 * Used to prevent creation of cycles in the circuit graph.
 */
template<class CircuitT>
bool isDescendant(CircuitT& circuit, GateId ancestor, GateId candidate)
{
    if (ancestor == candidate)
    {
        return true;
    }

    std::stack<GateId> stack;
    std::unordered_set<GateId> visited;

    stack.push(ancestor);

    while (!stack.empty())
    {
        GateId current = stack.top();
        stack.pop();

        if (current == candidate)
        {
            return true;
        }

        if (!visited.insert(current).second)
        {
            continue;
        }

        if (!circuit.isGateExists(current))
        {
            continue;
        }

        for (GateId user_id : circuit.getGateUsers(current))
        {
            if (!visited.count(user_id))
            {
                stack.push(user_id);
            }
        }
    }

    return false;
}

/**
 * @brief Selects a random non-input gate from the circuit.
 * @param circuit Circuit instance.
 * @param rng Random generator.
 * @return Identifier of a randomly selected gate.
 *
 * If no valid gate exists, returns 0.
 */
template<class CircuitT>
size_t randomGate(CircuitT& circuit, std::mt19937& rng)
{
    std::vector<GateId> valid;

    for (GateId i = 0; i < circuit.getNumberOfGates(); ++i)
    {
        if (circuit.isGateExists(i) && !circuit.isInputGate(i))
        {
            valid.push_back(i);
        }
    }

    if (valid.empty())
    {
        return 0;
    }

    return valid[utils::randomIndex(valid.size(), rng)];
}

/**
 * @brief Selects a random operand candidate for a gate.
 * @param circuit Circuit instance.
 * @param gate_id Identifier of the gate whose operands are being chosen.
 * @param rng Random generator.
 * @return Identifier of a randomly selected valid operand gate.
 */
template<class CircuitT>
GateId randomOperand(CircuitT& circuit, size_t gate_id, std::mt19937& rng)
{
    std::vector<GateId> candidates;

    for (GateId i = 0; i < gate_id; ++i)
    {
        if (circuit.isGateExists(i))
        {
            candidates.push_back(i);
        }
    }

    if (candidates.empty())
    {
        return 0;
    }

    return candidates[utils::randomIndex(candidates.size(), rng)];
}

}  // namespace cirbo::minimization::genetic