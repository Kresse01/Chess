#pragma once
/**
 * @file ch_kinght.h
 * @brief Knight movement masks via precomputed leaper table.
 * 
 * The knight ignores occupancy for geometry; we later intersect with
 *  - empty squares (Quiet)
 *  - enempy occupancy (Attacks)
 *  - inverse of own occupancy (All)
 */

 #include "chess/pieces/ch_piece.h"

 namespace ch
 {
    /**
     * @brief Knight movement mask from square @p s.
     * 
     * Use KNIGHT_ATK[s] from ch_bitboard.h; this geometry is independent of
     * occupancy. We then intersect depending on MovePhase.
     * 
     * @param c color of the knight (only affects which "own" occupancy to exclude)
     * @param s square index 0..63
     * @param b board (for occupancies)
     * @param phase Attacks / Quiet / All
     * @param o (unused for knights; retained for signature uniformity)
     * @return destination squares as a bitboard
     */
    inline BB move(knight_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts&)
    {
        BB atk = KNIGHT_ATK[fromSq];
        BB own = b.occ(c);
        BB opp = b.occ(opposite(c));

        // Never allow landing on own squares
        atk &= ~own;

        switch(phase) {
            case MovePhase::Quiet: return atk & ~opp; // empty only
            case MovePhase::Attacks: return atk & opp; // enemy only
            case MovePhase::All: return atk; // empty+enemy (not own)
        }
        return 0;
    }
 } // namespace ch
