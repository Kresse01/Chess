#pragma once
#include "chess/analysis/ch_attack.h" // attacks_side
#include "chess/pieces/ch_piece.h"

namespace ch
{
    /**
     * @brief Legal destinations for the king (no castling): adjacent empty or captures
     * that are not controlled by the opponent.
     */
    inline BB legal_king_moves(const Board& b, Color side)
    {
        BB kbb = b.bb(side, PieceKind::King);
        if(!kbb) return 0;
        int ks = lsb(kbb);

        // Geometric king moves (no castling)
        BB geo = KING_ATK[ks] & ~b.occ(side);

        // Squares controlled by the opponent
        BB enemy_ctrl = attacks_side(b, opposite(side));

        // legal = geometric minus enemy control
        return geo & ~enemy_ctrl;
    }
}