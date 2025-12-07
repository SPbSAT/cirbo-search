#ifndef CIRBO_SEARCH_STRING_UTILS_HPP
#define CIRBO_SEARCH_STRING_UTILS_HPP

#include <cstddef>
#include <string>
#include <string_view>

#include "utils/optimize.hpp"

namespace cirbo::string_utils
{

/**
 * Returns True iff given string is either empty, or a raw line
 * with only newline characters.
 *
 * @param str -- string view to process.
 */
CIRBO_OPT_FORCE_INLINE bool isEmptyLine(const std::string_view str) {
    return (str.empty() || str == "\n" || str == "\r" || str == "\r\n");
}

/**
 * Trims newline characters on the end of the string.
 * Ready to process line with "\n", "\r" or "\r\n".
 *
 * Note that this function modifies provided `string_view`.
 *
 * @param str -- string view, to be trimmed.
 */
CIRBO_OPT_FORCE_INLINE void trimNewlineCharacter(std::string_view* str) {
    if (str->empty()) {
        return;
    }

    if (str->back() == '\n') {
        str->remove_suffix(1);
    }

    if (str->back() == '\r') {
        str->remove_suffix(1);
    }
}

/**
 * Trims spaces from start and end of a `str`.
 * @param str -- string view, to be trimmed.
 */
CIRBO_OPT_FORCE_INLINE void trimSpaces(std::string_view* str)
{
    size_t const _b = str->find_first_not_of(" \t");
    size_t const _e = str->find_last_not_of(" \t");
    if (_b != std::string::npos)
    {
        *str = str->substr(_b, _e - _b + 1);
    }
    else if (_e != std::string::npos)
    {
        *str = str->substr(0, _e);
    }
    else
    {
        *str = str->substr(0, 0);
    }
}

}  // namespace cirbo::string_utils

#endif  // CIRBO_SEARCH_STRING_UTILS_HPP
