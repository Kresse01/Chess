#pragma once
/**
 * @file ch_legalize.h
 * @brief Filter pseudo-legal destination masks into legal destination masks.
 *
 * This module applies king-safety constraints to non-king moves:
 *  - If in check: restrict to block/capture squares (unless double check)
 *  - If pinned: restrict movement to the pin ray segment
 *  - Special EP case: ensure king is safe after EP capture
 *
 * Implementation lives in src/gen/ch_legalize.cpp
 */

#include "chess/core/ch_types.h"
#include "chess/analysis/ch_pins.h"
#include "chess/analysis/ch_legality.h"

namespace ch
{
    class Board; // forward declaration

    /**
     * @brief Filter a pseudo-legal destination mask for a non-king piece.
     *
     * @param b      current board
     * @param pseudo pseudo destination mask from geometry (already excludes own squares)
     * @param from   origin square of the piece
     * @param kind   kind of the moving piece (Pawn/Knight/.. but not King)
     * @param side   moving side
     * @param pins   pin information for side
     * @param cs     check information for side
     * @return bitboard of legal destinations for that piece from @p from
     */
    BB legalize_nonking_mask(
        const Board& b,
        BB pseudo,
        int from,
        PieceKind kind,
        Color side,
        const Pins& pins,
        const CheckState& cs
    );
} // namespace ch