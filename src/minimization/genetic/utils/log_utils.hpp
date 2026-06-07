#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "core/types.hpp"
#include "utils/cast.hpp"

namespace cirbo::minimization::genetic
{

inline void appendHistory(std::vector<std::string>* history, std::string entry)
{
    if (history != nullptr)
    {
        history->push_back(std::move(entry));
    }
}

inline std::string gateIdContainerToString(GateIdContainer const& ids)
{
    std::ostringstream out;
    out << "[";

    for (size_t i = 0; i < ids.size(); ++i)
    {
        if (i > 0)
        {
            out << ", ";
        }
        out << ids[i];
    }

    out << "]";
    return out.str();
}

inline std::string gateTypeToString(GateType gate_type)
{
    return cirbo::utils::gateTypeToString(gate_type);
}

}  // namespace cirbo::minimization::genetic
