#pragma once
#include "chess/core/ch_types.h"
#include <cctype>

namespace ch {
    inline int sq_from_str(const char* s)
    {
        // expects "a1".."h8"
        int f = std::tolower(static_cast<unsigned char>(s[0])) - 'a';
        int r = (s[1] - '1');
        return idx(f, r);
    }
}