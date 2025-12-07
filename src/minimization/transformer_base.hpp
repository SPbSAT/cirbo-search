#ifndef CIRBO_SEARCH_MINIMIZATION_TRANSFORMER_BASE_HPP
#define CIRBO_SEARCH_MINIMIZATION_TRANSFORMER_BASE_HPP

#include <type_traits>
#include <string>
#include <memory>
#include <utility>
#include <random>

#include "core/structures/icircuit.hpp"
#include "core/types.hpp"
#include "utils/encoder.hpp"
#include "utils/random.hpp"

namespace cirbo::minimization
{

using utils::NameEncoder;

template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT>>>
using CircuitAndEncoder = std::pair<std::unique_ptr<CircuitT>, std::unique_ptr<NameEncoder>>;

/**
 * Base interface for all circuit transformers.
 *
 * @tparam CircuitT -- type of a circuit structure.
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT>>>
class ITransformer
{
public:
    virtual ~ITransformer() = default;

    CircuitAndEncoder<CircuitT, std::string> apply(CircuitT const& circuit, NameEncoder const& encoder)
    {
        return transform(std::make_unique<CircuitT>(circuit), std::make_unique<NameEncoder>(encoder));
    }

    virtual CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT>,
        std::unique_ptr<NameEncoder>) = 0;
};

static std::string getUniqueId_()
{
    constexpr int MIN_INT = 100'999'999;
    constexpr int MAX_INT = 999'999'999;

    // Currently not the best way of random number generation
    // is presented, but it should be enough since number of
    // transformers applied to a circuit is relatively low.
    static auto engine(utils::getNewMersenneTwisterEngine());
    static std::uniform_int_distribution<> dist(MIN_INT, MAX_INT);

    return std::to_string(dist(engine));
}

inline std::string getNewGateName_(std::string const& prefix, GateId id) { return prefix + std::to_string(id); }

inline std::string getNewGateName_(std::string const& prefix, std::string&& id) { return prefix + id; }

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_TRANSFORMER_BASE_HPP