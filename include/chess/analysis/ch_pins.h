#pragma once
#include "chess/core/ch_board.h"
#include "chess/pieces/ch_piece.h"

namespace ch
{
    /**
     * @brief Info about pieces line-pinned to their own king.
     *  - pinned: all friendly pieces (excluding king) that are pinned.
     *  - ray_to_enemy[sq]: for a pinned piece on 'sq', the *closest* ray squares between
     *    the king and the pinning enemy inclusive (i.e., legal movement of that pinned
     *    piece is constrained to this line). Empty for non-pinned squares.
     */

     struct Pins {
        BB pinned = 0;
        BB ray_to_enemy[64]{}; // closed segment: king .. enemy (includes both endpoints)
     };

     /**
      * @brief Compute line pins for 'side'. Only rook / bishop / queen pins are considered.
      * Knights and pawns cannot pin along lines
      */
     Pins compute_pins(const Board& b, Color side);
}