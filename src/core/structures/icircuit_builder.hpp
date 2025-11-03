#ifndef CIRBO_SEARCH_ICIRCUIT_BUILDER_HPP
#define CIRBO_SEARCH_ICIRCUIT_BUILDER_HPP

#include <memory>
#include <type_traits>

#include "core/structures/icircuit.hpp"

namespace cirbo
{

/**
 * Base class for boolean circuit builders.
 * @tparam CircuitT -- data structure that will
 * be returned by member-function `instantiate`.
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
class ICircuitBuilder
{
public:
    virtual ~ICircuitBuilder() = default;

    /**
     * Instantiates d CircuitT.
     * @return Circuit instance, built according to current parser info.
     */
    virtual std::unique_ptr<CircuitT> instantiate() = 0;
};

}  // namespace cirbo

#endif  // CIRBO_SEARCH_ICIRCUIT_BUILDER_HPP
