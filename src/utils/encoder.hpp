#ifndef CIRBO_SEARCH_UTILS_ENCODER_HPP
#define CIRBO_SEARCH_UTILS_ENCODER_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

#include "core/types.hpp"

namespace cirbo::utils
{
/**
 * This class allows to encode original names of gates (strings) to a sequence of coalesced indexes.
 */
class NameEncoder
{
private:
    // Order matters since `map_` keys references `gate_names_` elements.
    //
    // Maps index to gate name.
    std::vector<std::string> gate_names_;
    // Maps gate name to index.
    std::unordered_map<std::string, GateId> map_;

public:
    /**
     * Encodes gate name by either issuing a new index, or returning existing one.
     * @param name: name of the gate to encode.
     * @return: index of the gate.
     */
    GateId encodeGate(std::string&& name)
    {
        if (auto const it = map_.find(name); it != map_.end())
        {
            return it->second;
        }

        const auto id = static_cast<GateId>(gate_names_.size());
        gate_names_.push_back(std::move(name));
        map_.emplace(std::string(gate_names_.back()), id);
        return id;
    }

    GateId encodeGate(std::string_view const name)
    {
        return encodeGate(std::string(name));
    }

    [[nodiscard]] std::string const& decodeGate(GateId const gateId) const
    {
        return gate_names_.at(gateId);
    }

    [[nodiscard]]
    bool keyExists(std::string const& key) const
    {
        return map_.contains(key);
    };

    [[nodiscard]] size_t size() const noexcept { return gate_names_.size(); }
    [[nodiscard]] bool empty() const noexcept { return gate_names_.empty(); }

    void clear() noexcept {
        gate_names_.clear();
        map_.clear();
    }
};

} // namespace cirbo::utils

#endif  // CIRBO_SEARCH_UTILS_ENCODER_HPP
