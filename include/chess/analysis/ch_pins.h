#pragma once
/**
 * @file ch_pins.h
 * @brief Compute line pins to the king for a side.
 *
 * A piece is "pinned" if moving it would expose the king to attack by an enemy
 * sliding piece (rook/bishop/queen) along a line (rank/file/diagonal).
 *
 * Pins are used by legality filtering to restrict pseudo-legal destinations.
 */

#include "chess/core/ch_types.h"

namespace ch
{
    class Board; // forward declaration

    /**
     * @brief Info about pieces line-pinned to their own king.
     *
     *  - pinned: bitboard of all friendly pieces (excluding king) that are pinned.
     *  - ray_to_enemy[sq]:
     *        For a pinned piece on 'sq', a *closed* segment bitboard:
     *            king ... pinned ... enemy_pinner
     *        Legal moves for that piece are constrained to this segment.
     *        For non-pinned squares, this entry is 0.
     */
    struct Pins
    {
        BB pinned = 0;
        BB ray_to_enemy[64]{}; // closed segment: king .. enemy (includes both endpoints)
    };

    /**
     * @brief Compute line pins for @p side.
     *
     * Only rook / bishop / queen pins are considered (line pins).
     * Knights and pawns cannot create line pins.
     */
    Pins compute_pins(const Board& b, Color side);
} // namespace ch
