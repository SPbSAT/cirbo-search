#pragma once

#include <algorithm>
#include <unordered_map>

#include "core/structures/dag.hpp"
#include "core/structures/gate_info.hpp"
#include "core/structures/icircuit.hpp"

namespace cirbo
{
class MutableCircuit : public ICircuit
{
private:

    class MutableNode
    {
    private:
        GateId id     = 0;
        GateType type = GateType::UNDEFINED;
        GateIdContainer operands;
        GateIdContainer users;

    public:
        MutableNode()                                     = default;
        ~MutableNode()                                    = default;
        MutableNode(MutableNode& m_node)                  = default;
        MutableNode(MutableNode const& m_node)            = default;
        MutableNode& operator=(MutableNode const& m_node) = default;

        MutableNode(MutableNode&& m_node)
            : id(std::exchange(m_node.id, 0))
            , type(std::exchange(m_node.type, GateType::UNDEFINED))
            , operands(std::exchange(m_node.operands, {}))
            , users(std::exchange(m_node.users, {}))
        {
        }

        MutableNode(
            GateId const g_id,
            GateType const g_type,
            GateIdContainer const& g_operands,
            GateIdContainer const& g_users)
            : id(g_id)
            , type(g_type)
            , operands(g_operands)
            , users(g_users)
        {
        }

        MutableNode(GateId const g_id, GateType const g_type, GateIdContainer&& g_operands, GateIdContainer&& g_users)
            : id(g_id)
            , type(g_type)
            , operands{std::move(g_operands)}
            , users{std::move(g_users)}
        {
        }

        MutableNode(GateId const g_id, GateType const g_type, GateIdContainer const& g_operands)
            : id(g_id)
            , type(g_type)
            , operands(g_operands)
        {
        }

        MutableNode(GateId const g_id, GateType const g_type, GateIdContainer&& g_operands)
            : id(g_id)
            , type(g_type)
            , operands{std::move(g_operands)}
        {
        }

        GateId getId() const { return id; }

        GateType getType() const { return type; }

        GateIdContainer const& getOperands() const { return operands; }

        GateIdContainer const& getUsers() const { return users; }

        /**
         * @brief Adds a user identifier to the node's user list
         * @param gateId user identifier to add
         * @post gateId is appended to the users container
         */
        void addUser(GateId gateId) { users.push_back(gateId); }

        /**
         * @brief Adds a container of users identifiers to the node's user list
         * @param usersIdContainer container of users identifiers to add
         * @post All elements of usersIdContainer are appended to the users container
         */
        void addUsersContainer(GateIdContainer const& usersIdContainer)
        {
            users.insert(users.end(), usersIdContainer.begin(), usersIdContainer.end());
        }

        /**
         * @brief Adds a container of users identifiers to the node's user list using move semantics
         * @param usersIdContainer container of users identifiers to move
         * @post All elements of usersIdContainer are appended to the users container
         */
        void addUsersContainer(GateIdContainer&& usersIdContainer)
        {
            users.insert(
                users.end(), std::move_iterator(usersIdContainer.begin()), std::move_iterator(usersIdContainer.end()));
        }

        /**
         * @brief Adds an operand identifier to the node's operand list
         * @param gateId operand identifier to add
         * @post gateId is appended to the operands container
         */
        void addOperand(GateId gateId) { operands.push_back(gateId); }

        /**
         * @brief Adds a container of operands identifiers to the node's operand list
         * @param operandsIdContainer container of operands identifiers to add
         * @post All elements of operandsIdContainer are appended to the operands container
         */
        void addOperandsContainer(GateIdContainer const& operandsIdContainer)
        {
            operands.insert(operands.end(), operandsIdContainer.begin(), operandsIdContainer.end());
        }

        /**
         * @brief Adds a container of operands identifiers to the node's operand list using move semantics
         * @param operandsIdContainer container of operands identifiers to move
         * @post All elements of operandsIdContainer are appended to the operands container
         */
        void addOperandsContainer(GateIdContainer&& operandsIdContainer)
        {
            operands.insert(
                operands.end(),
                std::move_iterator(operandsIdContainer.begin()),
                std::move_iterator(operandsIdContainer.end()));
        }

        void setType(GateType newType) { type = newType; }

        /**
         * @brief Replaces the current node operands with new ones
         * @param operandsIdContainer new container of operands identifiers
         */
        void setOperands(GateIdContainer const& operandsIdContainer) { operands = operandsIdContainer; }

        /**
         * @brief Replaces the current node operands with new ones using move semantics
         * @param operandsIdContainer new container of operands identifiers to move
         */
        void setOperands(GateIdContainer&& operandsIdContainer) { operands = std::move(operandsIdContainer); }

        /**
         * @brief Replaces the current node users with new ones
         * @param usersIdContainer new container of users identifiers
         */
        void setUsers(GateIdContainer const& usersIdContainer) { users = usersIdContainer; }

        /**
         * @brief Replaces the current node users with new ones using move semantics
         * @param usersIdContainer new container of users identifiers
         */
        void setUsers(GateIdContainer&& usersIdContainer) { users = std::move(usersIdContainer); }

        /**
         * @brief Removes an identifier from the node's user list
         * @param removing_id identifier to remove
         * @note If there are multiple occurrences of the same identifier in the container,
         *       only the first one encountered (from the beginning) will be removed
         */
        void removeIdFromUsers(GateId const removing_id)
        {
            auto it = std::ranges::find(users, removing_id);
            if (it != users.end())
            {
                users.erase(it);
            }
        }

        /**
         * @brief Removes an identifier from the node's operand list
         * @param removing_id identifier to remove
         * @note If there are multiple occurrences of the same identifier in the container,
         *       only the first one encountered (from the beginning) will be removed
         */
        void removeIdFromOperands(GateId const removing_id)
        {
            auto it = std::ranges::find(operands, removing_id);
            if (it != operands.end())
            {
                operands.erase(it);
            }
        }
    };

    std::unordered_map<GateId, std::pair<MutableNode, bool>> gates;
    GateIdContainer input_gates;
    GateIdContainer output_gates;
    GateId next_gate_id;

    void validateGateIds(GateIdContainer const& ids) const
    {
        for (GateId id : ids)
        {
            if (!gates.contains(id))
            {
                throw std::out_of_range("Gate ID " + std::to_string(id) + " does not exist");
            }
        }
    }

public:
    MutableCircuit()                                           = default;
    ~MutableCircuit()                                          = default;
    MutableCircuit(MutableCircuit& m_circuit)                  = default;
    MutableCircuit(MutableCircuit const& m_circuit)            = default;
    MutableCircuit& operator=(MutableCircuit const& m_circuit) = default;

    MutableCircuit(MutableCircuit&& m_circuit)
        : gates(std::exchange(m_circuit.gates, {}))
        , input_gates(std::exchange(m_circuit.input_gates, {}))
        , output_gates(std::exchange(m_circuit.output_gates, {}))
        , next_gate_id(std::exchange(m_circuit.next_gate_id, 0))
    {
    }

    MutableCircuit(
        GateInfoContainer const& gates_info,
        GateIdContainer const& inputs,
        GateIdContainer const& outputs)
        : input_gates(inputs)
        , output_gates(outputs)
        , next_gate_id(gates_info.size())
    {
        for (GateId id = 0; id < gates_info.size(); ++id)
        {
            GateInfo const& info = gates_info[id];
            gates[id] = std::make_pair(
                MutableNode(id, info.getType(), info.getOperands()),
                false
            );
        }

        for (auto& [id, pair] : gates)
        {
            MutableNode& node = pair.first;
            for (GateId operand_id : node.getOperands())
            {
                if (gates.contains(operand_id))
                {
                    gates.at(operand_id).first.addUser(id);
                }
            }
        }

        validateGateIds(input_gates);
        validateGateIds(output_gates);
    }

    MutableCircuit(GateInfoContainer&& gates_info,
                GateIdContainer&& inputs,
                GateIdContainer&& outputs)
        : input_gates(std::move(inputs))
        , output_gates(std::move(outputs))
        , next_gate_id(gates_info.size())
    {
        for (GateId id = 0; id < gates_info.size(); ++id)
        {
            GateInfo& info = gates_info[id];
            gates[id] = std::make_pair(
                MutableNode(id, info.getType(), info.moveOperands()),
                false
            );
        }

        for (auto& [id, pair] : gates)
        {
            MutableNode& node = pair.first;
            for (GateId operand_id : node.getOperands())
            {
                if (gates.contains(operand_id))
                {
                    gates.at(operand_id).first.addUser(id);
                }
            }
        }

        validateGateIds(input_gates);
        validateGateIds(output_gates);
    }

    /**
     * @brief Returns the type of a logic node by its identifier
     * @param gateId logic node identifier
     * @return GateType - logic node type
     * @throws std::out_of_range if gateId does not exist
     */
    [[nodiscard]]
    GateType getGateType(GateId gateId) const override
    {
        return gates.at(gateId).first.getType();
    }

    /**
     * @brief Returns the operand container of a logic node
     * @param gateId logic node identifier
     * @return GateIdContainer const& - container of operands identifiers
     * @throws std::out_of_range if gateId does not exist
     */
    [[nodiscard]]
    GateIdContainer const& getGateOperands(GateId gateId) const override
    {
        return gates.at(gateId).first.getOperands();
    }

    /**
     * @brief Returns the user container of a logic node
     * @param gateId logic node identifier
     * @return GateIdContainer const& - container of users identifiers
     * @throws std::out_of_range if gateId does not exist
     */
    [[nodiscard]]
    GateIdContainer const& getGateUsers(GateId gateId) const override
    {
        return gates.at(gateId).first.getUsers();
    }

    /**
     * @brief Returns the total number of logic nodes in the circuit
     * @return GateId - number of logic nodes
     * @post Result includes input, output, and internal nodes
     */
    [[nodiscard]]
    GateId getNumberOfGates() const override
    {
        return gates.size();
    }

    /**
     * @brief Returns the number of logic nodes excluding inputs
     * @return GateId - number of logic nodes without input gates
     * @post Result includes output and internal nodes
     */
    [[nodiscard]]
    GateId getNumberOfGatesWithoutInputs() const override
    {
        return gates.size() - input_gates.size();
    }

    /**
     * @brief Returns the container of output nodes identifiers for the circuit
     * @return GateIdContainer const& - container of output nodes identifiers
     */
    [[nodiscard]]
    GateIdContainer const& getOutputGates() const override
    {
        return output_gates;
    }

    /**
     * @brief Returns the container of input nodes identifiers for the circuit
     * @return GateIdContainer const& - container of input nodes identifiers
     */
    [[nodiscard]]
    GateIdContainer const& getInputGates() const override
    {
        return input_gates;
    }

    /**
     * @brief Checks if an element is an output node of the circuit
     * @param gateId logic node identifier
     * @return true if the node is an output, false otherwise
     */
    [[nodiscard]]
    bool isOutputGate(GateId gateId) const override
    {
        auto it = std::ranges::find(output_gates, gateId);
        if (it != output_gates.end())
        {
            return true;
        }
        return false;
    }

    /**
     * @brief Adds a new logic node to the circuit
     * @param g_type type of the new logic node
     * @param operands container of operand identifiers for the new node
     * @param is_output flag indicating whether the node is an output
     * @throws std::out_of_range if any operand identifier does not exist in the circuit
     * @post A new node with a unique identifier is created
     * @post For each operand in operands, a connection to the new node is established
     * @post If g_type == GateType::INPUT, the node is added to input_gates
     * @post If is_output == true, the node is added to output_gates
     */
    void addGate(GateType g_type, GateIdContainer const& operands = {}, bool is_output = false)
    {
        validateGateIds(operands);

        GateId new_id = next_gate_id++;
        gates[new_id] = std::make_pair(MutableNode(new_id, g_type, {}), false);
        for (GateId operand_id : operands)
        {
            if (gates.contains(operand_id))
            {
                connectGates(operand_id, new_id);
            }
        }
        if (g_type == GateType::INPUT)
        {
            input_gates.push_back(new_id);
        }
        if (is_output)
        {
            output_gates.push_back(new_id);
        }
    }

    /**
     * @brief Adds a new logic node to the circuit using move semantics for parameters
     * @param g_type type of the new logic node
     * @param operands container of operand identifiers for the new node
     * @param users container of user identifiers for the new node
     * @param is_output flag indicating whether the node is an output
     * @throws std::out_of_range if any operand or user identifier does not exist in the circuit
     * @post A new node with a unique identifier is created
     * @post For each operand in operands, a connection to the new node is established
     * @post For each user in users, a connection from the new node is established
     * @post If g_type == GateType::INPUT, the node is added to input_gates
     * @post If is_output == true, the node is added to output_gates
     */
    void addGate(GateType g_type, GateIdContainer&& operands = {}, GateIdContainer&& users = {}, bool is_output = false)
    {
        validateGateIds(operands);
        validateGateIds(users);

        GateId new_id = next_gate_id++;
        gates[new_id] = std::make_pair(MutableNode(new_id, g_type, {}), false);
        for (GateId operand_id : operands)
        {
            if (gates.contains(operand_id))
            {
                connectGates(operand_id, new_id);
            }
        }
        for (GateId user_id : users)
        {
            if (gates.contains(user_id))
            {
                connectGates(new_id, user_id);
            }
        }
        if (g_type == GateType::INPUT)
        {
            input_gates.push_back(new_id);
        }
        if (is_output)
        {
            output_gates.push_back(new_id);
        }
    }

    /**
     * @brief Establishes a connection between two nodes
     * @param new_operand_gate identifier of the node that becomes an operand
     * @param new_user_gate identifier of the node that becomes a user
     * @post new_operand_gate is added to the operands of new_user_gate
     * @post new_user_gate is added to the users of new_operand_gate
     * @throws std::out_of_range if any of the identifiers does not exist
     */
    void connectGates(GateId new_operand_gate, GateId new_user_gate)
    {
        gates.at(new_user_gate).first.addOperand(new_operand_gate);
        gates.at(new_operand_gate).first.addUser(new_user_gate);
    }

    /**
     * @brief Changes the type of an existing logic node
     * @param gate_id identifier of the node to modify
     * @param new_type new logic node type
     * @throws std::out_of_range if gate_id does not exist
     */
    void changeGateType(GateId gate_id, GateType new_type) { gates.at(gate_id).first.setType(new_type); }

    /**
     * @brief Clears the operand list of a logic node and removes corresponding connections
     * @param gate_id node identifier
     * @post All nodes are removed from the operand list of gate_id
     * @post For each former operand, gate_id is removed from the user list
     * @throws std::out_of_range if gate_id does not exist
     */
    void clearGateOperands(GateId gate_id)
    {
        GateIdContainer operands = gates.at(gate_id).first.getOperands();
        for (GateId operand_id : operands) { gates.at(operand_id).first.removeIdFromUsers(gate_id); }
        gates.at(gate_id).first.setOperands({});
    }

    /**
     * @brief Removes a node from the circuit's input gates list
     * @param removing_gate_id identifier of the node to remove
     * @post Only the first occurrence of removing_gate_id is removed from input_gates
     * @post The deleted node flag is set to true for the removed node
     * @note If removing_gate_id appears multiple times in input_gates,
     *       only the first occurrence (from the beginning) is removed
     * @note The node remains in the circuit's gates container but is marked as deleted
     */
    void removeFromInputs(GateId removing_gate_id)
    {
        auto it = std::ranges::find(input_gates, removing_gate_id);
        if (it != input_gates.end())
        {
            input_gates.erase(it);
            gates[removing_gate_id].second = true;
        }
    }

    /**
     * @brief Checks if a logic node exists in the circuit
     * @param gate_id identifier of the node to check
     * @return true if the element exists, false otherwise
     */
    bool isGateExists(GateId gate_id) const { return gates.find(gate_id) != gates.end(); }

    /**
     * @brief Checks if a logic node has users
     * @param gate_id identifier of the node to check
     * @return true if the element has at least one user, false otherwise
     * @throws std::out_of_range if gate_id does not exist
     */
    bool isGateHasUsers(GateId gate_id) const { return !gates.at(gate_id).first.getUsers().empty(); }

    /**
     * @brief Removes a logic node from the circuit
     * @param removing_gate_id identifier of the node to remove
     * @throws std::out_of_range if removing_gate_id does not exist
     * @throws std::logic_error if removing_gate_id has users
     * @post If the element was an input, it is removed from input_gates
     * @post If the element was not an input, its operands are cleared
     * @post The element remains in the gates container with a deletion flag
     */
    void removeGate(GateId removing_gate_id)
    {
        if (isGateHasUsers(removing_gate_id))
        {
            throw std::logic_error("Cannot remove gate " + std::to_string(removing_gate_id) + " that has users");
        }

        gates.at(removing_gate_id).second = true;

        if (gates.at(removing_gate_id).first.getType() == GateType::INPUT)
        {
            auto it = std::ranges::find(input_gates, removing_gate_id);
            if (it != input_gates.end())
            {
                input_gates.erase(it);
            }
        }
        else
        {
            clearGateOperands(removing_gate_id);
        }
    }
};
}  // namespace cirbo