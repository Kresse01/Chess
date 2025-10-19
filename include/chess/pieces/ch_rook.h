#pragma once
/**
 * @file ch_rook.h
 * @brief Rook movement masks using orthogonal rays
 */

#include "chess/pieces/ch_piece.h"

namespace ch
{
    /** @brief Helper: file+rank span from @p s including first blockers. */
    inline BB rook_span(int s, BB occ)
    {
        return ray_attacks_from(s, N, occ)
             | ray_attacks_from(s, S, occ)
             | ray_attacks_from(s, E, occ)
             | ray_attacks_from(s, W, occ);
    }

    /**
     * @brief Rook movement mask from @p s for @p c.
     */
    inline BB move(rook_t, Color c, int s, const Board& b, MovePhase phase, const MoveOpts&)
    {
        BB span = rook_span(s, b.occ_all());
        BB own = b.occ(c);
        BB all = b.occ_all();
        BB enn = b.occ(opposite(c));

        switch(phase)
        {
            case MovePhase::Quiet: return span & ~all;
            case MovePhase::Attacks: return span & enn;
            case MovePhase::All: return span & ~own;
        }
        return 0;
    }
}