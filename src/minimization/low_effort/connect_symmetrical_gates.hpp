#ifndef CIRBO_SEARCH_MINIMIZATION_CONNECT_SYMMETRICAL_GATES_HPP
#define CIRBO_SEARCH_MINIMIZATION_CONNECT_SYMMETRICAL_GATES_HPP

#include <memory>
#include <queue>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/algo.hpp"
#include "core/types.hpp"
#include "logger.hpp"
#include "minimization/transformer_base.hpp"
#include "utils/cast.hpp"

namespace cirbo::minimization::impl
{

struct VisitCounter
{
    GateId iteration_gate = SIZE_MAX;
    size_t counter        = 0;
};

}  // namespace cirbo::minimization::impl

namespace cirbo::minimization
{

/**
 * Transformer, that remove nesting of symmetrical gates (AND, OR and XOR) if they are specified in the params.
 * For example: AND(AND(0, 1), 2) => AND(0, 1, 2)
 *
 * Note that this algorithm requires RedundantGatesCleaner to be applied right before and right after.
 *
 * @tparam CircuitT
 */
template<class CircuitT, bool EnableAND = false, bool EnableOR = false, bool EnableXOR = false>
class ConnectSymmetricalGates_ : public ITransformer<CircuitT>
{
public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder) override
    {
        log::debug("START ConnectSymmetricalGates");

        static std::set<GateType> validConnectTypes{};
        if constexpr (EnableAND) validConnectTypes.insert(GateType::AND);
        if constexpr (EnableOR) validConnectTypes.insert(GateType::OR);
        if constexpr (EnableXOR) validConnectTypes.insert(GateType::XOR);

        GateInfoContainer gate_info(circuit->getNumberOfGates());
        cirbo::BoolVector visit_mask(circuit->getNumberOfGates(), false);
        std::vector<impl::VisitCounter> visit_counters(circuit->getNumberOfGates());

        // From outputs to inputs.
        for (GateId gateId : algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit))
        {
            // Mask is needed because some gates can be decided
            // to be unnecessary before their iteration comes.
            if (visit_mask.at(gateId)) { continue; }
            visit_mask.at(gateId) = true;

            if (validConnectTypes.find(circuit->getGateType(gateId)) != validConnectTypes.end())
            {
                gate_info.at(gateId) = {
                    circuit->getGateType(gateId), buildNewOperands_(*circuit, visit_mask, visit_counters, gateId)};
            }
            else { gate_info.at(gateId) = {circuit->getGateType(gateId), circuit->getGateOperands(gateId)}; }
        }

        log::debug("END ConnectSymmetricalGates");

        return {std::make_unique<CircuitT>(std::move(gate_info), circuit->getOutputGates()), std::move(encoder)};
    };

private:
    inline GateIdContainer buildNewOperands_(
        CircuitT const& circuit,
        cirbo::BoolVector& visit_mask,
        std::vector<impl::VisitCounter>& visit_counters,
        GateId iteration_gate)
    {
        bool is_xor_gate =
            (circuit.getGateType(iteration_gate) == GateType::XOR ||
             circuit.getGateType(iteration_gate) == GateType::NXOR);

        GateIdContainer new_operands_{};
        GateIdContainer gates_to_check{};

        std::unordered_map<GateId, size_t> number_of_takes{};
        number_of_takes[iteration_gate] = 1;

        std::queue<GateId> visit_queue;
        visit_queue.push(iteration_gate);
        while (!visit_queue.empty())
        {
            auto curr_gate = visit_queue.front();
            visit_queue.pop();

            visit_mask.at(curr_gate) = true;

            for (GateId operandId : circuit.getGateOperands(curr_gate))
            {
                // Increment counter of gate visits.
                if (visit_counters.at(operandId).counter == 0)
                {
                    visit_counters.at(operandId).iteration_gate = iteration_gate;
                }
                ++visit_counters.at(operandId).counter;

                if (circuit.getGateType(curr_gate) == circuit.getGateType(operandId) &&
                    visit_counters.at(operandId).iteration_gate == iteration_gate &&
                    visit_counters.at(operandId).counter == circuit.getGateUsers(operandId).size() &&
                    !circuit.isOutputGate(operandId))
                {
                    // Гейт нам полностью подходит. Мы берем всех его детей.
                    visit_queue.push(operandId);
                    number_of_takes[operandId] += number_of_takes[curr_gate];
                }
                else if (
                    circuit.getGateType(curr_gate) != circuit.getGateType(operandId) ||
                    visit_counters.at(operandId).iteration_gate != iteration_gate || circuit.isOutputGate(operandId))
                {
                    // Гейт нам точно не подходит. Он, либо имеет дугу на другой гейт
                    // (вне текущей итерации), либо не совпадает оператором либо он
                    // является выходным гейтом схемы.
                    if (is_xor_gate)
                    {
                        // We need to take each occurence in XOR since XOR evaluated oddity.
                        for (size_t i = 0; i < number_of_takes[curr_gate]; ++i) { new_operands_.push_back(operandId); }
                    }
                    else { new_operands_.push_back(operandId); }
                }
                else
                {
                    // Гейт совпадает операндом, но нам необходимо убедиться, что все его юзеры войдут в текущую
                    // итеррацию и будут тоже объеденены. В случае если мы пройдем всех его юзеров в текущей итерации
                    // он попадет в первую ветки и мы возьмем его детей. Иначе Id гейта будет взято после цикла.
                    gates_to_check.push_back(operandId);
                    number_of_takes[operandId] += number_of_takes[curr_gate];
                }
            }
        }

        for (GateId gateId : gates_to_check)
        {
            if (visit_counters.at(gateId).counter != circuit.getGateUsers(gateId).size())
            {
                if (is_xor_gate)
                {
                    // We need to take each occurence in XOR since XOR evaluated oddity.
                    while (number_of_takes[gateId] != 0)
                    {
                        new_operands_.push_back(gateId);
                        --number_of_takes[gateId];
                    }
                }
                else
                {
                    if (number_of_takes[gateId] != 0)
                    {
                        // For OR and AND we can take gate once
                        new_operands_.push_back(gateId);
                        number_of_takes[gateId] = 0;
                    }
                }
            }
        }

        return new_operands_;
    }
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_CONNECT_SYMMETRICAL_GATES_HPP