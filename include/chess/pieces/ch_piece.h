#pragma once
/**
 * @file ch_piece.h
 * @brief Unified movement API (bitboards) with per-piece implementation.
 * 
 * This file exposes a single uniform function:
 * 
 *    BB move(PieceKind k, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
 * 
 * And also compile-time, tag-dispatch overloads for each piece:
 * 
 *    BB move(pawn_t, Color, int, const Board&, MovePhase, const MoveOpts&);
 *    BB move(knight_t, Color, int, const Board&, MovePhase, const MoveOpts&);
 *    ...
 * 
 * The design uses *static polymorphism* (tag dispatch), not virtual inheritance,
 * to keep the hot path inlinable and branch-light. Each piece's movement rules
 * are expressed entirely as **bitwise operations** on bitboards.
 */

#include "chess/core/ch_board.h"
#include "chess/core/ch_bitboard.h"

namespace ch
{
    /**
     * @name Piece tags (compile-time dispatch)
     * These types exist solely to selec the correct overload of 'move()'.
     * @{
     */
    struct pawn_t{}; struct knight_t{}; struct bishop_t{};
    struct rook_t{}; struct queen_t{}; struct king_t{};

    inline constexpr pawn_t Pawn{};
    inline constexpr knight_t Knight{};
    inline constexpr bishop_t Bishop{};
    inline constexpr rook_t Rook{};
    inline constexpr queen_t Queen{};
    inline constexpr king_t King{};
    /** @} */

    /**
     * @brief Selects which destination to include in the returned mask.
     * 
     *  - Attacks: only squares currently occupied by the opponent (captures).
     *  - Quiet: only empty squares (non-capture).
     *  - All: union of both; equivalent to '~ownOCC' intersection with geometry.
     */
    enum class MovePhase : uint8_t { Attacks, Quiet, All };

    /**
     * @brief Contextual options that *shape* movement masks (no side effects).
     * 
     * These flags let you include context-sensitive moves (EP, castling, double push)
     * without performing full legality checking or state mutation.
     */
    struct MoveOpts
    {
        /**
         * @brief En-passant target square index (0..63) or -1 if none.
         * When >= 0, pawn capture masks will include this square if the
         * diagonal shift reaches it.
         */
        int ep_sq = -1;

        /**
         * @brief Whether king-side / queen-side castling destinations should be
         * included in the king's *quiet* mask (assuming empty destination squares).
         * Note: checking "squares not under attack" is intentionally left to a legality layer,
         * not this geometry layer
         */
        bool can_castle_k = false;
        bool can_castle_q = false;

        /**
         * @brief Include pawn double pushes from start rank.
         * If false, only single-step quiet pushes are produced.
         */
        bool allow_double_push = true;
    };

    /**
     * @name Per-piece move() overloads (declarations)
     * 
     * Each overload returns a bitboard of destination squares for the piece
     * on 'fromSq', considering the current occupancy in 'board' and shaping with 'MoveOpts'.
     * 
     * @param tag piece tag (Pawn, Knight, ...)
     * @param c color of the moving side
     * @param fromSq starting square index 0..63
     * @param b board state (bitboards+flags)
     * @param phase Attacks / Quiet / All
     * @param o movement shaping options (EP/castling/double push)
     * @return bitboard of destination squares matching @p phase
     * @{
     */
    BB move(pawn_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
    BB move(knight_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
    BB move(bishop_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
    BB move(rook_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
    BB move(queen_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
    BB move(king_t, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
    /** @} */

    /**
     * @brief Runtime dispatcher by PieceKind (thin switch that forwards to tag overloads).
     * 
     * Use this when you have a @ref PieceKind at runtime. The heavy lifting stays
     * in the tag overloads so inlining still applies when the kind is known
     */
    BB move(PieceKind k, Color c, int fromSq, const Board& b, MovePhase phase, const MoveOpts& o);
}