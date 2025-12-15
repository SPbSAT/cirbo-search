#pragma once

#include <algorithm>
#include <unordered_map>

#include "core/structures/dag.hpp"
#include "core/structures/gate_info.hpp"
#include "core/structures/icircuit.hpp"

namespace cirbo
{
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

    void addUser(GateId const& gateId) { users.push_back(gateId); }

    void addUsersContainer(GateIdContainer const& usersIdContainer)
    {
        users.insert(users.end(), usersIdContainer.begin(), usersIdContainer.end());
    }

    void addUsersContainer(GateIdContainer&& usersIdContainer)
    {
        users.insert(
            users.end(), std::move_iterator(usersIdContainer.begin()), std::move_iterator(usersIdContainer.end()));
    }

    void addOperand(GateId const& gateId) { operands.push_back(gateId); }

    void addOperandsContainer(GateIdContainer const& operandsIdContainer)
    {
        operands.insert(operands.end(), operandsIdContainer.begin(), operandsIdContainer.end());
    }

    void addOperandsContainer(GateIdContainer&& operandsIdContainer)
    {
        operands.insert(
            operands.end(),
            std::move_iterator(operandsIdContainer.begin()),
            std::move_iterator(operandsIdContainer.end()));
    }

    void setType(GateType const& newType) { type = newType; }

    void setOperands(GateIdContainer const& operandsIdContainer) { operands = operandsIdContainer; }

    void setOperands(GateIdContainer&& operandsIdContainer) { operands = std::move(operandsIdContainer); }

    void setUsers(GateIdContainer const& usersIdContainer) { users = usersIdContainer; }

    void setUsers(GateIdContainer&& usersIdContainer) { users = std::move(usersIdContainer); }

    void removeIdFromUsers(GateId const removing_id)
    {
        auto it = std::ranges::find(users, removing_id);
        if (it != users.end())
        {
            users.erase(it);
        }
    }

    void removeIdFromOperands(GateId const removing_id)
    {
        auto it = std::ranges::find(operands, removing_id);
        if (it != operands.end())
        {
            operands.erase(it);
        }
    }
};

class MutableCircuit : public ICircuit
{
private:
    std::unordered_map<GateId, std::pair<MutableNode, bool>> gates;
    GateIdContainer input_gates;
    GateIdContainer output_gates;
    GateId next_gate_id;

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
        std::unordered_map<GateId, std::pair<MutableNode, bool>> const& m_nodes,
        GateIdContainer const& inputs,
        GateIdContainer const& outputs)
        : gates(m_nodes)
        , input_gates(inputs)
        , output_gates(outputs)
        , next_gate_id(0)
    {
        if (!m_nodes.empty())
        {
            auto it = std::ranges::max_element(m_nodes, [](auto const& a, auto const& b) { return a.first < b.first; });

            next_gate_id = it->first + 1;
        }
    }

    MutableCircuit(
        std::vector<MutableNode> const& m_nodes,
        GateIdContainer const& inputs,
        GateIdContainer const& outputs)
        : input_gates(inputs)
        , output_gates(outputs)
        , next_gate_id(0)
    {
        size_t max_id = 0;
        for (MutableNode const& node : m_nodes)
        {
            gates[node.getId()] = std::make_pair(node, false);
            max_id              = std::max(max_id, node.getId());
        }
        next_gate_id = max_id + 1;
    }

    MutableCircuit(std::vector<MutableNode> const& m_nodes, GateIdContainer&& inputs, GateIdContainer&& outputs)
        : input_gates{std::move(inputs)}
        , output_gates{std::move(outputs)}
        , next_gate_id(0)
    {
        size_t max_id = 0;
        for (MutableNode const& node : m_nodes)
        {
            gates[node.getId()] = std::make_pair(node, false);
            max_id              = std::max(max_id, node.getId());
        }
        next_gate_id = max_id + 1;
    }

    [[nodiscard]]
    GateType getGateType(GateId gateId) const override
    {
        return gates.at(gateId).first.getType();
    }

    [[nodiscard]]
    GateIdContainer const& getGateOperands(GateId gateId) const override
    {
        return gates.at(gateId).first.getOperands();
    }

    [[nodiscard]]
    GateIdContainer const& getGateUsers(GateId gateId) const override
    {
        return gates.at(gateId).first.getUsers();
    }

    [[nodiscard]]
    GateId getNumberOfGates() const override
    {
        return gates.size();
    }

    [[nodiscard]]
    GateId getNumberOfGatesWithoutInputs() const override
    {
        return gates.size() - input_gates.size();
    }

    [[nodiscard]]
    GateIdContainer const& getOutputGates() const override
    {
        return output_gates;
    }

    [[nodiscard]]
    GateIdContainer const& getInputGates() const override
    {
        return input_gates;
    }

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

    void addGate(GateType const& g_type, GateIdContainer const& operands = {}, bool is_output = false)
    {
        GateId new_id = next_gate_id++;
        gates[new_id] = std::make_pair(MutableNode(new_id, g_type, operands), false);
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

    void addGate(
        GateType const&& g_type,
        GateIdContainer const&& operands = {},
        GateIdContainer const&& users    = {},
        bool is_output                   = false)
    {
        GateId new_id = next_gate_id++;
        gates[new_id] = std::make_pair(MutableNode(new_id, g_type, operands, users), false);
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

    void addGate(MutableNode const& m_node, bool is_output = false)
    {
        GateId new_id = next_gate_id++;
        gates[new_id] =
            std::make_pair(MutableNode(new_id, m_node.getType(), m_node.getOperands(), m_node.getUsers()), false);
        for (GateId operand_id : m_node.getOperands())
        {
            if (gates.contains(operand_id))
            {
                connectGates(operand_id, new_id);
            }
        }
        if (m_node.getType() == GateType::INPUT)
        {
            input_gates.push_back(new_id);
        }
        if (is_output)
        {
            output_gates.push_back(new_id);
        }
    }

    void connectGates(GateId new_operand_gate, GateId new_user_gate)
    {
        gates.at(new_user_gate).first.addOperand(new_operand_gate);
        gates.at(new_operand_gate).first.addUser(new_user_gate);
    }

    void changeGateType(GateId gate_id, GateType new_type) { gates.at(gate_id).first.setType(new_type); }

    void clearGateOperands(GateId gate_id)
    {
        GateIdContainer operands = gates.at(gate_id).first.getOperands();
        for (GateId operand_id : operands) { gates.at(operand_id).first.removeIdFromUsers(gate_id); }
        gates.at(gate_id).first.setOperands({});
    }

    void removeFromInputs(GateId removing_gate_id)
    {
        auto it = std::ranges::find(input_gates, removing_gate_id);
        if (it != input_gates.end())
        {
            input_gates.erase(it);
            gates[removing_gate_id].second = true;
        }
    }

    void removeFromOutputs(GateId removing_gate_id)
    {
        auto it = std::ranges::find(output_gates, removing_gate_id);
        if (it != output_gates.end())
        {
            output_gates.erase(it);
        }
    }

    bool isGateExists(GateId gate_id) const { return gates.find(gate_id) != gates.end(); }

    bool isGateHasUsers(GateId gate_id) const { return !gates.at(gate_id).first.getUsers().empty(); }

    void removeGate(GateId removing_gate_id)
    {
        if (isGateExists(removing_gate_id) && isGateHasUsers(removing_gate_id))
        {
            gates[removing_gate_id].second = true;
            if (gates[removing_gate_id].first.getType() == GateType::INPUT)
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
    }
};
}  // namespace cirbo