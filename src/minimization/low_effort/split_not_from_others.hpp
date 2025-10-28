#ifndef CIRBO_SEARCH_MINIMIZATION_SPLIT_NOT_FROM_OTHERS_HPP
#define CIRBO_SEARCH_MINIMIZATION_SPLIT_NOT_FROM_OTHERS_HPP

#include <algorithm>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

#include "core/algo.hpp"
#include "logger.hpp"
#include "minimization/transformer_base.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that split NOT with other operators, preserving functional equivalence on gateId.
 * For example:
 *
 *    Before            |          After
 *
 * INPUT(0)             |       INPUT(0)
 * INPUT(1)             |       INPUT(1)
 * 2 = NAND(0, 1)       |       2 = NOT(3)
 * OUTPUT(2)            |       3 = AND(0, 1)
 *                      |       OUTPUT(2)
 *
 * @tparam CircuitT
 */
template<class CircuitT>
class SplitNotFromOthers_ : public ITransformer<CircuitT>
{
public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        static std::map<GateType, GateType> const inverse_type = {
            {GateType::NAND, GateType::AND},
            {GateType::NOR,  GateType::OR },
            {GateType::NXOR, GateType::XOR}
        };

        log::debug("START SplitNotFromOthers");
        cirbo::GateIdContainer sorted_gates(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));

        size_t circuit_size = circuit->getNumberOfGates();
        GateInfoContainer gate_info(circuit_size);

        for (auto gateId : std::ranges::reverse_view(sorted_gates))
        {
            // В качестве ключей мапы `inverse_type` выступают только отрицания операторов.
            if (inverse_type.find(circuit->getGateType(gateId)) != inverse_type.end())
            {
                // Разделяем NAND/NOR/NXOR на NOT(AND)/NOT(OR)/NOT(XOR) и переподвешиваем.
                // Например: User'ы NAND теперь будут указывать на NOT(AND).

                // Инвертированный гейт получает новый id.
                GateId new_gate_id =
                    encoder->encodeGate("new_gate_split_not_with_others@" + std::to_string(circuit_size));
                gate_info.emplace_back(
                    inverse_type.at(circuit->getGateType(gateId)),
                    // Оставляем старые операнды т.к. они либо не изменились, либо стали NOT'ами.
                    circuit->getGateOperands(gateId));

                assert(new_gate_id == circuit_size);
                ++circuit_size;
                assert(gate_info.size() == circuit_size);

                // Соответствующий ему NOT будет кодироваться старым id,
                // т.к. эквивалентен старому гейту.
                gate_info.at(gateId) = {GateType::NOT, GateIdContainer{new_gate_id}};
            }
            else
            {
                // Берем гейт "as is".
                gate_info.at(gateId) = {circuit->getGateType(gateId), circuit->getGateOperands(gateId)};
            }
        }
        log::debug("END SplitNotFromOthers");

        return {
            std::make_unique<CircuitT>(std::move(gate_info), circuit->getOutputGates()),
            std::make_unique<NameEncoder>(*encoder)};
    };
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_SPLIT_NOT_FROM_OTHERS_HPP