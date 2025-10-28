#ifndef CIRBO_SEARCH_IASSIGNMENT_HPP
#define CIRBO_SEARCH_IASSIGNMENT_HPP

#include "core/types.hpp"

namespace cirbo
{

/**
 * Pure virtual class that defines behaviour for structures, that
 * carry current assignment (`GateState` values) of Circuit gates.
 *
 * Assignment is a (full or partial) assignment of gate values to
 * `GateState` value (e.g. FALSE, TRUE, UNDEFINED). Circuit can be
 * evaluated based on given assignment, if it is sufficient to imply
 * values of output gates.
 */
class IAssignment
{
public:
    virtual ~IAssignment() = default;

    /* Sets state of given gate to provided state */
    virtual void assign(GateId, GateState) = 0;
    /* Clears all assignments and history. */
    virtual void clear() = 0;

    /* Ensures capacity of underlying array is enough to assign gate value. If it is not, resizes array. */
    virtual void ensureCapacity(GateId) = 0;

    /* Returns state of gate `gateId`. Must return UNDEFINED when GateId is not assigned yet. */
    [[nodiscard]] virtual GateState getGateState(GateId) const = 0;
    /* Returns True iff gate state is not set or is explicitly set UNDEFINED. */
    [[nodiscard]] virtual bool isUndefined(GateId) const = 0;
    /* Returns True iff gate state is set and not UNDEFINED. */
    [[nodiscard]] virtual bool isDefined(GateId gateId) const = 0;
};

}  // namespace cirbo

#endif  // CIRBO_SEARCH_IASSIGNMENT_HPP
