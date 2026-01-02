#pragma once
#include <cstdint>
#include "ch_types.h"

namespace ch
{
    // 16-bit move encoding (fits in registers, easy to copy):
    // [0..5] to (0..63)
    // [6..11] from (0..63)
    // [12..13] promo (0=none,1=Knight,2=Bishop,3=Rook,4==Queen -> we store 0..3 and map to piece)
    // [14] capture
    // [15] special (castle or en-passant) - further detail via helper flags if needed
    //
    // Note:
    // - We do not store "pawn double push" etc. in the move; make/unmake infers from squares
    // - Castling / EP are indicated by 'special' and recognized by board state + geometry.
    struct Move {
        std::uint16_t v{0};

        [[nodiscard]] constexpr int from() const noexcept { return (v >> 6) & 0x3F; }
        [[nodiscard]] constexpr int to() const noexcept { return v & 0x3f; }

        [[nodiscard]] constexpr bool is_capture() const noexcept { return ((v >> 14) & 1u) != 0; }
        [[nodiscard]] constexpr bool is_special() const noexcept { return ((v >> 15) & 1u) != 0; } // castle/ep

        // 0..3 -> N,B,R,Q (only meaningful for promotions; otherwise typically 0)
        [[nodiscard]] constexpr int promo_code() const noexcept { return (v >> 12) & 0x3; }

        [[nodiscard]] static constexpr Move make(int from, int to, bool capture=false, int promoCode=0, bool special=false) noexcept
        {
            Move m;
            m.v = static_cast<std::uint16_t>(
                (to & 0x3F)
                | ((from & 0x3F) << 6)
                | ((promoCode & 0x3) << 12)
                | (capture ? (1u<<14) : 0u)
                | (special ? (1u<<15) : 0u));
            return m; 
        }

        [[nodiscard]] friend constexpr bool operator==(Move a, Move b) noexcept { return a.v == b.v; }
        [[nodiscard]] friend constexpr bool operator!=(Move a, Move b) noexcept { return a.v != b.v; }
    };

    /// Map promo code (0..3) to a PieceKind (N,B,R,Q).
    [[nodiscard]] inline constexpr PieceKind promo_code_to_kind(std::uint8_t code) noexcept
    {
        // 0..3 -> N, B, R, Q
        constexpr PieceKind map[4] = {
            PieceKind::Knight,
            PieceKind::Bishop,
            PieceKind::Rook,
            PieceKind::Queen
        };
        return map[code & 3u];
    }
}