#ifndef CIRBO_SEARCH_VECTOR_ASSIGNMENT_HPP
#define CIRBO_SEARCH_VECTOR_ASSIGNMENT_HPP

#include <cstddef>
#include <algorithm>
#include <vector>

#include "core/structures/iassignment.hpp"
#include "core/types.hpp"

namespace cirbo
{

using StateVector = std::vector<GateState>;

/**
 * Trivial assignment implementation that utilizes std::vector.
 *
 * @param DynamicResize -- if true, enables automatic resizing (false is more efficient).
 */
template<bool DynamicResize = true>
struct VectorAssignment final : virtual IAssignment
{
private:
    /* carries current assignment in a vector */
    StateVector gate_states_;

public:
    VectorAssignment()           = default;
    ~VectorAssignment() override = default;

    VectorAssignment(VectorAssignment const&)            = default;
    VectorAssignment(VectorAssignment&&)                 = default;
    VectorAssignment& operator=(VectorAssignment const&) = default;
    VectorAssignment& operator=(VectorAssignment&&)      = default;

    explicit VectorAssignment(size_t const numberOfGates)
        : gate_states_(numberOfGates, GateState::UNDEFINED)
    {
    }

    void assign(GateId const gateId, GateState const state) override
    {
        if constexpr (DynamicResize)
        {
            ensureCapacity(gateId);
        }
        gate_states_.at(gateId) = state;
    }

    [[nodiscard]]
    GateState getGateState(GateId const gateId) const override
    {
        if (containsValueFor_(gateId))
        {
            return gate_states_.at(gateId);
        }

        return GateState::UNDEFINED;
    }

    [[nodiscard]]
    bool isUndefined(GateId const gateId) const override
    {
        return getGateState(gateId) == GateState::UNDEFINED;
    }

    [[nodiscard]]
    bool isDefined(GateId const gateId) const override
    {
        return getGateState(gateId) != GateState::UNDEFINED;
    }

    void clear() noexcept override { gate_states_.clear(); }

    void ensureCapacity(GateId const sz) override
    {
        gate_states_.resize(std::max(sz + 1, gate_states_.size()), GateState::UNDEFINED);
    }

private:
    [[nodiscard]]
    bool containsValueFor_(GateId const gateId) const noexcept
    {
        return gate_states_.size() > gateId;
    }
};

}  // namespace cirbo

#endif  // CIRBO_SEARCH_VECTOR_ASSIGNMENT_HPP
