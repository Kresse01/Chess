#pragma once
/**
 * @file ch_bishop.h
 * @brief Bishop movement masks using ray attacks.
 * 
 * Sliding piece logic:
 *  - Generate attack rays in each diagonal direction until a blocker.
 *  - The ray helper includes the blocker square (so captures are present).
 *  - Filter out own occupancy.
 *  - Then restrict to attacks-only / quiet-only depending on MovePhase.
 */

#include "chess/pieces/ch_piece.h"

namespace ch
{
    inline BB move(bishop_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts&)
    {
        const BB occ_all = b.occ_all();

        BB atk = 0;
        atk |= ray_attacks_from(fromSq, NE, occ_all);
        atk |= ray_attacks_from(fromSq, NW, occ_all);
        atk |= ray_attacks_from(fromSq, SE, occ_all);
        atk |= ray_attacks_from(fromSq, SW, occ_all);

        const BB own = b.occ(c);
        const BB opp = b.occ(opposite(c));

        // Never land on own pieces
        atk &= ~own;

        switch (phase)
        {
            case MovePhase::Attacks: return atk & opp;
            case MovePhase::Quiet:   return atk & ~opp; // empty squares after removing own
            case MovePhase::All:     return atk;
        }
        return 0;
    }
} // namespace ch