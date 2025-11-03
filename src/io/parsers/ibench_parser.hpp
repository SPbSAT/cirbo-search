#ifndef CIRBO_SEARCH_PARSERS_IBENCH_PARSER_HPP
#define CIRBO_SEARCH_PARSERS_IBENCH_PARSER_HPP

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>

#include "core/types.hpp"
#include "io/parsers/iparser.hpp"
#include "logger.hpp"
#include "utils/encoder.hpp"
#include "utils/string_utils.hpp"

/**
 * Parser from `CircuitSAT.BENCH` file..
 */
namespace cirbo::io::parsers
{

constexpr std::string_view INPUT_STRING  = "INPUT";
constexpr std::string_view OUTPUT_STRING = "OUTPUT";

/**
 * Base class for CircuitSAT.BENCH parsers.
 */
class IBenchParser : public virtual ICircuitParser
{
public:
    ~IBenchParser() override = default;

    /**
     * Parser info from stream, that is .BENCH format file.
     * @param stream -- text stream, containing lines of some .BENCH file.
     */
    void parseStream(std::istream& stream) override
    {
        log::debug("Started parsing of BENCH stream.");
        std::string line;
        while (std::getline(stream, line)) { parseBenchLine_(line); }
        _eof();
        log::debug("Ended parsing of BENCH stream.");
    }

    /* Encoder of inputs and gates. */
    utils::NameEncoder encoder;

    /**
     * @return Encoder, built according to parser info.
     */
    [[nodiscard]]
    utils::NameEncoder const& getEncoder() const
    {
        return encoder;
    }

protected:
    /**
     * Encode circuit variable.
     * @param var_name -- name of encoded variable.
     */
    virtual GateId encodeGate(std::string_view var_name) { return encoder.encodeGate(var_name); };

    /**
     * Circuit input handler.
     * @param gateId -- name of processed variable.
     */
    inline virtual void handleInput(GateId gateId) = 0;

    /**
     * Circuit output handler.
     * @param gateId -- name of processed variable.
     */
    inline virtual void handleOutput(GateId gateId) = 0;

    /**
     * Circuit gate handler.
     * @param op -- operation.
     * @param gateId -- gate.
     * @param var_operands -- operands of gate.
     */
    inline virtual void handleGate(std::string_view op, GateId gateId, GateIdContainer const& var_operands) = 0;

    /**
     * Handles specific operators which are found in some benchmarks.
     * e.g. CONST(0) and CONST(1) operators.
     * @param op -- operation.
     * @param gateId -- gate.
     * @param operands_str -- operands of gate.
     */
    inline virtual bool specialOperatorCallback_(GateId gateId, std::string_view op, std::string_view operands_str) = 0;

    /* Parses one line of bench file. */
    virtual void parseBenchLine_(std::string_view line)
    {
        log::debug("Parsing Line: \"", line, "\".");
        string_utils::trimSpaces(&line);
        if (line.empty() || line[0] == '#' || line == "\n")
        {
            log::debug("\tReceived comment or empty line.");
            return;
        }

        if (line.starts_with(INPUT_STRING))
        {
            log::debug("\tReceived input gate line.");
            std::string_view var_name =
                line.substr(INPUT_STRING.length() + 1, line.find(')') - INPUT_STRING.length() - 1);
            string_utils::trimSpaces(&var_name);

            log::debug("\tEncoding input gate: \"", var_name, "\".");
            GateId const gateId = encodeGate(var_name);
            handleInput(gateId);
            return;
        }

        if (line.starts_with(OUTPUT_STRING))
        {
            log::debug("\tReceived output gate line.");
            std::string_view var_name =
                line.substr(OUTPUT_STRING.length() + 1, line.find(')') - OUTPUT_STRING.length() - 1);
            string_utils::trimSpaces(&var_name);

            log::debug("\tEncoding output gate: \"", var_name, "\".");
            GateId const gateId = encodeGate(var_name);
            handleOutput(gateId);
            return;
        }

        // Operator gate

        // Find special delimiters positions
        auto [eq_idx, l_bkt_idx, r_bkt_idx] = _getDelimitersPositions(line);

        std::string_view var_name = line.substr(0, eq_idx);
        string_utils::trimSpaces(&var_name);

        std::string_view op = line.substr(eq_idx + 1, l_bkt_idx - eq_idx - 1);
        string_utils::trimSpaces(&op);

        std::string_view operands_str;
        if (r_bkt_idx > l_bkt_idx)
        {
            operands_str = line.substr(l_bkt_idx + 1, r_bkt_idx - l_bkt_idx - 1);
            string_utils::trimSpaces(&operands_str);
        }
        else
        {
            operands_str = line.substr(0, 0);
        }

        GateId const gateId = encodeGate(var_name);

        if (specialOperatorCallback_(gateId, op, operands_str))
        {
#ifdef ENABLE_DEBUG_LOGGING
            log::debug(
                "Line with specific operator is parsed. ",
                "Operator: \"",
                op,
                "\";\n",
                "\tEncoded Name: \"",
                encoder.encodeGate(var_name),
                "\";\n",
                "\tOperands str: \"",
                operands_str,
                "\"");
#endif

            // If operator is specific it will be handled in the callback method, then no
            // additional processing is necessary
            return;
        }

        GateIdContainer var_operands;
        size_t comma_idx = 0;
        while ((comma_idx = operands_str.find(',')) != std::string::npos)
        {
            std::string_view operand = operands_str.substr(0, comma_idx);
            string_utils::trimSpaces(&operand);
            var_operands.push_back(encodeGate(operand));
            operands_str = operands_str.substr(comma_idx + 1, operands_str.size() - comma_idx - 1);
        }
        string_utils::trimSpaces(&operands_str);
        var_operands.push_back(encodeGate(operands_str));

#ifdef ENABLE_DEBUG_LOGGING
        log::debug(
            "Line parsed. Operator: \"",
            op,
            "\";\n",
            "\tOperator: \"",
            op,
            "\";\n",
            "\tName: \"",
            var_name,
            "\";\n",
            "\tEncoded Name: \"",
            encoder.encodeGate(var_name),
            "\";\n",
            "\tOperands:\n");
        for (auto const& x : var_operands) { log::debug("\t\"", x, "\"."); }
#endif

        handleGate(op, gateId, var_operands);
    };

    /* Post-parsing calculations of data structures. */
    virtual void _eof() {
        // empty
    };

    /* Returns delimiters positions ( '=', '(', ')' ) in an operator bench line. */
    static std::tuple<size_t, size_t, size_t> _getDelimitersPositions(std::string_view const line)
    {
        std::size_t const line_size = line.size();
        // Find special delimiters positions
        std::size_t eq_idx    = std::string::npos;
        std::size_t l_bkt_idx = std::string::npos;
        std::size_t r_bkt_idx = std::string::npos;
        for (size_t idx = 0; idx < line_size; ++idx)
        {
            switch (line.at(idx))
            {
                case '=':
                    eq_idx = idx;
                    break;
                case '(':
                    l_bkt_idx = idx;
                    break;
                case ')':
                    r_bkt_idx = idx;
                    break;
                default:
                    break;
            }
        }

        // Check validity of delimiters position
        if (eq_idx != std::string::npos && l_bkt_idx == std::string::npos && r_bkt_idx == std::string::npos)
        {
            std::string_view token = line.substr(eq_idx + 1, line_size - eq_idx - 1);
            string_utils::trimSpaces(&token);
            if (token == "vdd")
            {
                // The line with the operator `vdd` does not contain any brackets or operands.
                // After `=` char there is only token `vdd` written (despite spaces),
                // therefore the index of the left bracket is set to be equal to the end of the line.
                // The index of the right bracket is also made equal to the end of the line,
                // for this case a separate processing is implemented.
                return {eq_idx, line_size, line_size};
            }
        }

        if (eq_idx == std::string::npos || l_bkt_idx == std::string::npos || r_bkt_idx == std::string::npos ||
            eq_idx >= l_bkt_idx || eq_idx >= r_bkt_idx || l_bkt_idx >= r_bkt_idx)
        {
            std::cerr << "Can't parse line: \"" << line << "\"" << std::endl;
            std::abort();
        }

        return {eq_idx, l_bkt_idx, r_bkt_idx};
    }
};

}  // namespace cirbo::io::parsers

#endif  // CIRBO_SEARCH_PARSERS_IBENCH_PARSER_HPP
