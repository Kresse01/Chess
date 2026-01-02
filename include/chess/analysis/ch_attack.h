#pragma once
/**
 * @file ch_attack.h
 * @brief Attack / control queries used by king safety, checks, pins, and legality.
 *
 * This header is intentionally lightweight: it declares the public attack-query API.
 * Implementations are in src/analysis/ch_attack.cpp and depend on bitboard tables,
 * ray helpers, and per-piece geometry.
 */

#include "chess/core/ch_types.h"

namespace ch
{
    class Board; // forward declaration

    /**
     * @brief Return a bitboard of attackers (pieces of color @p by) that attack square @p sq.
     *
     * Convention: returned bits are the *squares of the attacking pieces*.
     */
    BB attackers_to(const Board& b, int sq, Color by);

    /**
     * @brief True if @p side's king is currently in check.
     */
    bool in_check(const Board& b, Color side);

    /**
     * @brief True if square @p sq is attacked by color @p by.
     */
    inline bool is_attacked(const Board& b, int sq, Color by)
    {
        return attackers_to(b, sq, by) != 0;
    }

    /**
     * @brief Squares attacked (controlled) by a *single piece* at @p fromSq.
     *
     * Notes:
     *  - Pawns: capture directions only (they "attack" diagonals, not pushes).
     *  - Castling is not an attack; en-passant target does not change attacks.
     */
    BB attacks_from(const Board& b, Color by, PieceKind kind, int fromSq);

    /**
     * @brief Union of all squares attacked by side @p by.
     */
    BB attacks_side(const Board& b, Color by);
} // namespace ch
