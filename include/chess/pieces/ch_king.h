#pragma once
/**
 * @file ch_king.h
 * @brief King movement masks from precomputed table, with optional castling destinations.
 *
 * Important: This file is *geometry only*. It does NOT check whether destination
 * squares are attacked (king safety) and does NOT fully validate castling.
 * Full king legality (including castling constraints) is handled by:
 *   - chess/gen/ch_king_legal.*
 */

#include "chess/pieces/ch_piece.h"

namespace ch
{
    inline BB move(king_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o)
    {
        BB atk = KING_ATK[fromSq];
        const BB own = b.occ(c);
        const BB opp = b.occ(opposite(c));

        // Never land on own pieces
        atk &= ~own;

        // Optional: add castling destination squares as "potential king moves"
        // Only if king is on home square e1/e8.
        const int homeRank = (c == Color::White) ? 0 : 7;
        const int e = idx(4, homeRank);

        if (fromSq == e)
        {
            if (o.can_castle_k)
                atk |= bit(idx(6, homeRank)); // g1 / g8

            if (o.can_castle_q)
                atk |= bit(idx(2, homeRank)); // c1 / c8
        }

        switch (phase)
        {
            case MovePhase::Attacks: return atk & opp;
            case MovePhase::Quiet:   return atk & ~opp;
            case MovePhase::All:     return atk;
        }
        return 0;
    }
} // namespace ch