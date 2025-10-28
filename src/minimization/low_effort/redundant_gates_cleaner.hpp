#ifndef CIRBO_SEARCH_MINIMIZATION_REDUNDANT_GATES_CLEANER_HPP
#define CIRBO_SEARCH_MINIMIZATION_REDUNDANT_GATES_CLEANER_HPP

#include <cassert>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "core/algo.hpp"
#include "core/types.hpp"
#include "minimization/transformer_base.hpp"
#include "core/structures/gate_info.hpp"
#include "core/structures/icircuit.hpp"
#include "utils/encoder.hpp"
#include "logger.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that cleans circuit from all
 * gates that are not reachable from outputs.
 *
 * @tparam CircuitT
 * @tparam preserveInputs -- if True, then Inputs won't be removed from a circuit.
 */
template<
    class CircuitT,
    bool preserveInputs = false,
    typename            = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT>>>
class RedundantGatesCleaner_ : public ITransformer<CircuitT>
{
  public:
    /**
     * Applies RedundantGatesCleaner_ transformer to `circuit`
     * @param circuit -- circuit to transform.
     * @param encoder -- circuit encoder.
     * @return  circuit and encoder after transformation.
     */
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        log::debug("=========================================================================================");
        log::debug("START RedundantGatesCleaner.");

        NameEncoder new_encoder{};
        // Use dfs to get markers of visited and unvisited gates
        auto mask_use_output = algo::performDepthFirstSearch(*circuit, circuit->getOutputGates());

        // First step: getting valid encoding -- encode only gates, which will
        // be taken to the new circuit (hence discarding all redundant gates).
        for (GateId gateId = 0; gateId < circuit->getNumberOfGates(); ++gateId)
        {
            std::string_view const gate_name = encoder->decodeGate(gateId);
            if (mask_use_output.at(gateId) != algo::DFSState::UNVISITED ||
                (preserveInputs && circuit->getGateType(gateId) == GateType::INPUT))
            {
                new_encoder.encodeGate(gate_name);
                continue;
            }

            log::debug("Gate '", gate_name, "' (#", gateId, ") is redundant and will be removed");
        }

        // Second step: recollect each gate data by encoding all its operands with
        // new encoder build above.
        GateInfoContainer gate_info(new_encoder.size());
        for (GateId gateId = 0; gateId < circuit->getNumberOfGates(); ++gateId)
        {
            std::string const& gate_name = encoder->decodeGate(gateId);
            // If new encoder doesn't contain name of a gate, this gate is redundant.
            if (new_encoder.keyExists(gate_name))
            {
                GateIdContainer encoded_operands_{};
                for (GateId operand : circuit->getGateOperands(gateId))
                {
                    // All operands must be visited, since current gate was visited.
                    assert(
                        mask_use_output.at(gateId) != algo::DFSState::UNVISITED ||
                        (preserveInputs && circuit->getGateType(gateId) == GateType::INPUT));

                    std::string_view const operand_name = encoder->decodeGate(operand);
                    encoded_operands_.push_back(new_encoder.encodeGate(operand_name));
                }
                // Build new gate info object for current gate.
                gate_info.at(new_encoder.encodeGate(gate_name)) = {
                    circuit->getGateType(gateId), std::move(encoded_operands_)};
            }
        }

        // Third step: recollect output gates.
        // All outputs must be visited since DFS starts from them.
        GateIdContainer new_output_gates{};
        new_output_gates.reserve(circuit->getOutputGates().size());
        for (GateId output_gate : circuit->getOutputGates())
        {
            assert(
                mask_use_output.at(output_gate) != algo::DFSState::UNVISITED ||
                (preserveInputs && circuit->getGateType(output_gate) == GateType::INPUT));

            std::string_view const output_name = encoder->decodeGate(output_gate);
            new_output_gates.push_back(new_encoder.encodeGate(output_name));
        }

        log::debug("END RedundantGatesCleaner.");
        log::debug("=========================================================================================");
        return {
            std::make_unique<CircuitT>(std::move(gate_info), std::move(new_output_gates)),
            std::make_unique<NameEncoder>(new_encoder)};
    };
};

}  // namespace cirbo::minimization

#endif // CIRBO_SEARCH_MINIMIZATION_REDUNDANT_GATES_CLEANER_HPP