#ifndef CIRBO_SEARCH_MINIMIZATION_REDUCE_NOT_COMPOSITION_HPP
#define CIRBO_SEARCH_MINIMIZATION_REDUCE_NOT_COMPOSITION_HPP

#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "core/algo.hpp"
#include "core/structures/gate_info.hpp"
#include "core/structures/icircuit.hpp"
#include "core/types.hpp"
#include "logger.hpp"
#include "minimization/transformer_base.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that cleans the circuit from unnecessary gates NOT.
 * For example: NOT(NOT(x)) => x
 *
 * Note that this algorithm requires RedundantGatesCleaner to be applied right after.
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT>>>
class ReduceNotComposition_ : public ITransformer<CircuitT>
{
private:
    std::set<GateType> validParams;

public:
    /**
     * Applies ReduceNotComposition_ transformer to `circuit`
     * @param circuit -- circuit to transform.
     * @param encoder -- circuit encoder.
     * @return  circuit and encoder after transformation.
     */
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        log::debug("=========================================================================================");
        log::debug("START ReduceNotComposition");

        log::debug("Top sort");
        GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));

        log::debug("Rebuild schema");
        GateInfoContainer gate_info(circuit->getNumberOfGates());

        for (GateId gateId : gate_sorting)
        {
            GateIdContainer new_operands_{};
            for (GateId operands : circuit->getGateOperands(gateId))
            {
                // if the current operand is NOT, then we look at its operand and, if possible, reduce the number of NOT
                if (circuit->getGateType(operands) == GateType::NOT)
                {
                    new_operands_.push_back(get_operand(*circuit, operands));
                }
                else
                {
                    new_operands_.push_back(operands);
                }
            }
            gate_info.at(gateId) = {circuit->getGateType(gateId), new_operands_};
        }

        log::debug("END ReduceNotComposition");
        log::debug("=========================================================================================");

        return {
            std::make_unique<CircuitT>(gate_info, circuit->getOutputGates()), std::make_unique<NameEncoder>(*encoder)};
    };

private:
    /**
     * Receives gate's ID which type is NOT. If the operand of this gate is also NOT, then the algorithm
     * looks at the operand of this (finded) gate and so on until it reaches an operand whose type is not NOT.
     * After counting the number of NOT's encountered, the algorithm reduces them pairwise. As a result,
     * the output will be either gate's ID whose type is not NOT, or gate's ID whose type is NOT, but its
     * operand is definitely not NOT.
     * @param circuit -- circuit to transform.
     * @param gateId -- gate's ID which type is NOT and for the user of which we should find operand
     * @return  new gate ID. Operand for user which used `gateId`
     */
    GateId get_operand(CircuitT const& circuit, GateId gateId)
    {
        bool flag         = false;
        GateId check_gate = circuit.getGateOperands(gateId).at(0);
        while (circuit.getGateType(check_gate) == GateType::NOT)
        {
            flag       = not flag;
            gateId     = check_gate;
            check_gate = circuit.getGateOperands(gateId).at(0);
        }

        if (flag)
        {
            return check_gate;
        }
        else
        {
            return gateId;
        }
    }
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_REDUCE_NOT_COMPOSITION_HPP