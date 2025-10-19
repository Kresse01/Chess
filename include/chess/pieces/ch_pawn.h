#pragma once
/**
 * @file ch_pawn.h
 * @brief Pawn movement masks via directional shifts and masks.
 * 
 * White pawns move "up" the board (towards higher square indices) and black
 * pawns move "down" the board (towards lower indices) with our a1=0..h8=63 indexing.
 * 
 * Quiet pushes: single-step forward if empty; double-step from start rank
 * when both intermediate and destination are empty (shaped by MoveOpts).
 * 
 * Captures: diagonally forward (up-left/up-right for White, down-left/down-right
 * for Black) intersected with enemy occupancy. If MoveOpts::ep_sq >= 0, we also
 * include the en-passant target square if a diagonal reaches it.
 */

#include "chess/pieces/ch_piece.h"

namespace ch
{
    /** @brief Quiet push mask (single + optional double) from @p s. */
    inline BB pawn_quiet_mask(Color c, int s, const Board& b, const MoveOpts& o)
    {
        BB empty = ~b.occ_all();
        BB from = bit(s);
        if (c==Color::White)
        {
            BB one = (from << 8) & empty;
            BB two = (o.allow_double_push ? ((one<<8) & empty) & RANK_MASK[3] : 0); //land on rank 4
            return one | two;
        } else {
            BB one = (from >> 8) & empty;
            BB two = (o.allow_double_push ? ((one>>8) & empty) & RANK_MASK[4]: 0); //land on rank 5
            return one | two;
        }
    }

    /** @brief Capture mask (including optional en-passant target). */
    inline BB pawn_capture_mask(Color c, int s, const Board& b, const MoveOpts& o)
    {
        BB enemy = b.occ(opposite(c));
        BB from = bit(s);

        if (c == Color::White)
        {
            BB l = (from<<7) & ~FILE_MASK[7]; //up-left
            BB r = (from<<9) & ~FILE_MASK[0]; //up-right
            BB caps = (l|r) & enemy;
            //En-passant: allow capturing onto ep_sq if diagonally reachable
            if (o.ep_sq >= 0) caps |= (l | r) & bit(o.ep_sq);
            return caps;
        } else {
            BB l = (from>>9) & ~FILE_MASK[7]; //down-left
            BB r = (from>>7) & ~FILE_MASK[0]; //down-right
            BB caps = (l | r)& enemy;
            if (o.ep_sq >= 0) caps |= (l | r) & bit(o.ep_sq);
            return caps;
        }
    }

    /**
     * @brief Pawn movement mask for @p phase.
     * 
     *  - Quiet: single (+optional double) pushes to empty squares
     *  - Attacks: diagonal captures (and ep target if set)
     *  - All: union of the above
     */
    inline BB move(pawn_t, Color c, int s, const Board& b, MovePhase phase, const MoveOpts& o)
    {
        switch (phase)
        {
            case MovePhase::Quiet: return pawn_quiet_mask(c, s, b, o);
            case MovePhase::Attacks: return pawn_capture_mask(c, s, b, o);
            case MovePhase::All: return pawn_quiet_mask(c, s, b, o) | pawn_capture_mask(c, s, b, o);
        }
        return 0;
    }
}