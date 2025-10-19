#pragma once
/**
 * @file ch_king.h
 * @brief King movement masks via precomputed leaper table and optional castling shape.
 * 
 * The king's "quiet" mask can optionally include the castling destination squares
 * (g1/c1 for white, g8/c8 for Black) if MovOpts indicates rights. This header
 * *only shapes* those destinations; checks such as "squares not under attack" and
 * "path is clear" can be applied by a legality layer if desired
 */

 #include "chess/pieces/ch_piece.h"

 namespace ch
 {
    /** @brief Non-capturing king moves (adjacent empty squares). */
    inline BB king_quiet(Color c, int s, const Board& b)
    {
        return KING_ATK[s] & ~b.occ_all();
    }

    /** @brief Capturing king moves (adjacent enemy-occupied squares). */
    inline BB king_attacks(Color c, int s, const Board& b)
    {
        return KING_ATK[s] & b.occ(opposite(c));
    }

    /**
     * @brief Candidate castling destination squares (king target only).
     * 
     * This function *does not* verify rook presence, path clearance, or check
     * conditions. It merely returns the destination squares for the king if the
     * corresponding MoveOpts flags are set and the destination is empty.
     */
    inline BB castle_mask(Color c, const Board& b, const MoveOpts& o)
    {
        BB m = 0;
        if (c==Color::White)
        {
            if (o.can_castle_k) m |= bit(6); // g1
            if (o.can_castle_q) m |= bit(2); // c1
        } else {
            if (o.can_castle_k) m |= bit(62); // g8
            if (o.can_castle_q) m |= bit(58); // c8
        }
        //Ensure destination squares are themselves empty (path checks optional elsewhere)
        return m & ~b.occ_all();
    }

    /**
     * @brief King movement mask from @p s to @p c
     * 
     *  - Quiet: adjacent empty squares + optional castle destinations
     *  - Attacks: adjecent enemy-occupied squares
     *  - All: union of the above
     */
    inline BB move(king_t, Color c, int s, const Board& b, MovePhase phase, const MoveOpts& o)
    {
        switch (phase)
        {
            case MovePhase::Quiet: return king_quiet(c, s, b) | castle_mask(c, b, o);
            case MovePhase::Attacks: return king_attacks(c, s, b);
            case MovePhase::All: return king_quiet(c, s, b) | king_attacks(c, s, b) | castle_mask(c, b, o);
        }
        return 0;
    }
 }