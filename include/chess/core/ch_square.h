#pragma once
#include "chess/core/ch_types.h"

#include <cctype>
#include <string>

namespace ch
{
    /// Return true if sq is valid board square index [0..63].
    [[nodiscard]] inline constexpr bool is_valid_sq(int sq) noexcept
    {
        return static_cast<unsigned>(sq) < 64u;
    }
    
    /**
    * @brief Parse "a1".."h8" into a square index [0..63].
    *
    * This is a "fast" helper and assumes the input is valid.
    * If you want safety, use try_sq_from_str().
    */
    [[nodiscard]] inline int sq_from_str(const char* s)
    {
        int f = std::tolower(static_cast<unsigned char>(s[0])) - 'a';
        int r = (s[1] - '1');
        return idx(f, r);
    }

    /**
     * @brief Safe parse of "a1".."h8". Returns true on sucess
     */
    [[nodiscard]] inline bool try_sq_from_str(const char* s, int& out_sq) noexcept
    {
        if (!s) return false;

        const unsigned char c0 = static_cast<unsigned char>(s[0]);
        const unsigned char c1 = static_cast<unsigned char>(s[1]);

        if (c0 == 0 || c1 == 0) return false;

        const int f = std::tolower(c0) - 'a';
        const int r = c1 - '1';

        if (static_cast<unsigned>(f) >= 8u) return false;
        if (static_cast<unsigned>(r) >= 8u) return false;

        out_sq = idx(f, r);
        return true;
    }

    /**
     * @brief Convert square index [0..63] to algebraic "a1".."h8"
     * If sq is invalid returns "--".
     */
    [[nodiscard]] inline std::string sq_to_str(int sq)
    {
        if (!is_valid_sq(sq)) return "--";
        const int f = sq & 7;
        const int r = sq >> 3;
        std::string s;
        s.resize(2);
        s[0] = static_cast<char>('a' + f);
        s[1] = static_cast<char>('1' + r);
        return s;
    }
} // namespace ch