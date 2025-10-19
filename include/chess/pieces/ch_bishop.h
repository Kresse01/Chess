#pragma once
/**
 * @file ch_bishop.h
 * @brief Bishop movement masks using directional rays.
 * 
 * We form a "span" by stepping along NE, NW, SE, SW until an edge or blocker.
 * The span includes the first blocker (to represent a capture), then we intersect
 * it with empty/enemy/own to produce Quite/Attacks/All.
 */

#include "chess/pieces/ch_piece.h"

namespace ch
{
    /** @brief Helper: full diagonal span from @p s including first blockers. */
    inline BB bishop_span(int s, BB occ)
    {
        return ray_attacks_from(s, NE, occ)
             | ray_attacks_from(s, NW, occ)
             | ray_attacks_from(s, SE, occ)
             | ray_attacks_from(s, SW, occ);
    }

    /**
     * @brief Bishop movement mask from @p s for @p c.
     */
    inline BB move(bishop_t, Color c, int s, const Board& b, MovePhase phase, const MoveOpts&)
    {
        BB span = bishop_span(s, b.occ_all());
        BB own = b.occ(c);
        BB all = b.occ_all();
        BB enn = b.occ(opposite(c));

        switch (phase)
        {
            case MovePhase::Quiet: return span & ~all;
            case MovePhase::Attacks: return span & enn;
            case MovePhase::All: return span & ~own;
        }
        return 0;
    }
}