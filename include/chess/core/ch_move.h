#pragma once
#include <cstdint>
#include "ch_types.h"

namespace ch
{
    // 16-bit move encoding (fits in registers, easy to copy):
    // [0..5] to (0..63)
    // [6..11] to (0..63)
    // [12..13] promo (0=none,1=Knight,2=Bishop,3=Rook,4==Queen -> we store 0..3 and map to piece)
    // [14] capture
    // [15] special (castle or en-passant) - further detail via helper flags if needed
    struct Move {
        uint16_t v{};
        int from() const { return (v >> 6) & 0x3F; }
        int to() const { return v & 0x3f; }
        bool is_capture() const { return (v >> 14) & 1; }
        bool is_special() const { return (v >> 15) & 1; } // castle/ep
        int promo_code() const { return (v >> 12) & 0x3; } // 0..3 -> N,B,R,Q via helper

        static Move make(int from, int to, bool cap=false, int promoCode=0, bool special=false)
        {
            Move m;
            m.v = (to & 0x3F)
                | ((from & 0x3F) <<6)
                | ((promoCode & 0x3) << 12)
                | (cap ? (1u<<14) : 0)
                | (special ? (1u<<15) : 0);
            return m; 
        }
    };

    inline PieceKind promo_code_to_kind(int c)
    {
        // 0..3 -> N, B, R, Q
        static constexpr PieceKind map[4] = { PieceKind::Knight, PieceKind::Bishop, PieceKind::Rook, PieceKind::Queen };
        return map[c & 3];
    }
}