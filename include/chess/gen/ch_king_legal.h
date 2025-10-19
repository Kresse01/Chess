#pragma once
#include "chess/analysis/ch_attack.h" // attacks_side
#include "chess/pieces/ch_piece.h"

namespace ch
{
    /**
     * @brief Legal destinations for the king (no castling): adjacent empty or captures
     * that are not controlled by the opponent.
     */
    BB legal_king_moves(const Board& b, Color side);
}