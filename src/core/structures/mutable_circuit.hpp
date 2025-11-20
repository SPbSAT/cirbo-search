#pragma once

#include <algorithm>
#include "core/structures/dag.hpp"
#include "core/structures/gate_info.hpp"
#include "core/structures/icircuit.hpp"

namespace cirbo
{
    class MutableNode
    {
    private:
        GateId id = 0;
        GateType type = GateType::UNDEFINED;
        GateIdContainer operands;
        GateIdContainer users;

    public:
        MutableNode() = default;
        ~MutableNode() = default;
        MutableNode(MutableNode& m_node) = default;
        MutableNode(MutableNode const& m_node) = default;
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
            GateIdContainer const& g_users
            )
            : id(g_id)
            , type(g_type)
            , operands(g_operands)
            , users(g_users)
        {
        }
        
        MutableNode(
            GateId const g_id, 
            GateType const g_type, 
            GateIdContainer&& g_operands,
            GateIdContainer&& g_users
        )
            : id(g_id)
            , type(g_type)
            , operands{std::move(g_operands)}
            , users{std::move(g_users)}
        {
        }

        MutableNode(
            GateId const g_id, 
            GateType const g_type,
            GateIdContainer const& g_operands
            )
            : id(g_id)
            , type(g_type)
            , operands(g_operands)
        {
        }
        
        MutableNode(
            GateId const g_id, 
            GateType const g_type, 
            GateIdContainer&& g_operands
            )
            : id(g_id)
            , type(g_type)
            , operands{std::move(g_operands)}
        {
        }

        GateId getId() const 
        { 
            return id;
        }

        GateType getType() const
        {
            return type;
        }

        GateIdContainer const& getOperands() const
        {
            return operands;
        }

        GateIdContainer const& getUsers() const
        {
            return users;
        }

        void addUser(GateId const& gateId)
        {
            users.push_back(gateId);
        }

        void addUsersContainer(GateIdContainer const& usersIdContainer)
        {
            users.insert(users.end(), usersIdContainer.begin(), usersIdContainer.end());
        }

        void addUsersContainer(GateIdContainer&& usersIdContainer)
        {
            users.insert(users.end(), 
                        std::move_iterator(usersIdContainer.begin()),
                        std::move_iterator(usersIdContainer.end()));
        }

        void addOperand(GateId const& gateId)
        {
            operands.push_back(gateId);
        }

        void addOperandsContainer(GateIdContainer const& operandsIdContainer)
        {
            operands.insert(operands.end(), operandsIdContainer.begin(), operandsIdContainer.end());
        }

        void addOperandsContainer(GateIdContainer&& operandsIdContainer)
        {
            operands.insert(operands.end(), 
                        std::move_iterator(operandsIdContainer.begin()),
                        std::move_iterator(operandsIdContainer.end()));
        }

        void setType(GateType const& newType)
        {
            type = newType;
        }

        void setOperands(GateIdContainer const& operandsIdContainer)
        {
            operands = operandsIdContainer;
        }

        void setOperands(GateIdContainer&& operandsIdContainer)
        {
            operands = std::move(operandsIdContainer);
        }

        void setUsers(GateIdContainer const& usersIdContainer)
        {
            users = usersIdContainer;
        }

        void setUsers(GateIdContainer&& usersIdContainer)
        {
            users = std::move(usersIdContainer);
        }

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
    
}