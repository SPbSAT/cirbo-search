#ifndef CIRBO_SEARCH_GATE_INFO_HPP
#define CIRBO_SEARCH_GATE_INFO_HPP

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "core/types.hpp"
#include "utils/cast.hpp"

namespace cirbo
{

/**
 * Auxiliary structure to carry non-structured information about some gate.
 *
 * Invariants:
 *   - if gate type is symmetric, stored operands are always sorted in ascending order.
 *
 */
struct GateInfo
{
protected:
    GateType type_ = GateType::UNDEFINED;
    GateIdContainer operands_;

public:
    GateInfo()  = default;
    ~GateInfo() = default;

    GateInfo(GateType const type, GateIdContainer const& operands)
        : type_(type)
        , operands_(operands)
    {
        if (utils::symmetricOperatorQ(type)) { std::ranges::sort(operands_); }
    }

    GateInfo(GateType const type, GateIdContainer&& operands)
        : type_(type)
        , operands_(std::move(operands))
    {
        if (utils::symmetricOperatorQ(type)) { std::ranges::sort(operands_); }
    }

    [[nodiscard]]
    GateIdContainer const& getOperands() const
    {
        return operands_;
    }

    [[nodiscard]]
    GateType getType() const
    {
        return type_;
    }

    [[nodiscard]]
    GateIdContainer&& moveOperands()
    {
        // This function leaves `this` in
        // invalid state. Use it carefully.
        return std::move(operands_);
    }
};

/** At i'th position carries info about gate with GateId = i. **/
using GateInfoContainer = std::vector<GateInfo>;

}  // namespace cirbo

#endif  // CIRBO_SEARCH_GATE_INFO_HPP
