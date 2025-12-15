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

    /**
     * @brief Возвращает идентификатор узла
     * @return GateId - идентификатор узла
     */
    GateId getId() const { return id; }

    /**
     * @brief Возвращает тип логического узла
     * @return GateType - тип логического узла
     */
    GateType getType() const { return type; }

    /**
     * @brief Возвращает константную ссылку на контейнер операндов узла
     * @return GateIdContainer const& - контейнер идентификаторов операндов
     */
    GateIdContainer const& getOperands() const { return operands; }

    /**
     * @brief Возвращает константную ссылку на контейнер пользователей узла
     * @return GateIdContainer const& - контейнер идентификаторов пользователей
     */
    GateIdContainer const& getUsers() const { return users; }

    /**
     * @brief Добавляет идентификатор пользователя в список пользователей узла
     * @param gateId идентификатор пользователя для добавления
     * @post gateId добавлен в конец контейнера users
     */
    void addUser(GateId const& gateId) { users.push_back(gateId); }

    /**
     * @brief Добавляет контейнер идентификаторов пользователей в список пользователей узла
     * @param usersIdContainer контейнер идентификаторов пользователей для добавления
     * @post Все элементы usersIdContainer добавлены в конец контейнера users
     */
    void addUsersContainer(GateIdContainer const& usersIdContainer)
    {
        users.insert(users.end(), usersIdContainer.begin(), usersIdContainer.end());
    }

    /**
     * @brief Добавляет контейнер идентификаторов пользователей в список пользователей узла с перемещением
     * @param usersIdContainer контейнер идентификаторов пользователей для перемещения
     * @post Все элементы usersIdContainer добавлены в конец контейнера users
     */
    void addUsersContainer(GateIdContainer&& usersIdContainer)
    {
        users.insert(
            users.end(), std::move_iterator(usersIdContainer.begin()), std::move_iterator(usersIdContainer.end()));
    }

    /**
     * @brief Добавляет идентификатор операнда в список операндов узла
     * @param gateId идентификатор операнда для добавления
     * @post gateId добавлен в конец контейнера operands
     */
    void addOperand(GateId const& gateId) { operands.push_back(gateId); }

    /**
     * @brief Добавляет контейнер идентификаторов операндов в список операндов узла
     * @param operandsIdContainer контейнер идентификаторов операндов для добавления
     * @post Все элементы operandsIdContainer добавлены в конец контейнера operands
     */
    void addOperandsContainer(GateIdContainer const& operandsIdContainer)
    {
        operands.insert(operands.end(), operandsIdContainer.begin(), operandsIdContainer.end());
    }

    /**
     * @brief Добавляет контейнер идентификаторов операндов в список операндов узла с перемещением
     * @param operandsIdContainer контейнер идентификаторов операндов для перемещения
     * @post Все элементы operandsIdContainer добавлены в конец контейнера operands
     */
    void addOperandsContainer(GateIdContainer&& operandsIdContainer)
    {
        operands.insert(
            operands.end(),
            std::move_iterator(operandsIdContainer.begin()),
            std::move_iterator(operandsIdContainer.end()));
    }

    /**
     * @brief Устанавливает новый тип логического узла
     * @param newType новый тип логического узла
     */
    void setType(GateType const& newType) { type = newType; }

    /**
     * @brief Заменяет текущие операнды узла новыми
     * @param operandsIdContainer новый контейнер идентификаторов операндов
     */
    void setOperands(GateIdContainer const& operandsIdContainer) { operands = operandsIdContainer; }

    /**
     * @brief Заменяет текущие операнды узла новыми с перемещением
     * @param operandsIdContainer новый контейнер идентификаторов операндов для перемещения
     */
    void setOperands(GateIdContainer&& operandsIdContainer) { operands = std::move(operandsIdContainer); }

    /**
     * @brief Заменяет текущих пользователей узла новыми
     * @param usersIdContainer новый контейнер идентификаторов пользователей
     */
    void setUsers(GateIdContainer const& usersIdContainer) { users = usersIdContainer; }

    /**
     * @brief Заменяет текущих пользователей узла новыми с перемещением
     * @param usersIdContainer новый контейнер идентификаторов пользователей
     */
    void setUsers(GateIdContainer&& usersIdContainer) { users = std::move(usersIdContainer); }

    /**
     * @brief Удаляет идентификатор из списка пользователей узла
     * @param removing_id идентификатор для удаления
     * @post removing_id удален из контейнера users
     * @post Если removing_id не найден, контейнер users не изменяется
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
     * @brief Удаляет идентификатор из списка операндов узла
     * @param removing_id идентификатор для удаления
     * @post removing_id удален из контейнера operands
     * @post Если removing_id не найден, контейнер operands не изменяется
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

    /**
     * @brief Возвращает тип логического узла по его идентификатору
     * @param gateId идентификатор логического узла
     * @return GateType - тип логического узла
     * @throws std::out_of_range если gateId не существует
     */
    [[nodiscard]]
    GateType getGateType(GateId gateId) const override
    {
        return gates.at(gateId).first.getType();
    }

    /**
     * @brief Возвращает контейнер операндов логического узла
     * @param gateId идентификатор логического узла
     * @return GateIdContainer const& - контейнер идентификаторов операндов
     * @throws std::out_of_range если gateId не существует
     */
    [[nodiscard]]
    GateIdContainer const& getGateOperands(GateId gateId) const override
    {
        return gates.at(gateId).first.getOperands();
    }

    /**
     * @brief Возвращает контейнер пользователей логического узла
     * @param gateId идентификатор логического узла
     * @return GateIdContainer const& - контейнер идентификаторов пользователей
     * @throws std::out_of_range если gateId не существует
     */
    [[nodiscard]]
    GateIdContainer const& getGateUsers(GateId gateId) const override
    {
        return gates.at(gateId).first.getUsers();
    }

    /**
     * @brief Возвращает общее количество логических узлов в схеме
     * @return GateId - количество логических узлов
     * @post Результат включает входные, выходные и внутренние узлы
     */
    [[nodiscard]]
    GateId getNumberOfGates() const override
    {
        return gates.size();
    }

    /**
     * @brief Возвращает количество логических узлов без учета входных
     * @return GateId - количество логических узлов без входных гейтов
     * @post Результат включает выходные и внутренние узлы
     */
    [[nodiscard]]
    GateId getNumberOfGatesWithoutInputs() const override
    {
        return gates.size() - input_gates.size();
    }

    /**
     * @brief Возвращает контейнер идентификаторов выходных узлов схемы
     * @return GateIdContainer const& - контейнер идентификаторов выходных узлов
     */
    [[nodiscard]]
    GateIdContainer const& getOutputGates() const override
    {
        return output_gates;
    }

    /**
     * @brief Возвращает контейнер идентификаторов входных узлов схемы
     * @return GateIdContainer const& - контейнер идентификаторов входных узлов
     */
    [[nodiscard]]
    GateIdContainer const& getInputGates() const override
    {
        return input_gates;
    }

    /**
     * @brief Проверяет, является ли элемент выходным узлом схемы
     * @param gateId идентификатор логического узлв
     * @return true если узел является выходным, иначе false
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
     * @brief Добавляет новый логический узел в схему
     * @param g_type тип нового логического узла
     * @param operands контейнер идентификаторов операндов нового узла
     * @param is_output флаг, указывающий является ли узел выходным
     * @post Создан новый узел с уникальным идентификатором
     * @post Для каждого операнда в operands установлена связь с новым узлом
     * @post Если g_type == GateType::INPUT, узел добавлен в input_gates
     * @post Если is_output == true, узел добавлен в output_gates
     * @post next_gate_id увеличен на 1
     */
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

    /**
     * @brief Добавляет новый логический узел в схему с перемещением параметров
     * @param g_type тип нового логического узла
     * @param operands контейнер идентификаторов операндов нового узла
     * @param is_output флаг, указывающий является ли узел выходным
     * @post Создан новый узел с уникальным идентификатором
     * @post Для каждого операнда в operands установлена связь с новым узлом
     * @post Если g_type == GateType::INPUT, узел добавлен в input_gates
     * @post Если is_output == true, узел добавлен в output_gates
     * @post next_gate_id увеличен на 1
     */
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

    /**
     * @brief Добавляет новый логический узел на основе существующего
     * @param m_node узел, на основе которого создается новый
     * @param is_output флаг, указывающий является ли узел выходным
     * @post Создан новый узел с уникальным идентификатором и параметрами из m_node
     * @post Для каждого операнда в m_node установлена связь с новым узлом
     * @post Если тип узла == GateType::INPUT, он добавлен в input_gates
     * @post Если is_output == true, он добавлен в output_gates
     * @post next_gate_id увеличен на 1
     */
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

    /**
     * @brief Устанавливает связь между двумя узлами
     * @param new_operand_gate идентификатор узла, который становится операндом
     * @param new_user_gate идентификатор узла, который становится пользователем
     * @post new_operand_gate добавлен в операнды new_user_gate
     * @post new_user_gate добавлен в пользователи new_operand_gate
     * @throws std::out_of_range если любой из идентификаторов не существует
     */
    void connectGates(GateId new_operand_gate, GateId new_user_gate)
    {
        gates.at(new_user_gate).first.addOperand(new_operand_gate);
        gates.at(new_operand_gate).first.addUser(new_user_gate);
    }

    /**
     * @brief Изменяет тип существующего логического узла
     * @param gate_id идентификатор изменяемого узла
     * @param new_type новый тип логического узла
     * @throws std::out_of_range если gate_id не существует
     */
    void changeGateType(GateId gate_id, GateType new_type) { gates.at(gate_id).first.setType(new_type); }

    /**
     * @brief Очищает список операндов логического узла и удаляет соответствующие связи
     * @param gate_id идентификатор узла
     * @post Все узлы удалены из списка операндов gate_id
     * @post Для каждого бывшего операнда удален gate_id из списка пользователей
     * @throws std::out_of_range если gate_id не существует
     */
    void clearGateOperands(GateId gate_id)
    {
        GateIdContainer operands = gates.at(gate_id).first.getOperands();
        for (GateId operand_id : operands) { gates.at(operand_id).first.removeIdFromUsers(gate_id); }
        gates.at(gate_id).first.setOperands({});
    }

    /**
     * @brief Удаляет узел из списка входных гейтов схемы
     * @param removing_gate_id идентификатор удаляемого узла
     * @post Если removing_gate_id был во входных узлах, он удален из input_gates
     * @post Флаг удалённого узла установлен в true
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

    // /**
    //  * @brief Удаляет элемент из списка выходных элементов схемы
    //  * @param removing_gate_id идентификатор удаляемого элемента
    //  */
    // void removeFromOutputs(GateId removing_gate_id)
    // {
    //     auto it = std::ranges::find(output_gates, removing_gate_id);
    //     if (it != output_gates.end())
    //     {
    //         output_gates.erase(it);
    //     }
    // }

    /**
     * @brief Проверяет существование логического узла в схеме
     * @param gate_id идентификатор проверяемого узла
     * @return true если элемент существует, false в противном случае
     */
    bool isGateExists(GateId gate_id) const { return gates.find(gate_id) != gates.end(); }

    /**
     * @brief Проверяет, имеет ли логический узел пользователей
     * @param gate_id идентификатор проверяемого узла
     * @return true если элемент имеет хотя бы одного пользователя, false в противном случае
     * @throws std::out_of_range если gate_id не существует
     */
    bool isGateHasUsers(GateId gate_id) const { return !gates.at(gate_id).first.getUsers().empty(); }

    /**
     * @brief Удаляет логический узел из схемы
     * @param removing_gate_id идентификатор удаляемого узла
     * @pre removing_gate_id должен существовать в схеме
     * @pre removing_gate_id не должен иметь пользователей
     * @post Если элемент был входным, он удаляется из input_gates
     * @post Если элемент не был входным, очищаются его операнды
     * @post Элемент остается в контейнере gates с флагом удаления
     */
    void removeGate(GateId removing_gate_id)
    {
        if (isGateExists(removing_gate_id) && !isGateHasUsers(removing_gate_id))
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