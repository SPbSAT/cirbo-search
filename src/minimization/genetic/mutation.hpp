#pragma once

#include <algorithm>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "core/structures/mutable_circuit.hpp"
#include "minimization/genetic/utils/circuit_utils.hpp"
#include "minimization/genetic/utils/gate_utils.hpp"
#include "minimization/genetic/utils/log_utils.hpp"
#include "utils/random.hpp"

namespace cirbo::minimization::genetic
{
/**
 * @brief Reconnects one operand of a randomly selected gate.
 *
 * One operand of the gate is replaced with another valid gate
 * while preventing cycles in the circuit.
 */
template<class CircuitT>
void reconnectOperandMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 1)
    {
        return;
    }

    size_t gate = randomGate(*circuit, rng);

    if (circuit->isInputGate(gate) || circuit->isOutputGate(gate))
    {
        return;
    }

    auto operands = circuit->getGateOperands(gate);

    if (operands.size() < 2)
    {
        return;
    }

    size_t operand_index = utils::randomIndex(operands.size(), rng);
    GateId old_operand   = operands[operand_index];

    GateId new_operand = old_operand;

    int const max_attempts = 20;

    for (int attempt = 0; attempt < max_attempts && new_operand == old_operand; ++attempt)
    {
        GateId candidate = randomOperand(*circuit, gate, rng);

        if (!isDescendant(*circuit, gate, candidate))
        {
            new_operand = candidate;
        }
    }

    if (new_operand == old_operand)
    {
        return;
    }

    operands[operand_index] = new_operand;

    auto users = circuit->getGateUsers(gate);

    bool was_output = circuit->isOutputGate(gate);

    GateType type = circuit->getGateType(gate);

    if (!isValidArity(type, operands))
    {
        return;
    }

    circuit->addGate(type, operands, false);

    GateId new_gate_id = circuit->getNumberOfGates() - 1;

    for (GateId user_id : users)
    {
        if (!circuit->isGateExists(user_id))
        {
            continue;
        }

        if (isDescendant(*circuit, new_operand, user_id))
        {
            continue;
        }

        circuit->replaceOperand(user_id, gate, new_gate_id);
    }

    if (!circuit->isOutputGate(gate) && !circuit->isGateHasUsers(gate))
    {
        circuit->removeGate(gate);
    }

    appendHistory(
        history,
        "reconnectOperand: gate " + std::to_string(gate) + ", operand[" + std::to_string(operand_index)
            + "] " + std::to_string(old_operand) + " -> " + std::to_string(new_operand)
            + ", rebuilt as gate " + std::to_string(new_gate_id) + " (" + gateTypeToString(type) + ")");
}

/**
 * @brief Inserts a NOT gate before a randomly selected operand.
 *
 * Creates a new NOT gate that negates one operand of the selected gate
 * and reconnects the circuit to use the new node.
 */
template<class CircuitT>
void insertNotMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 1)
    {
        return;
    }

    size_t gate = randomGate(*circuit, rng);

    if (circuit->isInputGate(gate))
    {
        return;
    }

    auto operands = circuit->getGateOperands(gate);

    if (operands.empty())
    {
        return;
    }

    size_t operand_index = utils::randomIndex(operands.size(), rng);

    GateId target_operand = operands[operand_index];

    circuit->addGate(GateType::NOT, {target_operand}, false);

    GateId not_gate_id = circuit->getNumberOfGates() - 1;

    operands[operand_index] = not_gate_id;

    auto users = circuit->getGateUsers(gate);

    bool was_output = circuit->isOutputGate(gate);

    GateType type = circuit->getGateType(gate);

    if (!isValidArity(type, operands))
    {
        return;
    }

    circuit->addGate(type, operands, false);

    GateId new_gate_id = circuit->getNumberOfGates() - 1;

    if (was_output)
    {
        circuit->replaceOutput(gate, new_gate_id);
    }

    for (GateId user_id : users)
    {
        if (circuit->isGateExists(user_id) && user_id != new_gate_id)
        {
            if (!isDescendant(*circuit, not_gate_id, user_id))
            {
                circuit->replaceOperand(user_id, gate, new_gate_id);
            }
        }
    }

    if (!circuit->isOutputGate(gate) && !circuit->isGateHasUsers(gate))
    {
        circuit->removeGate(gate);
    }

    appendHistory(
        history,
        "insertNot: gate " + std::to_string(gate) + ", operand[" + std::to_string(operand_index)
            + "] " + std::to_string(target_operand) + " -> NOT gate " + std::to_string(not_gate_id)
            + ", rebuilt as gate " + std::to_string(new_gate_id) + " (" + gateTypeToString(type) + ")");
}

/**
 * @brief Adds a new randomly generated gate to the circuit.
 *
 * A new gate with randomly selected type and operands is created.
 * With certain probability it may replace an operand in an existing user gate.
 */
template<class CircuitT>
void addRandomGateMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 1)
    {
        return;
    }

    GateId existing_gate = randomGate(*circuit, rng);

    std::vector<GateType> possible_types = {GateType::AND, GateType::NOT};

    GateType new_type = possible_types[utils::randomIndex(possible_types.size(), rng)];

    std::vector<GateId> operands;

    if (new_type == GateType::NOT)
    {
        GateId operand = randomOperand(*circuit, existing_gate, rng);
        operands       = {operand};
    }
    else
    {
        GateId operand1 = randomOperand(*circuit, existing_gate, rng);
        GateId operand2 = randomOperand(*circuit, existing_gate, rng);

        int const max_attempts = 2;

        for (int attempt = 0; attempt < max_attempts && operand2 == operand1; ++attempt)
        {
            operand2 = randomOperand(*circuit, existing_gate, rng);
        }

        operands = {operand1, operand2};
    }

    if (!isValidArity(new_type, operands))
    {
        return;
    }

    circuit->addGate(new_type, operands, false);

    GateId new_gate_id = circuit->getNumberOfGates() - 1;

    bool should_connect = utils::randomDouble(rng) < 0.7;

    if (should_connect && circuit->getGateUsers(existing_gate).size() > 0)
    {
        auto users = circuit->getGateUsers(existing_gate);

        if (!users.empty())
        {
            GateId random_user = users[utils::randomIndex(users.size(), rng)];

            if (!circuit->isGateExists(random_user))
            {
                return;
            }

            if (random_user == new_gate_id)
            {
                return;
            }

            if (isDescendant(*circuit, existing_gate, random_user))
            {
                return;
            }

            circuit->replaceOperand(random_user, existing_gate, new_gate_id);

            appendHistory(
                history,
                "addRandomGate: added gate " + std::to_string(new_gate_id) + " (" + gateTypeToString(new_type)
                    + ") with operands " + gateIdContainerToString(operands)
                    + ", rewired user " + std::to_string(random_user) + " from gate "
                    + std::to_string(existing_gate));
            return;
        }
    }

    appendHistory(
        history,
        "addRandomGate: added gate " + std::to_string(new_gate_id) + " (" + gateTypeToString(new_type)
            + ") with operands " + gateIdContainerToString(operands) + ", no user rewired");
}

/**
 * @brief Duplicates an existing gate.
 *
 * Creates a new gate with the same type and operands
 * as a randomly selected gate in the circuit.
 */
template<class CircuitT>
void duplicateGateMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 1)
    {
        return;
    }

    size_t gate = randomGate(*circuit, rng);

    if (circuit->isInputGate(gate))
    {
        return;
    }

    auto operands = circuit->getGateOperands(gate);
    auto type     = circuit->getGateType(gate);

    for (auto op : operands)
    {
        if (!circuit->isGateExists(op))
        {
            return;
        }
    }

    circuit->addGate(type, operands, false);

    appendHistory(
        history,
        "duplicateGate: duplicated gate " + std::to_string(gate) + " as gate "
            + std::to_string(circuit->getNumberOfGates() - 1) + " (" + gateTypeToString(type) + ")");
}

/**
 * @brief Removes a randomly selected gate.
 *
 * The gate is removed only if it is not an input, not an output,
 * and has no users in the circuit.
 */
template<class CircuitT>
void removeRandomGateMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 1)
    {
        return;
    }

    size_t gate = randomGate(*circuit, rng);

    if (circuit->isInputGate(gate) || circuit->isOutputGate(gate))
    {
        return;
    }

    auto users = circuit->getGateUsers(gate);

    if (!users.empty())
    {
        return;
    }

    circuit->removeGate(gate);

    appendHistory(history, "removeRandomGate: removed gate " + std::to_string(gate));
}

/**
 * @brief Replaces one output gate with another gate.
 *
 * Randomly selects an existing output gate and replaces it
 * with another valid non-input gate in the circuit.
 */
template<class CircuitT>
void changeOutputGateMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 1)
    {
        return;
    }

    auto current_outputs = circuit->getOutputGates();

    size_t output_index = utils::randomIndex(current_outputs.size(), rng);

    GateId old_output = current_outputs[output_index];

    GateId new_output = old_output;

    int const max_attempts = 20;

    for (int attempt = 0; attempt < max_attempts && new_output == old_output; ++attempt)
    {
        GateId candidate = randomGate(*circuit, rng);

        if (!circuit->isOutputGate(candidate) && !circuit->isInputGate(candidate))
        {
            new_output = candidate;
        }
    }

    if (new_output == old_output)
    {
        return;
    }

    circuit->replaceOutput(old_output, new_output);

    appendHistory(
        history,
        "changeOutputGate: output " + std::to_string(old_output) + " -> " + std::to_string(new_output));
}

/**
 * @brief Replaces one subtree with another subtree.
 *
 * Selects a target gate and replaces all its usages with another
 * randomly selected compatible gate, effectively swapping subgraphs.
 */
template<class CircuitT>
void replaceSubtreeMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 3)
    {
        return;
    }

    GateId target_root = randomGate(*circuit, rng);
    GateId donor_root  = randomGate(*circuit, rng);

    if (target_root == donor_root)
    {
        return;
    }

    if (!circuit->isGateExists(target_root) || !circuit->isGateExists(donor_root))
    {
        return;
    }

    if (circuit->isInputGate(target_root))
    {
        return;
    }

    if (isDescendant(*circuit, target_root, donor_root))
    {
        return;
    }

    auto users = circuit->getGateUsers(target_root);
    size_t replaced_edges = 0;

    for (GateId user_id : users)
    {
        if (!circuit->isGateExists(user_id))
        {
            continue;
        }

        if (isDescendant(*circuit, donor_root, user_id))
        {
            continue;
        }

        circuit->replaceOperand(user_id, target_root, donor_root);
        ++replaced_edges;
    }

    if (!circuit->isOutputGate(target_root) && !circuit->isGateHasUsers(target_root))
    {
        circuit->removeGate(target_root);
    }

    appendHistory(
        history,
        "replaceSubtree: target " + std::to_string(target_root) + " <- donor " + std::to_string(donor_root)
            + ", replaced " + std::to_string(replaced_edges) + " user edge(s)");
}

/**
 * @brief Applies one randomly selected mutation.
 *
 * Randomly chooses one of the available mutation types
 * and applies it to the circuit.
 */
template<class CircuitT>
void applyRandomMutation(CircuitT* circuit, std::mt19937& rng, std::vector<std::string>* history = nullptr)
{
    if (circuit->getActualNumberOfGates() <= 3)
    {
        return;
    }

    using MutationFn = void (*)(CircuitT*, std::mt19937&, std::vector<std::string>*);

    static MutationFn mutations[] = {
        reconnectOperandMutation<CircuitT>,
        insertNotMutation<CircuitT>,
        addRandomGateMutation<CircuitT>,
        duplicateGateMutation<CircuitT>,
        removeRandomGateMutation<CircuitT>,
        changeOutputGateMutation<CircuitT>,
        replaceSubtreeMutation<CircuitT>};

    std::uniform_int_distribution<int> dist(0, static_cast<int>(std::size(mutations)) - 1);

    int idx = dist(rng);

    mutations[idx](circuit, rng, history);
}

}  // namespace cirbo::minimization::genetic
