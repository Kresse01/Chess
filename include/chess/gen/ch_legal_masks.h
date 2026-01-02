#pragma once
/**
 * @file ch_legal_masks.h
 * @brief Compute per-square legal destination masks for a given side.
 *
 * This is a utility for analysis/debugging (and can also be used by GUI highlighting).
 * It returns a 64-entry array where:
 *   - per_square[from] is a bitboard of legal destination squares for the piece on 'from'
 *   - entries for empty squares are 0
 *
 * Implementation lives in src/gen/ch_legal_masks.cpp and uses:
 *   - pins + check state computation
 *   - king legality
 *   - non-king legalization
 */
#include "chess/core/ch_types.h"

namespace ch
{
    class Board; // forward declaration

    /**
     * @brief Legal destination masks per origin square.
     */
    struct LegalMasks
    {
        BB per_square[64]{}; ///< destinations for piece sitting on square i (0 if none)
    };

    /**
     * @brief Compute legal destination masks for every piece of @p side.
     *
     * Notes:
     *  - Only squares containing a piece of @p side will have non-zero masks.
     *  - King mask includes only *legal king moves* as defined by legal_king_moves().
     *    (Castling handling depends on how legal_king_moves is implemented.)
     */
    LegalMasks legal_masks_for_side(const Board& b, Color side);
} // namespace ch