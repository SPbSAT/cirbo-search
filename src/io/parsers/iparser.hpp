#ifndef CIRBO_SEARCH_IO_PARSERS_IPARSER_HPP
#define CIRBO_SEARCH_IO_PARSERS_IPARSER_HPP

#include <iostream>

#include "core/types.hpp"
#include "utils/encoder.hpp"

/**
 * Parser that process stream of lines defining some boolean circuit and return a `RawCircuit` structure.
 */
namespace cirbo::io::parsers
{

/**
 * Base class for a boolean circuit parsers.
 */
class ICircuitParser
{
public:
    virtual ~ICircuitParser() = default;

    /**
     * Clears internal state of a parser.
     */
    virtual void clear() = 0;

    /**
     * Parser info from stream.
     * @param stream -- text stream, containing lines of some .BENCH file.
     */
    virtual void parseStream(std::istream& stream) = 0;
};

}  // namespace cirbo::io::parsers

#endif  // CIRBO_SEARCH_IO_PARSERS_IPARSER_HPP
