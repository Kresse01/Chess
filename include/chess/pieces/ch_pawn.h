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

        if (c == Color::White)
        {
            BB one = (from << 8) & empty;

            BB two = 0;
            if (o.allow_double_push && (s >> 3) == 1)
                two = (one << 8) & empty;
            return one | two;
        }
        else
        {
            const BB one = (from >> 8) & empty;

            BB two = 0;
            if (o.allow_double_push && (s >> 3) == 6)
                two = (one >> 8) & empty;
            return one | two;
        }
    }

    /** @brief Capture mask (including optional en-passant target). */
    inline BB pawn_capture_mask(Color c, int s, const Board& b, const MoveOpts& o)
    {
        BB enemy = b.occ(opposite(c));
        BB from = bit(s);

        // IMPORTANT: mask source file BEFORE shifting to avoid wrap-around.
        BB diagL = 0;
        BB diagR = 0;   

        if (c == Color::White)
        {
            // up-left: +7 (must not start on file A)
            diagL = ((from & ~FILE_MASK[0]) << 7);
            // up-right: +9 (must not start on file H)
            diagR = ((from & ~FILE_MASK[7]) << 9);
        }
        else
        {
            // down-left: -9 (must not start on file A)
            diagL = ((from & ~FILE_MASK[0]) >> 9);
            // down-right: -7 (must not start on file H)
            diagR = ((from & ~FILE_MASK[7]) >> 7);
        }
        
        BB caps = (diagL | diagR) & enemy;

        // En-passant: allow capturing onto ep_sq if diagonally reachable
        if (o.ep_sq >= 0)
            caps |= (diagL | diagR) & bit(o.ep_sq);

        return caps;
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
} // namespace ch