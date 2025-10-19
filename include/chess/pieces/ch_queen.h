#pragma once
/**
 * @file ch_queen.h
 * @brief Queen movement masks by composing bishop+rook spans.
 */

#include "chess/pieces/ch_bishop.h"
#include "chess/pieces/ch_rook.h"

namespace ch
{
    /**
     * @brief Queen movement mask = rook_span | bishop_span, then intersect per phase.
     */
    inline BB move(queen_t, Color c, int s, const Board& b, MovePhase phase, const MoveOpts& o)
    {
        BB span = bishop_span(s,b.occ_all()) | rook_span(s,b.occ_all());
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