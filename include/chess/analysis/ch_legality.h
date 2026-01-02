#pragma once
/**
 * @file ch_legality.h
 * @brief King-safety and check-related legality helpers.
 *
 * This module provides:
 *  - CheckState computation (in-check, double-check, checker square, block mask)
 *  - Helpers needed by move legalization layers (including EP king-safety)
 *
 * Implementations live in src/analysis/ch_legality.cpp
 */

#include "chess/core/ch_types.h"
#include "chess/core/ch_move.h"

namespace ch
{
    class Board; // forward declaration

    /**
     * @brief Summary of check status against a side's king.
     *
     * Typical usage:
     *  - If double_check == true: only king moves are legal.
     *  - If in_check == true: non-king moves must either capture the checker
     *    or block the checking line (block_mask).
     */
    struct CheckState
    {
        bool in_check = false;
        bool double_check = false;

        int king_sq = -1;       ///< side's king square
        int checker_sq = -1;    ///< square of a checker (valid when in_check)

        BB block_mask = 0;      ///< squares that block the check line (incl checker for capture)
    };

    /**
     * @brief Compute check status for @p side.
     */
    CheckState compute_check_state(const Board& b, Color side);

    /**
     * @brief Specialized check for EP captures: ensures king is not left in check after EP.
     *
     * EP is tricky because the captured pawn is not on the destination square.
     * This helper is typically used by the legalization layer when considering an EP move.
     *
     * @param b     position before EP
     * @param side  moving side
     * @param from  pawn origin square
     * @param to    EP destination square
     * @return true if king is safe after the EP capture is applied
     */
    //bool king_safe_after_ep(const Board& b, Color side, int from, int to);

    /**
     * @brief True if @p side is currently in checkmate.
     *
     * (Optional utility; keep if you already use it.)
     */
    //bool is_checkmate(const Board& b, Color side);

    /**
     * @brief True if @p side is currently stalemated.
     *
     * (Optional utility; keep if you already use it.)
     */
    //bool is_stalemate(const Board& b, Color side);
} // namespace ch
