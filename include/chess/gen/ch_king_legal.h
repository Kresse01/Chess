#pragma once
/**
 * @file ch_king_legal.h
 * @brief Compute fully legal king destination squares (including castling if legal).
 *
 * This module enforces king safety:
 *  - King may not move into check.
 *  - Castling is included only if all castling constraints are satisfied.
 *
 * Implementation lives in src/gen/ch_king_legal.cpp and depends on:
 *  - attack queries
 *  - board state (castling flags, occupancy)
 */

#include "chess/core/ch_types.h"

namespace ch
{
    class Board; // forward declaration

    /**
     * @brief Return all legal destination squares for the king of @p side.
     *
     * The returned mask contains only squares the king may legally move to.
     * If castling is legal, the destination squares (g1/c1 or g8/c8) are included.
     */
    BB legal_king_moves(const Board& b, Color side);
} // namespace ch
