#pragma once
/**
 * @file ch_queen.h
 * @brief Queen sliding movement masks using ray attacks (rook + bishop directions).
 */

#include "chess/pieces/ch_piece.h"

namespace ch
{
    inline BB move(queen_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts&)
    {
        const BB occ_all = b.occ_all();

        BB atk = 0;
        // Rook directions
        atk |= ray_attacks_from(fromSq, N, occ_all);
        atk |= ray_attacks_from(fromSq, S, occ_all);
        atk |= ray_attacks_from(fromSq, E, occ_all);
        atk |= ray_attacks_from(fromSq, W, occ_all);
        // Bishop directions
        atk |= ray_attacks_from(fromSq, NE, occ_all);
        atk |= ray_attacks_from(fromSq, NW, occ_all);
        atk |= ray_attacks_from(fromSq, SE, occ_all);
        atk |= ray_attacks_from(fromSq, SW, occ_all);

        const BB own = b.occ(c);
        const BB opp = b.occ(opposite(c));

        atk &= ~own;

        switch (phase)
        {
            case MovePhase::Attacks: return atk & opp;
            case MovePhase::Quiet:   return atk & ~opp;
            case MovePhase::All:     return atk;
        }
        return 0;
    }
} // namespace ch