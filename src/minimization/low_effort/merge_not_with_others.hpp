#ifndef CIRBO_SEARCH_MINIMIZATION_MERGE_NOT_WITH_OTHERS_HPP
#define CIRBO_SEARCH_MINIMIZATION_MERGE_NOT_WITH_OTHERS_HPP

#include <map>
#include <memory>
#include <type_traits>
#include <vector>

#include "core/algo.hpp"
#include "logger.hpp"
#include "minimization/transformer_base.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that join NOT with other operators. For example:
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(0)
 * INPUT(1)             |       INPUT(1)
 * 2 = AND(0, 1)        |       2 = NAND(0, 1)
 * 3 = NOT(3)           |       OUTPUT(2)
 * OUTPUT(3)            |
 *
 * Note that this algorithm requires RedundantGatesCleaner to be applied right after.
 *
 * @tparam CircuitT
 */
template<class CircuitT>
class MergeNotWithOthers_ : public ITransformer<CircuitT>
{
public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        log::debug("START MergeNotWithOthers");
        static std::map<GateType, GateType> const inverseType{
            {GateType::AND,  GateType::NAND},
            {GateType::OR,   GateType::NOR },
            {GateType::XOR,  GateType::NXOR},
            {GateType::NAND, GateType::AND },
            {GateType::NOR,  GateType::OR  },
            {GateType::NXOR, GateType::XOR }
        };

        GateInfoContainer gate_info(circuit->getNumberOfGates());
        BoolVector visited(circuit->getNumberOfGates(), false);
        for (GateId gateId : algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit))
        {
            // Mask is necessary because we can change prepare
            // some gates before their iteration comes.
            if (visited.at(gateId)) { continue; }
            visited.at(gateId) = true;

            if (circuit->getGateType(gateId) == GateType::NOT &&
                inverseType.find(circuit->getGateType(circuit->getGateOperands(gateId).at(0))) != inverseType.end())
            {
                GateId operandId = circuit->getGateOperands(gateId).at(0);
                if (circuit->getGateUsers(operandId).size() == 1)
                {
                    // Применяем inverseType и объединяем гейты
                    // NOT + AND = NAND; NOT + NAND = AND; ...
                    gate_info.at(gateId) = {
                        inverseType.at(circuit->getGateType(operandId)), circuit->getGateOperands(operandId)};
                }
                else if (
                    circuit->getGateType(operandId) == GateType::AND ||
                    circuit->getGateType(operandId) == GateType::OR || circuit->getGateType(operandId) == GateType::XOR)
                {
                    // Оставляем и NOT, и AND/OR/XOR как есть.
                    gate_info.at(gateId) = {GateType::NOT, {operandId}};
                }
                else if (
                    circuit->getGateType(operandId) == GateType::NAND ||
                    circuit->getGateType(operandId) == GateType::NOR ||
                    circuit->getGateType(operandId) == GateType::NXOR)
                {
                    // Разбиваем NAND/NOR/NXOR и переподвешиваем. Users NAND теперь
                    // будут указывать на NOT(AND), а Users NOT(NAND) -- на AND.
                    gate_info.at(operandId) = {GateType::NOT, {gateId}};

                    gate_info.at(gateId) = {
                        inverseType.at(circuit->getGateType(operandId)), circuit->getGateOperands(operandId)};
                    // Пересобрали операнд здесь, не нужно посещать его позже.
                    visited.at(operandId) = true;
                }
            }
            else
            {
                // Gate is taken as is, but given a new id.
                gate_info.at(gateId) = {circuit->getGateType(gateId), circuit->getGateOperands(gateId)};
            }
        }
        log::debug("END MergeNotWithOthers");

        return {std::make_unique<CircuitT>(std::move(gate_info), circuit->getOutputGates()), std::move(encoder)};
    };
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_MERGE_NOT_WITH_OTHERS_HPP