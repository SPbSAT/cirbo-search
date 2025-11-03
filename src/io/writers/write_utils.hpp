#ifndef CIRBO_SEARCH_WRITE_UTILS_HPP
#define CIRBO_SEARCH_WRITE_UTILS_HPP

#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <type_traits>

#include "core/structures/dag.hpp"
#include "core/structures/icircuit.hpp"
#include "core/types.hpp"
#include "logger.hpp"
#include "utils/cast.hpp"
#include "utils/encoder.hpp"

namespace cirbo::io::writers
{

/**
 * Write the circuit to a bench file
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
void writeBenchFile(CircuitT const& circuit, utils::NameEncoder const& encoder, std::ofstream& file_out)
{
    log::debug("writeBenchFile start.");

    log::debug("recording INPUTs.");
    for (GateId input : circuit.getInputGates()) { file_out << "INPUT(" << encoder.decodeGate(input) << ")\n"; }
    file_out << "\n";

    log::debug("recording OUTPUTs.");
    for (GateId output : circuit.getOutputGates()) { file_out << "OUTPUT(" << encoder.decodeGate(output) << ")\n"; }
    file_out << "\n";

    log::debug("recording Gates.");
    for (size_t gateId = 0; gateId < circuit.getNumberOfGates(); ++gateId)
    {
        if (circuit.getGateType(gateId) != GateType::INPUT)
        {
            file_out << encoder.decodeGate(gateId) << " = " << utils::gateTypeToString(circuit.getGateType(gateId))
                     << "(";

            auto operands       = circuit.getGateOperands(gateId);
            size_t num_operands = operands.size();

            if (num_operands == 0)
            {
                file_out << ")\n";
                continue;
            }

            for (size_t operand = 0; operand < (num_operands - 1); ++operand)
            {
                file_out << encoder.decodeGate(operands.at(operand)) << ", ";
            }
            file_out << encoder.decodeGate(operands.at(num_operands - 1)) << ")\n";
        }
    }
    log::debug("writeBenchFile end.");
}

/**
 * Prints circuit to stdout where each gate is written in the following notation
 * "<encoded name> => <name from original file>"
 */
inline void printCircuit(DAG const& circuit, utils::NameEncoder const& encoder)
{
    for (auto input : circuit.getInputGates())
    {
        std::cout << "INPUT(" << input << " => " << encoder.decodeGate(input) << ")\n";
    }

    for (auto output : circuit.getOutputGates())
    {
        std::cout << "OUTPUT(" << output << " => " << encoder.decodeGate(output) << ")\n";
    }

    for (size_t gateId = 0; gateId < circuit.getNumberOfGates(); ++gateId)
    {
        if (circuit.getGateType(gateId) != GateType::INPUT)
        {
            std::cout << gateId << " => " << encoder.decodeGate(gateId) << " = "
                      << utils::gateTypeToString(circuit.getGateType(gateId)) << "(";

            auto operands       = circuit.getGateOperands(gateId);
            size_t const num_operands = operands.size();

            if (num_operands == 0)
            {
                std::cout << ")\n";
                continue;
            }

            for (size_t operand = 0; operand < (num_operands - 1); ++operand)
            {
                std::cout << operands.at(operand) << " => " << encoder.decodeGate(operands.at(operand)) << ", ";
            }
            std::cout << operands.at(num_operands - 1) << " => " << encoder.decodeGate(operands.at(num_operands - 1))
                      << ")\n";
        }

        std::cout << std::flush;
    }
}

}  // namespace cirbo::io::writers

#endif  // CIRBO_SEARCH_WRITE_UTILS_HPP
