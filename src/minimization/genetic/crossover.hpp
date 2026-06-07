#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

#include "core/structures/mutable_circuit.hpp"
#include "minimization/genetic/utils/log_utils.hpp"
// #include "core/algo.hpp"


namespace cirbo::minimization::genetic
{
template <typename CircuitT>
std::vector<GateId> buildTopologicalOrder(CircuitT const& circuit)
{
    enum class DFSState : uint8_t
    {
        UNVISITED,
        ENTERED,
        VISITED
    };

    std::unordered_map<GateId, DFSState> state;

    /*
     * INIT STATES ONLY FOR EXISTING GATES
     */

    for (GateId id = 0; id < circuit.getNumberOfGates(); ++id)
    {
        if (circuit.isGateExists(id))
        {
            state[id] = DFSState::UNVISITED;
        }
    }

    /*
     * SOURCES:
     * same logic as TopSortAlgorithm<DFSTopSort>
     */

    std::vector<GateId> sources;

    for (GateId id = 0; id < circuit.getNumberOfGates(); ++id)
    {
        if (!circuit.isGateExists(id))
        {
            continue;
        }

        if (circuit.getGateUsers(id).empty())
        {
            sources.push_back(id);
        }
    }

    std::vector<GateId> topo;

    std::stack<GateId> stack;

    auto push_if_unvisited =
        [&](GateId id)
    {
        auto it = state.find(id);

        if (it != state.end() &&
            it->second == DFSState::UNVISITED)
        {
            stack.push(id);
        }
    };

    /*
     * DFS
     */

    for (GateId start : sources)
    {
        push_if_unvisited(start);

        while (!stack.empty())
        {
            GateId gate = stack.top();

            switch (state[gate])
            {
                case DFSState::UNVISITED:
                {
                    state[gate] = DFSState::ENTERED;

                    auto const& operands =
                        circuit.getGateOperands(gate);

                    /*
                     * IMPORTANT:
                     * reverse iteration exactly matches original DFS
                     */

                    for (auto it = operands.rbegin();
                         it != operands.rend();
                         ++it)
                    {
                        push_if_unvisited(*it);
                    }

                    break;
                }

                case DFSState::ENTERED:
                {
                    state[gate] = DFSState::VISITED;

                    topo.push_back(gate);

                    stack.pop();

                    break;
                }

                case DFSState::VISITED:
                {
                    stack.pop();
                    break;
                }
            }
        }
    }

    /*
     * Same reverse as original algorithm
     */

    std::ranges::reverse(topo);

    /*
     * Add disconnected gates
     */

    for (auto const& [id, st] : state)
    {
        if (st == DFSState::UNVISITED)
        {
            topo.push_back(id);
        }
    }

    return topo;
}

template <typename CircuitT>
std::unique_ptr<CircuitT> layerCrossoverV3(
    CircuitT const& parent1,
    CircuitT const& parent2,
    std::mt19937& rng,
    std::vector<std::string>* history = nullptr)
{
    auto topo1 = buildTopologicalOrder(parent1);
    auto topo2 = buildTopologicalOrder(parent2);

    // buildTopologicalOrder returns outputs first; addGate needs operands first.
    std::ranges::reverse(topo1);
    std::ranges::reverse(topo2);

    if (topo1.size() < 2 || topo2.size() < 2)
    {
        appendHistory(
            history,
            "crossover_v3: status=fallback, reason=topological order too small"
                + std::string(", parent1_topo_size=") + std::to_string(topo1.size())
                + ", parent2_topo_size=" + std::to_string(topo2.size()));
        return std::make_unique<CircuitT>(parent1);
    }

    size_t const parent1_input_count = parent1.getInputGates().size();
    size_t const parent2_input_count = parent2.getInputGates().size();
    size_t const parent1_output_count = parent1.getOutputGates().size();
    size_t const parent2_output_count = parent2.getOutputGates().size();

    if (parent1_input_count != parent2_input_count ||
        parent1_output_count != parent2_output_count)
    {
        appendHistory(
            history,
            "crossover_v3: status=fallback, reason=parent input/output count mismatch"
                + std::string(", parent1_inputs=") + std::to_string(parent1_input_count)
                + ", parent2_inputs=" + std::to_string(parent2_input_count)
                + ", parent1_outputs=" + std::to_string(parent1_output_count)
                + ", parent2_outputs=" + std::to_string(parent2_output_count));
        return std::make_unique<CircuitT>(parent1);
    }

    size_t const max_cut = std::min(topo1.size(), topo2.size()) - 1;

    std::vector<size_t> parent1_prefix_input_count(topo1.size() + 1, 0);
    std::vector<size_t> parent2_suffix_input_count(topo2.size() + 1, 0);

    for (size_t i = 0; i < topo1.size(); ++i)
    {
        parent1_prefix_input_count[i + 1] =
            parent1_prefix_input_count[i] +
            (parent1.getGateType(topo1[i]) == GateType::INPUT ? 1 : 0);
    }

    for (size_t i = topo2.size(); i > 0; --i)
    {
        size_t const position = i - 1;
        parent2_suffix_input_count[position] =
            parent2_suffix_input_count[position + 1] +
            (parent2.getGateType(topo2[position]) == GateType::INPUT ? 1 : 0);
    }

    std::vector<size_t> compatible_cuts;

    for (size_t cut = 1; cut <= max_cut; ++cut)
    {
        size_t const child_input_count =
            parent1_prefix_input_count[cut] +
            parent2_suffix_input_count[cut];

        if (child_input_count == parent1_input_count)
        {
            compatible_cuts.push_back(cut);
        }
    }

    if (compatible_cuts.empty())
    {
        appendHistory(
            history,
            "crossover_v3: status=fallback, reason=no compatible cut preserving input count"
                + std::string(", parent_input_count=") + std::to_string(parent1_input_count)
                + ", parent1_topo_size=" + std::to_string(topo1.size())
                + ", parent2_topo_size=" + std::to_string(topo2.size()));
        return std::make_unique<CircuitT>(parent1);
    }

    std::uniform_int_distribution<size_t> dist(0, compatible_cuts.size() - 1);
    size_t const cut_index = compatible_cuts[dist(rng)];

    std::unordered_map<GateId, size_t> topo_index1;
    std::unordered_map<GateId, size_t> topo_index2;

    for (size_t i = 0; i < topo1.size(); ++i) { topo_index1[topo1[i]] = i; }
    for (size_t i = 0; i < topo2.size(); ++i) { topo_index2[topo2[i]] = i; }

    auto child = std::make_unique<CircuitT>();

    std::unordered_map<GateId, GateId> upper_gate_to_child;
    std::unordered_map<GateId, GateId> lower_gate_to_child;
    std::unordered_map<size_t, GateId> upper_position_to_child;
    std::unordered_map<size_t, GateId> lower_position_to_child;

    size_t copied_from_parent1 = 0;
    size_t copied_from_parent2 = 0;
    size_t fixed_lower_operands_from_upper = 0;

    auto final_message = [&](
                             std::string const& status,
                             std::string const& reason = std::string{}) -> std::string
    {
        std::string message =
            "crossover_v3: status=" + status
            + ", reason=" + reason
            + ", cut_index=" + std::to_string(cut_index)
            + ", parent1_topo_size=" + std::to_string(topo1.size())
            + ", parent2_topo_size=" + std::to_string(topo2.size())
            + ", child_gates=" + std::to_string(child->getNumberOfGates())
            + ", child_inputs=" + gateIdContainerToString(child->getInputGates())
            + ", child_outputs=" + gateIdContainerToString(child->getOutputGates())
            + ", copied_from_parent1=" + std::to_string(copied_from_parent1)
            + ", copied_from_parent2=" + std::to_string(copied_from_parent2)
            + ", fixed_lower_operands_from_upper=" + std::to_string(fixed_lower_operands_from_upper);

        return message;
    };

    auto add_upper_gate = [&](size_t position) -> bool
    {
        GateId const old_id = topo1[position];

        if (upper_gate_to_child.contains(old_id))
        {
            return true;
        }

        GateIdContainer operands;

        for (GateId op : parent1.getGateOperands(old_id))
        {
            auto const op_position_it = topo_index1.find(op);

            if (op_position_it == topo_index1.end())
            {
                return false;
            }

            auto const mapped_op_it = upper_position_to_child.find(op_position_it->second);

            if (mapped_op_it == upper_position_to_child.end())
            {
                return false;
            }

            operands.push_back(mapped_op_it->second);
        }

        bool const is_output = parent2.isOutputGate(topo2[position]);
        child->addGate(parent1.getGateType(old_id), operands, is_output);

        GateId const new_id = child->getNumberOfGates() - 1;
        upper_gate_to_child[old_id] = new_id;
        upper_position_to_child[position] = new_id;
        ++copied_from_parent1;

        return true;
    };

    for (size_t i = 0; i < cut_index; ++i)
    {
        if (!add_upper_gate(i))
        {
            appendHistory(
                history,
                final_message("fallback", "unresolved upper prefix operand at position " + std::to_string(i)));
            return std::make_unique<CircuitT>(parent1);
        }

        GateId const lower_gate_at_same_position = topo2[i];
        lower_gate_to_child[lower_gate_at_same_position] = upper_position_to_child[i];
        lower_position_to_child[i] = upper_position_to_child[i];
    }

    auto add_lower_gate = [&](size_t position) -> bool
    {
        GateId const old_id = topo2[position];

        if (lower_gate_to_child.contains(old_id))
        {
            return true;
        }

        GateIdContainer operands;

        for (GateId op : parent2.getGateOperands(old_id))
        {
            auto const op_position_it = topo_index2.find(op);

            if (op_position_it == topo_index2.end())
            {
                return false;
            }

            size_t const op_position = op_position_it->second;

            if (op_position < cut_index)
            {
                auto const mapped_upper_it = upper_position_to_child.find(op_position);

                if (mapped_upper_it == upper_position_to_child.end())
                {
                    return false;
                }

                operands.push_back(mapped_upper_it->second);
                ++fixed_lower_operands_from_upper;
                continue;
            }

            auto const mapped_lower_it = lower_position_to_child.find(op_position);

            if (mapped_lower_it == lower_position_to_child.end())
            {
                return false;
            }

            operands.push_back(mapped_lower_it->second);
        }

        child->addGate(parent2.getGateType(old_id), operands, parent2.isOutputGate(old_id));

        GateId const new_id = child->getNumberOfGates() - 1;
        lower_gate_to_child[old_id] = new_id;
        lower_position_to_child[position] = new_id;
        ++copied_from_parent2;

        return true;
    };

    for (size_t i = cut_index; i < topo2.size(); ++i)
    {
        if (!add_lower_gate(i))
        {
            appendHistory(
                history,
                final_message("fallback", "unresolved lower suffix operand at position " + std::to_string(i)));
            return std::make_unique<CircuitT>(parent1);
        }
    }

    if (child->getInputGates().size() != parent1_input_count)
    {
        appendHistory(history, final_message("fallback", "child input count mismatch"));
        return std::make_unique<CircuitT>(parent1);
    }

    if (child->getOutputGates().size() != parent1_output_count)
    {
        appendHistory(history, final_message("fallback", "child output count mismatch"));
        return std::make_unique<CircuitT>(parent1);
    }

    for (GateId out : parent2.getOutputGates())
    {
        if (!lower_gate_to_child.contains(out))
        {
            appendHistory(history, final_message("fallback", "unresolved parent2 output " + std::to_string(out)));
            return std::make_unique<CircuitT>(parent1);
        }
    }

    appendHistory(history, final_message("success"));

    return child;
}

template <typename CircuitT>
std::unique_ptr<CircuitT> layerCrossoverTwoCut(
    CircuitT const& parent1,
    CircuitT const& parent2,
    std::mt19937& rng,
    std::vector<std::string>* history = nullptr)
{
    auto topo1 = buildTopologicalOrder(parent1);
    auto topo2 = buildTopologicalOrder(parent2);

    // buildTopologicalOrder returns outputs first; addGate needs operands first.
    std::ranges::reverse(topo1);
    std::ranges::reverse(topo2);

    if (topo1.size() < 3 || topo2.size() < 3)
    {
        appendHistory(
            history,
            "crossover_twocut: status=fallback, reason=topological order too small"
                + std::string(", parent1_topo_size=") + std::to_string(topo1.size())
                + ", parent2_topo_size=" + std::to_string(topo2.size()));
        return std::make_unique<CircuitT>(parent1);
    }

    size_t const parent1_input_count = parent1.getInputGates().size();
    size_t const parent2_input_count = parent2.getInputGates().size();
    size_t const parent1_output_count = parent1.getOutputGates().size();
    size_t const parent2_output_count = parent2.getOutputGates().size();

    if (parent1_input_count != parent2_input_count ||
        parent1_output_count != parent2_output_count)
    {
        appendHistory(
            history,
            "crossover_twocut: status=fallback, reason=parent input/output count mismatch"
                + std::string(", parent1_inputs=") + std::to_string(parent1_input_count)
                + ", parent2_inputs=" + std::to_string(parent2_input_count)
                + ", parent1_outputs=" + std::to_string(parent1_output_count)
                + ", parent2_outputs=" + std::to_string(parent2_output_count));
        return std::make_unique<CircuitT>(parent1);
    }

    std::vector<size_t> parent1_prefix_input_count(topo1.size() + 1, 0);
    std::vector<size_t> parent2_prefix_output_count(topo2.size() + 1, 0);
    std::vector<size_t> parent2_suffix_input_count(topo2.size() + 1, 0);
    std::vector<size_t> parent2_suffix_output_count(topo2.size() + 1, 0);

    for (size_t i = 0; i < topo1.size(); ++i)
    {
        parent1_prefix_input_count[i + 1] =
            parent1_prefix_input_count[i] +
            (parent1.getGateType(topo1[i]) == GateType::INPUT ? 1 : 0);
    }

    for (size_t i = 0; i < topo2.size(); ++i)
    {
        parent2_prefix_output_count[i + 1] =
            parent2_prefix_output_count[i] +
            (parent2.isOutputGate(topo2[i]) ? 1 : 0);
    }

    for (size_t i = topo2.size(); i > 0; --i)
    {
        size_t const position = i - 1;
        parent2_suffix_input_count[position] =
            parent2_suffix_input_count[position + 1] +
            (parent2.getGateType(topo2[position]) == GateType::INPUT ? 1 : 0);
        parent2_suffix_output_count[position] =
            parent2_suffix_output_count[position + 1] +
            (parent2.isOutputGate(topo2[position]) ? 1 : 0);
    }

    std::vector<std::pair<size_t, size_t>> compatible_cuts;

    size_t const max_cut1 = std::min(topo1.size(), topo2.size() - 1);

    for (size_t cut1 = 1; cut1 < max_cut1; ++cut1)
    {
        for (size_t cut2 = cut1 + 1; cut2 < topo2.size(); ++cut2)
        {
            size_t const child_input_count =
                parent1_prefix_input_count[cut1] +
                parent2_suffix_input_count[cut2];

            size_t const child_output_count =
                parent2_prefix_output_count[cut1] +
                parent2_suffix_output_count[cut2];

            if (child_input_count == parent1_input_count &&
                child_output_count == parent1_output_count)
            {
                compatible_cuts.emplace_back(cut1, cut2);
            }
        }
    }

    if (compatible_cuts.empty())
    {
        appendHistory(
            history,
            "crossover_twocut: status=fallback, reason=no compatible two-cut preserving input/output count"
                + std::string(", parent_inputs=") + std::to_string(parent1_input_count)
                + ", parent_outputs=" + std::to_string(parent1_output_count)
                + ", parent1_topo_size=" + std::to_string(topo1.size())
                + ", parent2_topo_size=" + std::to_string(topo2.size()));
        return std::make_unique<CircuitT>(parent1);
    }

    std::uniform_int_distribution<size_t> dist(0, compatible_cuts.size() - 1);
    auto const [cut1, cut2] = compatible_cuts[dist(rng)];

    std::unordered_map<GateId, size_t> topo_index1;
    std::unordered_map<GateId, size_t> topo_index2;

    for (size_t i = 0; i < topo1.size(); ++i) { topo_index1[topo1[i]] = i; }
    for (size_t i = 0; i < topo2.size(); ++i) { topo_index2[topo2[i]] = i; }

    auto child = std::make_unique<CircuitT>();

    std::unordered_map<GateId, GateId> upper_gate_to_child;
    std::unordered_map<GateId, GateId> lower_gate_to_child;
    std::unordered_map<size_t, GateId> upper_position_to_child;
    std::unordered_map<size_t, GateId> lower_position_to_child;

    size_t copied_from_parent1 = 0;
    size_t copied_from_parent2 = 0;
    size_t fixed_lower_operands_from_upper = 0;
    size_t rewired_removed_middle_operands = 0;

    auto final_message = [&](
                             std::string const& status,
                             std::string const& reason = std::string{}) -> std::string
    {
        std::string message =
            "crossover_twocut: status=" + status
            + ", reason=" + reason
            + ", cut1=" + std::to_string(cut1)
            + ", cut2=" + std::to_string(cut2)
            + ", removed_middle_width=" + std::to_string(cut2 - cut1)
            + ", parent1_topo_size=" + std::to_string(topo1.size())
            + ", parent2_topo_size=" + std::to_string(topo2.size())
            + ", child_gates=" + std::to_string(child->getNumberOfGates())
            + ", child_inputs=" + gateIdContainerToString(child->getInputGates())
            + ", child_outputs=" + gateIdContainerToString(child->getOutputGates())
            + ", copied_from_parent1=" + std::to_string(copied_from_parent1)
            + ", copied_from_parent2=" + std::to_string(copied_from_parent2)
            + ", fixed_lower_operands_from_upper=" + std::to_string(fixed_lower_operands_from_upper)
            + ", rewired_removed_middle_operands=" + std::to_string(rewired_removed_middle_operands);

        return message;
    };

    auto add_upper_gate = [&](size_t position) -> bool
    {
        GateId const old_id = topo1[position];

        if (upper_gate_to_child.contains(old_id))
        {
            return true;
        }

        GateIdContainer operands;

        for (GateId op : parent1.getGateOperands(old_id))
        {
            auto const op_position_it = topo_index1.find(op);

            if (op_position_it == topo_index1.end())
            {
                return false;
            }

            auto const mapped_op_it = upper_position_to_child.find(op_position_it->second);

            if (mapped_op_it == upper_position_to_child.end())
            {
                return false;
            }

            operands.push_back(mapped_op_it->second);
        }

        bool const is_output = parent2.isOutputGate(topo2[position]);
        child->addGate(parent1.getGateType(old_id), operands, is_output);

        GateId const new_id = child->getNumberOfGates() - 1;
        upper_gate_to_child[old_id] = new_id;
        upper_position_to_child[position] = new_id;
        ++copied_from_parent1;

        return true;
    };

    for (size_t i = 0; i < cut1; ++i)
    {
        if (!add_upper_gate(i))
        {
            appendHistory(
                history,
                final_message("fallback", "unresolved upper prefix operand at position " + std::to_string(i)));
            return std::make_unique<CircuitT>(parent1);
        }

        GateId const lower_gate_at_same_position = topo2[i];
        lower_gate_to_child[lower_gate_at_same_position] = upper_position_to_child[i];
        lower_position_to_child[i] = upper_position_to_child[i];
    }

    auto repair_removed_middle_operand = [&]() -> GateId
    {
        return upper_position_to_child[cut1 - 1];
    };

    auto add_lower_gate = [&](size_t position) -> bool
    {
        GateId const old_id = topo2[position];

        if (lower_gate_to_child.contains(old_id))
        {
            return true;
        }

        GateIdContainer operands;

        for (GateId op : parent2.getGateOperands(old_id))
        {
            auto const op_position_it = topo_index2.find(op);

            if (op_position_it == topo_index2.end())
            {
                return false;
            }

            size_t const op_position = op_position_it->second;

            if (op_position < cut1)
            {
                auto const mapped_upper_it = upper_position_to_child.find(op_position);

                if (mapped_upper_it == upper_position_to_child.end())
                {
                    return false;
                }

                operands.push_back(mapped_upper_it->second);
                ++fixed_lower_operands_from_upper;
                continue;
            }

            if (op_position < cut2)
            {
                operands.push_back(repair_removed_middle_operand());
                ++rewired_removed_middle_operands;
                continue;
            }

            auto const mapped_lower_it = lower_position_to_child.find(op_position);

            if (mapped_lower_it == lower_position_to_child.end())
            {
                return false;
            }

            operands.push_back(mapped_lower_it->second);
        }

        child->addGate(parent2.getGateType(old_id), operands, parent2.isOutputGate(old_id));

        GateId const new_id = child->getNumberOfGates() - 1;
        lower_gate_to_child[old_id] = new_id;
        lower_position_to_child[position] = new_id;
        ++copied_from_parent2;

        return true;
    };

    for (size_t i = cut2; i < topo2.size(); ++i)
    {
        if (!add_lower_gate(i))
        {
            appendHistory(
                history,
                final_message("fallback", "unresolved lower suffix operand at position " + std::to_string(i)));
            return std::make_unique<CircuitT>(parent1);
        }
    }

    if (child->getInputGates().size() != parent1_input_count)
    {
        appendHistory(history, final_message("fallback", "child input count mismatch"));
        return std::make_unique<CircuitT>(parent1);
    }

    if (child->getOutputGates().size() != parent1_output_count)
    {
        appendHistory(history, final_message("fallback", "child output count mismatch"));
        return std::make_unique<CircuitT>(parent1);
    }

    for (GateId out : parent2.getOutputGates())
    {
        if (!lower_gate_to_child.contains(out))
        {
            appendHistory(history, final_message("fallback", "unresolved parent2 output " + std::to_string(out)));
            return std::make_unique<CircuitT>(parent1);
        }
    }

    appendHistory(history, final_message("success"));

    return child;
}

} // namespace cirbo::minimization::genetic
