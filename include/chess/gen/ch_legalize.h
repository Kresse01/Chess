#pragma once
/**
 * @file ch_legalize.h
 * @brief Mask-level legality filtering for non-king pieces
 * 
 * Input:
 *  - pseudo: a pseudo-legal destination mask for a single piece (fromSq, kind, side),
 *            computed e.g., with move(tag,...,MovePhase::All).
 *  - pins: results of compute_pins(b, side)
 *  - cs: results of compute_check_state(b, side)
 * 
 * Output:
 *  - A mask containing only *legal* destination for that piece (no make/unmake).
 * 
 * Rules enforced:
 *  - Double check -> only king may move (returns 0)
 *  - Pin -> must stay on the king<->pinner ray (or capture the pinner on that ray).
 *  - Single check -> must block the checking ray or capture the checker.
 *  - En passant -> only if, after the EP capture, our king is not attacked.
 */
#include "chess/core/ch_board.h"
#include "chess/analysis/ch_pins.h"
#include "chess/analysis/ch_legality.h"
#include "chess/pieces/ch_piece.h"
#include "chess/core/ch_types.h"

namespace ch
{
    /**
     * @brief Given a pseudo-legal, mask for non-king piece, restrict it to legal destinations
     * by applying:
     *  - double-check rule,
     *  - single-check block/capture mask,
     *  - pin constraint (stay on king<->pinner ray).
     */
    BB legalize_nonking_mask(const Board& b,
                             BB pseudo,
                             int fromSq,
                             PieceKind kind,
                             Color side,
                             const Pins& pins,
                             const CheckState& cs);
}