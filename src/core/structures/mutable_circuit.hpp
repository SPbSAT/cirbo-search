#pragma once

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

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

        /**
         * @brief Returns the operand container of the node
         * @return GateIdContainer const& - container of operands identifiers
         * @note The container may contain duplicate identifiers when the same gate
         *       is used multiple times as an operand
         * @note The order of operands is preserved and may be significant for
         *       non-commutative operations
         */
        GateIdContainer const& getOperands() const { return operands; }

        /**
         * @brief Returns the user container of the node
         * @return GateIdContainer const& - container of users identifiers
         * @note The container may contain duplicate identifiers when this gate
         *       is used multiple times as an operand by another gate
         *       (e.g., if y = AND(x, x) exists, then x.users will contain y twice)
         */
        GateIdContainer const& getUsers() const { return users; }

        /**
         * @brief Adds a user identifier to the node's user list
         * @param gateId user identifier to add
         * @post gateId is appended to the users container
         * @note Multiple identical user identifiers are allowed to support
         *       gates with duplicate operands like AND(x, x)
         */
        void addUser(GateId gateId) { users.push_back(gateId); }

        /**
         * @brief Adds a container of users identifiers to the node's user list
         * @param usersIdContainer container of users identifiers to add
         * @post All elements of usersIdContainer are appended to the users container
         * @note Multiple identical users identifiers are allowed
         */
        void addUsersContainer(GateIdContainer const& usersIdContainer)
        {
            users.insert(users.end(), usersIdContainer.begin(), usersIdContainer.end());
        }

        /**
         * @brief Adds a container of users identifiers to the node's user list using move semantics
         * @param usersIdContainer container of users identifiers to move
         * @post All elements of usersIdContainer are appended to the users container
         * @note Multiple identical users identifiers are allowed
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
         * @note Multiple identical operand identifiers are allowed
         */
        void addOperand(GateId gateId) { operands.push_back(gateId); }

        /**
         * @brief Adds a container of operands identifiers to the node's operand list
         * @param operandsIdContainer container of operands identifiers to add
         * @post All elements of operandsIdContainer are appended to the operands container
         * @note Multiple identical operands identifiers are allowed
         */
        void addOperandsContainer(GateIdContainer const& operandsIdContainer)
        {
            operands.insert(operands.end(), operandsIdContainer.begin(), operandsIdContainer.end());
        }

        /**
         * @brief Adds a container of operands identifiers to the node's operand list using move semantics
         * @param operandsIdContainer container of operands identifiers to move
         * @post All elements of operandsIdContainer are appended to the operands container
         * @note Multiple identical operands identifiers are allowed
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
         * @note Multiple identical operands identifiers are allowed
         */
        void setOperands(GateIdContainer const& operandsIdContainer) { operands = operandsIdContainer; }

        /**
         * @brief Replaces the current node operands with new ones using move semantics
         * @param operandsIdContainer new container of operands identifiers to move
         * @note Multiple identical operand identifiers are allowed
         */
        void setOperands(GateIdContainer&& operandsIdContainer) { operands = std::move(operandsIdContainer); }

        /**
         * @brief Replaces the current node users with new ones
         * @param usersIdContainer new container of users identifiers
         * @note Multiple identical users identifiers are allowed
         */
        void setUsers(GateIdContainer const& usersIdContainer) { users = usersIdContainer; }

        /**
         * @brief Replaces the current node users with new ones using move semantics
         * @param usersIdContainer new container of users identifiers
         * @note Multiple identical users identifiers are allowed
         */
        void setUsers(GateIdContainer&& usersIdContainer) { users = std::move(usersIdContainer); }

        /**
         * @brief Removes an identifier from the node's user list
         * @param removing_id identifier to remove
         * @param remove_all if true, removes all occurrences; if false, removes only the first occurrence
         * @post If remove_all is true, all occurrences of removing_id are removed
         * @post If remove_all is false, only the first occurrence is removed
         * @note When dealing with duplicate user identifiers (e.g., from AND(x, x)),
         *       remove_all=false will only remove one connection, leaving the other intact
         */
        void removeIdFromUsers(GateId removing_id, bool remove_all = false)
        {
            if (remove_all)
            {
                users.erase(std::remove(users.begin(), users.end(), removing_id), users.end());
            }
            else
            {
                auto it = std::ranges::find(users, removing_id);
                if (it != users.end())
                {
                    users.erase(it);
                }
            }
        }

        /**
         * @brief Removes an identifier from the node's operand list
         * @param removing_id identifier to remove
         * @param remove_all if true, removes all occurrences; if false, removes only the first occurrence
         * @post If remove_all is true, all occurrences of removing_id are removed
         * @post If remove_all is false, only the first occurrence is removed
         * @note For gates with duplicate operands (e.g., AND(x, x)),
         *       remove_all=false will change the gate's semantics (AND(x, x) becomes AND(x))
         */
        void removeIdFromOperands(GateId removing_id, bool remove_all = false)
        {
            if (remove_all)
            {
                operands.erase(std::remove(operands.begin(), operands.end(), removing_id), operands.end());
            }
            else
            {
                auto it = std::ranges::find(operands, removing_id);
                if (it != operands.end())
                {
                    operands.erase(it);
                }
            }
        }
    };

    std::unordered_map<GateId, MutableNode> gates;
    GateIdContainer input_gates;
    GateIdContainer output_gates;
    GateId next_gate_id = 0;

    /**
     * @brief Validates that all gates identifiers in the container exist in the circuit
     * @param ids Container of gate identifiers to validate
     * @throws std::logic_error if any gate identifier in the container does not exist
     * @note The check is performed iteratively; the first non-existent ID encountered
     *       triggers an exception with the specific ID included in the error message
     */
    void validateGateIds(GateIdContainer const& ids) const
    {
        for (GateId id : ids)
        {
            if (!gates.contains(id))
            {
                throw std::logic_error("Gate ID " + std::to_string(id) + " does not exist");
            }
        }
    }

    /**
     * @brief Checks for duplicate identifiers in a container
     * @param container Container of gate identifiers to check for duplicates
     * @param context Descriptive string indicating the context of the check for meaningful error messages
     * @throws std::logic_error if the container contains duplicate identifiers
     * @note An empty container is considered valid and the check is skipped
     */
    void checkForDuplicates(GateIdContainer const& container, std::string const& context) const
    {
        if (container.empty())
        {
            return;
        }

        std::unordered_set<GateId> unique_check(container.begin(), container.end());
        if (unique_check.size() != container.size())
        {
            throw std::logic_error("Duplicate identifiers in " + context);
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

    MutableCircuit(GateInfoContainer const& gates_info, GateIdContainer const& inputs, GateIdContainer const& outputs)
        : input_gates(inputs)
        , output_gates(outputs)
        , next_gate_id(gates_info.size())
    {
        checkForDuplicates(inputs, "inputs");
        checkForDuplicates(outputs, "outputs");

        for (GateId id = 0; id < gates_info.size(); ++id)
        {
            GateInfo const& info = gates_info[id];
            gates[id]            = MutableNode(id, info.getType(), info.getOperands());
        }

        for (auto& [id, node] : gates)
        {
            GateIdContainer node_operands = node.getOperands();
            validateGateIds(node_operands);
            for (GateId operand_id : node_operands) { gates.at(operand_id).addUser(id); }
        }

        validateGateIds(input_gates);
        validateGateIds(output_gates);
    }

    MutableCircuit(GateInfoContainer&& gates_info, GateIdContainer&& inputs, GateIdContainer&& outputs)
        : input_gates(std::move(inputs))
        , output_gates(std::move(outputs))
        , next_gate_id(gates_info.size())
    {
        checkForDuplicates(input_gates, "inputs");
        checkForDuplicates(output_gates, "outputs");

        for (GateId id = 0; id < gates_info.size(); ++id)
        {
            GateInfo& info = gates_info[id];
            gates[id]      = MutableNode(id, info.getType(), info.moveOperands());
        }

        for (auto& [id, node] : gates)
        {
            GateIdContainer node_operands = node.getOperands();
            validateGateIds(node_operands);
            for (GateId operand_id : node_operands) { gates.at(operand_id).addUser(id); }
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
        return gates.at(gateId).getType();
    }

    /**
     * @brief Returns the operand container of a logic node
     * @param gateId logic node identifier
     * @return GateIdContainer const& - container of operands identifiers
     * @throws std::out_of_range if gateId does not exist
     * @note The container may contain duplicate identifiers when the same gate
     *       is used multiple times as an operand (e.g., AND(x, x) returns [x, x])
     */
    [[nodiscard]]
    GateIdContainer const& getGateOperands(GateId gateId) const override
    {
        return gates.at(gateId).getOperands();
    }

    /**
     * @brief Returns the user container of a logic node
     * @param gateId logic node identifier
     * @return GateIdContainer const& - container of users identifiers
     * @throws std::out_of_range if gateId does not exist
     * @note The container may contain duplicate identifiers when this gate
     *       is used multiple times as an operand by another gate
     *       (e.g., if y = AND(x, x) exists, then getGateUsers(x) returns [y, y])
     */
    [[nodiscard]]
    GateIdContainer const& getGateUsers(GateId gateId) const override
    {
        return gates.at(gateId).getUsers();
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
     * @note Input identifiers are guaranteed to be unique within this container
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
     * @note Output identifiers are guaranteed to be unique within this container
     * @throws std::out_of_range if gate does not exist
     */
    [[nodiscard]]
    bool isOutputGate(GateId gateId) const override
    {
        if (!gates.contains(gateId))
        {
            throw std::out_of_range("Gate " + std::to_string(gateId) + " does not exist");
        }
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
     * @param operands container of operands identifiers for the new node
     * @param is_output flag indicating whether the node is an output
     * @throws std::out_of_range if any operand identifier does not exist in the circuit
     * @post A new node with a unique identifier is created
     * @post For each operand in operands, a connection to the new node is established
     * @post If g_type == GateType::INPUT, the node is added to input_gates
     * @post If is_output == true, the node is added to output_gates
     * @note Multiple identical operands identifiers are allowed
     */
    void addGate(GateType g_type, GateIdContainer const& operands = {}, bool is_output = false)
    {
        validateGateIds(operands);

        GateId new_id = next_gate_id++;
        gates[new_id] = MutableNode(new_id, g_type, {});
        for (GateId operand_id : operands) { connectGates(operand_id, new_id); }
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
     * @param operands container of operands identifiers for the new node
     * @param users container of users identifiers for the new node
     * @param is_output flag indicating whether the node is an output
     * @throws std::out_of_range if any operand or user identifier does not exist in the circuit
     * @post A new node with a unique identifier is created
     * @post For each operand in operands, a connection to the new node is established
     * @post For each user in users, a connection from the new node is established
     * @post If g_type == GateType::INPUT, the node is added to input_gates
     * @post If is_output == true, the node is added to output_gates
     * @note Multiple identical operands identifiers and users identifiers are allowed
     */
    void addGate(GateType g_type, GateIdContainer&& operands = {}, GateIdContainer&& users = {}, bool is_output = false)
    {
        validateGateIds(operands);
        validateGateIds(users);

        GateId new_id = next_gate_id++;
        gates[new_id] = MutableNode(new_id, g_type, {});
        for (GateId operand_id : operands) { connectGates(operand_id, new_id); }
        for (GateId user_id : users) { connectGates(new_id, user_id); }
        if (g_type == GateType::INPUT)
        {
            input_gates.push_back(new_id);
            checkForDuplicates(input_gates, "inputs");
        }
        if (is_output)
        {
            output_gates.push_back(new_id);
            checkForDuplicates(output_gates, "outputs");
        }
    }

    /**
     * @brief Establishes a connection between two nodes
     * @param new_operand_gate identifier of the node that becomes an operand
     * @param new_user_gate identifier of the node that becomes a user
     * @post new_operand_gate is added to the operands of new_user_gate
     * @post new_user_gate is added to the users of new_operand_gate
     * @throws std::out_of_range if any of the identifiers does not exist
     * @note Multiple identical operand/user identifiers are allowed
     */
    void connectGates(GateId new_operand_gate, GateId new_user_gate)
    {
        gates.at(new_user_gate).addOperand(new_operand_gate);
        gates.at(new_operand_gate).addUser(new_user_gate);
    }

    /**
     * @brief Changes the type of an existing logic node
     * @param gate_id identifier of the node to modify
     * @param new_type new logic node type
     * @throws std::out_of_range if gate_id does not exist
     */
    void changeGateType(GateId gate_id, GateType new_type) { gates.at(gate_id).setType(new_type); }

    /**
     * @brief Removes a node from the circuit's input gates list
     * @param removing_gate_id identifier of the node to remove
     * @post Only the first occurrence of removing_gate_id is removed from input_gates
     * @note If removing_gate_id appears multiple times in input_gates,
     *       only the first occurrence (from the beginning) is removed
     * @note The node is completely removed from the circuit
     * @throws std::out_of_range if removing_gate_id does not exist or
     *         does not contains in input_gates
     */
    void removeFromInputs(GateId removing_gate_id)
    {
        if (!gates.contains(removing_gate_id))
        {
            throw std::out_of_range("Gate " + std::to_string(removing_gate_id) + " does not exist");
        }

        auto it = std::ranges::find(input_gates, removing_gate_id);
        if (it != input_gates.end())
        {
            input_gates.erase(it);
        }
        else
        {
            throw std::out_of_range("Gate " + std::to_string(removing_gate_id) + " does not contains in input_gates");
        }
    }

    /**
     * @brief Checks if a logic node has users
     * @param gate_id identifier of the node to check
     * @return true if the element has at least one user, false otherwise
     * @throws std::out_of_range if gate_id does not exist
     */
    bool isGateHasUsers(GateId gate_id) const { return !gates.at(gate_id).getUsers().empty(); }

    /**
     * @brief Removes a logic node from the circuit
     * @param removing_gate_id identifier of the node to remove
     * @throws std::out_of_range if removing_gate_id does not exist
     * @throws std::logic_error if removing_gate_id has users
     * @throws std::logic_error if removing_gate_id is an output node
     * @post All connections to and from the gate are properly removed
     * @post The gate is completely removed from the circuit
     * @post If the gate was an input, it is removed from input_gates
     */
    void removeGate(GateId removing_gate_id)
    {
        if (!gates.contains(removing_gate_id))
        {
            throw std::out_of_range("Gate " + std::to_string(removing_gate_id) + " does not exist");
        }

        if (isGateHasUsers(removing_gate_id))
        {
            throw std::logic_error("Cannot remove gate " + std::to_string(removing_gate_id) + " that has users");
        }

        if (isOutputGate(removing_gate_id))
        {
            throw std::logic_error("Cannot remove gate " + std::to_string(removing_gate_id) + " that is an output");
        }

        MutableNode& node = gates.at(removing_gate_id);

        for (GateId operand_id : node.getOperands())
        {
            if (gates.contains(operand_id))
            {
                gates.at(operand_id).removeIdFromUsers(removing_gate_id, true);
            }
        }

        if (node.getType() == GateType::INPUT)
        {
            removeFromInputs(removing_gate_id);
        }

        gates.erase(removing_gate_id);
    }
};
}  // namespace cirbo