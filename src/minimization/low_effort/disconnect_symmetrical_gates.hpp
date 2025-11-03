#ifndef CIRBO_SEARCH_MINIMIZATION_DISCONNECT_SYMMETRICAL_GATES_HPP
#define CIRBO_SEARCH_MINIMIZATION_DISCONNECT_SYMMETRICAL_GATES_HPP

#include <memory>
#include <set>
#include <type_traits>
#include <vector>

#include "core/algo.hpp"
#include "core/types.hpp"
#include "logger.hpp"
#include "minimization/transformer_base.hpp"
#include "utils/cast.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that separates symmetrical gates (AND and OR) into gates of the specified arity using nesting.
 * For example: arity = 2; AND(0, 1, 2) => AND(AND(0, 1), 2)
 *
 * Note that this algorithm requires RedundantGatesCleaner to be applied right before.
 *
 * @tparam CircuitT
 * @param arity
 */
template<
    class CircuitT,
    int arity,
    bool EnableAND = false,
    bool EnableOR  = false,
    bool EnableXOR = false,
    typename       = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
class DisconnectSymmetricalGates_ : public ITransformer<CircuitT>
{
private:
    std::set<GateType> validParams;

public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        log::debug("=========================================================================================");
        log::debug("START DisconnectSymmetricalGates");

        if constexpr (EnableAND)
        {
            validParams.insert(GateType::AND);
        }
        if constexpr (EnableOR)
        {
            validParams.insert(GateType::OR);
        }
        if constexpr (EnableXOR)
        {
            validParams.insert(GateType::XOR);
        }

        log::debug("Top sort");
        cirbo::GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));

        log::debug("Rebuild schema");
        GateInfoContainer gate_info(circuit->getNumberOfGates());

        for (auto it = gate_sorting.rbegin(); it != gate_sorting.rend(); ++it)
        {
            GateId gateId                   = *it;
            GateIdContainer const& operands = circuit->getGateOperands(gateId);
            if ((operands.size() > arity) && (validParams.find(circuit->getGateType(gateId)) != validParams.end()))
            {
                GateId operandId = 0;
                GateIdContainer new_operands_{};
                for (; operandId < (operands.size() - 1); ++operandId)
                {
                    new_operands_.push_back(operands.at(operandId));

                    if (new_operands_.size() == arity)
                    {
                        GateId new_gateID =
                            encoder->encodeGate("new_gate_disconnect_gates" + std::to_string((*encoder).size()));
                        gate_info.emplace_back(circuit->getGateType(gateId), new_operands_);

                        new_operands_.clear();
                        new_operands_.push_back(new_gateID);
                    }
                }
                new_operands_.push_back(operands.at(operandId));
                gate_info.at(gateId) = {circuit->getGateType(gateId), new_operands_};
            }
            else
            {
                gate_info.at(gateId) = {circuit->getGateType(gateId), operands};
            }
        }

        log::debug("END DisconnectSymmetricalGates");
        log::debug("=========================================================================================");

        return {
            std::make_unique<CircuitT>(gate_info, circuit->getOutputGates()), std::make_unique<NameEncoder>(*encoder)};
    };
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_DISCONNECT_SYMMETRICAL_GATES_HPP