#ifndef CIRBO_SEARCH_MINIMIZATION_NEST_HPP
#define CIRBO_SEARCH_MINIMIZATION_NEST_HPP

#include <cstddef>
#include <string>
#include <utility>

#include "core/structures/icircuit.hpp"
#include "minimization/composition.hpp"
#include "minimization/transformer_base.hpp"

namespace cirbo::minimization
{

/**
 * Class that represents a nest of transformers, meaning that
 * provided transformers will be applied to a circuit `n` times.
 *
 * @tparam CircuitT -- class that carries circuit.
 * @tparam OtherTransformersT -- transformers which application is to be nested `n` times.
 *
 */
template<class CircuitT, std::size_t n, class... OtherTransformersT>
struct Nest : virtual ITransformer<CircuitT>
{
    static_assert(std::is_base_of<ICircuit, CircuitT>::value, "CircuitT must be an implementation of a ICircuit.");
    static_assert(
        (std::is_base_of_v<ITransformer<CircuitT>, OtherTransformersT> && ...),
        "All simplifier template args of Composition must implement "
        "ITransformer and be parametrized with CircuitT type.");

public:
    /**
     * Applies all defined in template TransformersT to
     * `circuit` `n` times, in left-to-right order. For example,
     * Nest<DAG, 3, T1, T2, T3>::apply(circuit) will perform
     *
     *     Composition<DAG, T1, T2, T3>().apply(
     *         Composition<DAG, T1, T2, T3>().apply(
     *             Composition<DAG, T1, T2, T3>().apply(circuit)
     *         )
     *     );
     * @param circuit -- circuit to transform.
     * @return circuit, that is result of transformation.
     */
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<NameEncoder> encoder)
    {
        std::unique_ptr<CircuitT> circuit_    = std::move(circuit);
        std::unique_ptr<NameEncoder> encoder_ = std::move(encoder);
        for (std::size_t it = 0; it < n; ++it)
        {
            auto comp                    = Composition<CircuitT, OtherTransformersT...>();
            std::tie(circuit_, encoder_) = comp.transform(std::move(circuit_), std::move(encoder_));
        }
        return CircuitAndEncoder<CircuitT, std::string>(std::move(circuit_), std::move(encoder_));
    }
};

}  // namespace cirbo::minimization

#endif  // CIRBO_SEARCH_MINIMIZATION_NEST_HPP