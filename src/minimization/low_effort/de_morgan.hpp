#ifndef CIRBO_SEARCH_MINIMIZATION_DE_MORGAN_HPP
#define CIRBO_SEARCH_MINIMIZATION_DE_MORGAN_HPP

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include "core/algo.hpp"
#include "core/structures/gate_info.hpp"
#include "core/structures/icircuit.hpp"
#include "core/types.hpp"
#include "logger.hpp"
#include "minimization/transformer_base.hpp"

namespace cirbo::minimization
{

/**
 * Transformer, that moves NOT closer to INPUT using de Morgan's low: NOT(AND(1, 2)) = OR(NOT(1), NOT(2));
 *                                                                    NOT(OR(1, 2)) = AND(NOT(1), NOT(2))
 *
 * Note that this algorithm requires RedundantGatesCleaner and DuplicateGatesCleaner and MergeNotWithOthers
 * to be applied right before and RedundantGatesCleaner right after.
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
class DeMorgan_ : public ITransformer<CircuitT>
{
public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        log::debug("=========================================================================================");
        log::debug("START DeMorgan");
        std::map<GateType, GateType> inverse_type = {
            {GateType::AND,  GateType::OR },
            {GateType::OR,   GateType::AND},
            {GateType::NAND, GateType::OR },
            {GateType::NOR,  GateType::AND},
            {GateType::NXOR, GateType::XOR},
        };
        bool rehang = false;

        auto new_gate_name_prefix = (getUniqueId_() + "::new_gate_de_Morgan@");

        log::debug("Top sort");
        GateIdContainer const gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));

        log::debug("Rebuild schema");
        GateIdContainer indexes_of_not(circuit->getNumberOfGates(), SIZE_MAX);
        GateIdContainer count_branches(circuit->getNumberOfGates(), 0);
        GateInfoContainer gate_info(circuit->getNumberOfGates());

        for (GateId gateId : gate_sorting)
        {
            GateIdContainer const& operands = circuit->getGateOperands(gateId);
            if (indexes_of_not.at(gateId) != SIZE_MAX)  // если перед гейтом, есть фиктивный NOT
            {
                if (circuit->getGateType(gateId) == GateType::AND || circuit->getGateType(gateId) == GateType::OR)
                {
                    // если хотябы один User гейта не использует его отрицание (т.е. User не использовал правило де
                    // Моргана), то гейт оставляем не тронутым, чтобы не дублировать поддерево
                    if (count_branches[gateId] != circuit->getGateUsers(gateId).size())
                    {
                        // создаем NOT
                        gate_info.at(indexes_of_not.at(gateId)) = {GateType::NOT, GateIdContainer{gateId}};
                    }
                    else
                    {
                        // применяем правило де Моргана
                        gate_info.at(indexes_of_not.at(gateId)) = {
                            inverse_type.at(circuit->getGateType(gateId)),
                            get_new_operands_(
                                *circuit,
                                gate_info,
                                *encoder,
                                indexes_of_not,
                                gateId,
                                new_gate_name_prefix,
                                count_branches)};
                    }
                }
                else if (
                    circuit->getGateType(gateId) == GateType::NAND || circuit->getGateType(gateId) == GateType::NOR ||
                    circuit->getGateType(gateId) == GateType::NXOR)
                {
                    // заменяем NOT + NOR/NAND/NXOR на OR/AND/XOR
                    gate_info.at(indexes_of_not.at(gateId)) = {
                        static_cast<GateType>(static_cast<uint8_t>(circuit->getGateType(gateId)) - 1), operands};

                    // если хотябы один User гейта не использует его отрицание, то необходимо перевесить гейты, то есть
                    // измениь тип текущего гейта на NOT. Все User'ы использующие правило де Моргана будут ссылаться на
                    // OR/AND/XOR, остальные на NOT(OR/AND/XOR)
                    if (count_branches[gateId] != circuit->getGateUsers(gateId).size())
                    {
                        rehang = true;
                    }
                }
                else
                {
                    // создаем NOT
                    gate_info.at(indexes_of_not.at(gateId)) = {GateType::NOT, GateIdContainer{gateId}};
                }

                // сохраняем гейт в нетронутом виде или изменяем его тип на NOT при необходимости для не переподвешенных
                // ссылок
                if (rehang)
                {
                    gate_info.at(gateId) = {GateType::NOT, GateIdContainer{indexes_of_not.at(gateId)}};
                    rehang               = false;
                }
                else
                {
                    gate_info.at(gateId) = {circuit->getGateType(gateId), operands};
                }
            }
            else if (
                circuit->getGateType(gateId) == GateType::NOT &&
                (circuit->getGateType(operands.at(0)) == GateType::AND ||
                 circuit->getGateType(operands.at(0)) == GateType::OR))
            {
                // так как мы незнаем можно ли здесь применяться де моргана, то представим, что текущий NOT - фиктивный,
                // и примим решение когда дойдет до его операнда
                indexes_of_not.at(operands.at(0)) = gateId;
            }
            else if (circuit->getGateType(gateId) == GateType::NAND || circuit->getGateType(gateId) == GateType::NOR)
            {
                // применяем правило де Моргана
                gate_info.at(gateId) = {
                    inverse_type.at(circuit->getGateType(gateId)),
                    get_new_operands_(
                        *circuit, gate_info, *encoder, indexes_of_not, gateId, new_gate_name_prefix, count_branches)};
            }
            else
            {
                // гейт не трогаем
                gate_info.at(gateId) = {circuit->getGateType(gateId), operands};
            }
        }

        log::debug("END DeMorgan");
        log::debug("=========================================================================================");

        return {
            std::make_unique<CircuitT>(gate_info, circuit->getOutputGates()), std::make_unique<NameEncoder>(*encoder)};
    };

private:
    GateId find_index_of_not_(CircuitT const& circuit, GateIdContainer const& indexes_of_not, GateId gateId)
    {
        // NOT у гейта будет всегда только один, так как перед алгоритмом DeMorgan_ применяется DuplicatesCleaner,
        // а во время его работы создается фиктивный гейт NOT только после проверки, что реального нет

        if (indexes_of_not.at(gateId) != SIZE_MAX)
        {
            return indexes_of_not.at(gateId);
        }

        for (GateId user : circuit.getGateUsers(gateId))
        {
            if (circuit.getGateType(user) == GateType::NOT)
            {
                return user;
            }
        }
        return SIZE_MAX;
    }

    GateIdContainer get_new_operands_(
        CircuitT const& circuit,
        GateInfoContainer& gate_info,
        NameEncoder& encoder,
        GateIdContainer& indexes_of_not,
        GateId gateId,
        std::string const& new_gate_name_prefix,
        GateIdContainer& count_branches)
    {
        GateIdContainer new_operands{};
        for (GateId operand : circuit.getGateOperands(gateId))
        {
            GateId index_of_not = find_index_of_not_(circuit, indexes_of_not, operand);
            if (index_of_not == SIZE_MAX)
            {
                GateId const new_gateId = encoder.encodeGate(new_gate_name_prefix + std::to_string(encoder.size()));
                gate_info.resize(new_gateId + 1);
                indexes_of_not.at(operand) = new_gateId;
                new_operands.push_back(new_gateId);
            }
            else
            {
                new_operands.push_back(index_of_not);
            }
            // запишем что один из пользователей operand требует NOT(operand)
            count_branches[operand] += 1;
        }

        return new_operands;
    }
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_DE_MORGAN_HPP