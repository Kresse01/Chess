// src/gen/ch_king_legal.cpp
#include "chess/core/ch_board.h"
#include "chess/core/ch_types.h"
#include "chess/core/ch_square.h"     // for bit(sq), idx(f,r)
#include "chess/analysis/ch_attack.h" // attacks_from(board, color, kind, fromSq)

namespace ch {

// Explicit color flip (avoid ~side)
static inline Color opponent(Color c) {
    return (c == Color::White) ? Color::Black : Color::White;
}

// Attack query using your existing per-piece attack generator.
// We check whether ANY piece of 'by' attacks 'sq'.
static bool square_attacked(const Board& board, Color by, int sq) {
    const BB target = bit(sq);

    // Iterate each piece kind for 'by'
    for (int ki = 0; ki < 6; ++ki) {
        const PieceKind kind = static_cast<PieceKind>(ki);
        BB bbPieces = board.bb(by, kind);
        if (!bbPieces) continue;

        // Simple (portable) iterator over set bits
        for (int from = 0; from < 64; ++from) {
            if (bbPieces & bit(from)) {
                BB atk = attacks_from(board, by, kind, from);
                if (atk & target) return true;
            }
        }
    }
    return false;
}

// Return legal king destinations that are specific to CASTLING.
// (Normal king 1-step moves are handled elsewhere in your movegen;
// this function’s main job is to contribute castle targets G/C when legal.)
BB legal_king_moves(const Board& board, Color side) {
    BB out = 0;

    const int r  = (side == Color::White) ? 0 : 7;
    const int sqA = idx(0, r);
    const int sqB = idx(1, r);
    const int sqC = idx(2, r);
    const int sqD = idx(3, r);
    const int sqE = idx(4, r);
    const int sqF = idx(5, r);
    const int sqG = idx(6, r);
    const int sqH = idx(7, r);

    const BB empty = ~board.occ_all();
    const Color opp = opponent(side);

    // --- King-side castle (E -> G) ---
    if (board.castle_k(side)) {
        const bool pathEmpty = (empty & bit(sqF)) && (empty & bit(sqG));
        if (pathEmpty) {
            const bool safe =
                !square_attacked(board, opp, sqE) &&
                !square_attacked(board, opp, sqF) &&
                !square_attacked(board, opp, sqG);
            if (safe) out |= bit(sqG);
        }
    }

    // --- Queen-side castle (E -> C) ---
    if (board.castle_q(side)) {
        const bool pathEmpty = (empty & bit(sqD)) && (empty & bit(sqC)) && (empty & bit(sqB));
        if (pathEmpty) {
            const bool safe =
                !square_attacked(board, opp, sqE) &&
                !square_attacked(board, opp, sqD) &&
                !square_attacked(board, opp, sqC);
            if (safe) out |= bit(sqC);
        }
    }

    return out;
}

} // namespace ch