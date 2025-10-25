#include "chess/core/ch_board.h"
#include "chess/core/ch_types.h"
#include "chess/core/ch_square.h"     // bit(sq), idx(f,r)
#include "chess/analysis/ch_attack.h" // attacks_from(board, color, kind, from)

namespace ch {

static inline Color opponent(Color c) {
    return (c == Color::White) ? Color::Black : Color::White;
}

static bool square_attacked(const Board& board, Color by, int sq) {
    const BB target = bit(sq);
    for (int ki = 0; ki < 6; ++ki) {
        const PieceKind kind = static_cast<PieceKind>(ki);
        BB bbPieces = board.bb(by, kind);
        if (!bbPieces) continue;
        for (int from = 0; from < 64; ++from) {
            if (bbPieces & bit(from)) {
                BB atk = attacks_from(board, by, kind, from);
                if (atk & target) return true;
            }
        }
    }
    return false;
}

BB legal_king_moves(const Board& board, Color side) {
    BB out = 0;

    // ---- (A) normal king steps ----
    // locate king square
    int ksq = -1;
    {
        BB kbb = board.bb(side, PieceKind::King);
        if (kbb) {
            for (int s = 0; s < 64; ++s) if (kbb & bit(s)) { ksq = s; break; }
        }
    }
    if (ksq >= 0) {
        const Color opp = opponent(side);
        const BB ownOcc = board.occ(side);

        // All pseudo-legal king steps from ksq:
        BB stepMask = attacks_from(board, side, PieceKind::King, ksq);
        // Remove own pieces
        stepMask &= ~ownOcc;

        // Filter out attacked squares
        for (int s = 0; s < 64; ++s) {
            if (stepMask & bit(s)) {
                if (!square_attacked(board, opp, s))
                    out |= bit(s);
            }
        }
    }

    // ---- (B) castling ----
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

    // King side: E -> G (F,G empty; E,F,G not attacked; rights set)
    if (board.castle_k(side)) {
        const bool pathEmpty = ( (empty & bit(sqF)) && (empty & bit(sqG)) );
        if (pathEmpty) {
            if (!square_attacked(board, opp, sqE) &&
                !square_attacked(board, opp, sqF) &&
                !square_attacked(board, opp, sqG)) {
                out |= bit(sqG);
            }
        }
    }

    // Queen side: E -> C (D,C,B empty; E,D,C not attacked; rights set)
    if (board.castle_q(side)) {
        const bool pathEmpty = ( (empty & bit(sqD)) &&
                                 (empty & bit(sqC)) &&
                                 (empty & bit(sqB)) );
        if (pathEmpty) {
            if (!square_attacked(board, opp, sqE) &&
                !square_attacked(board, opp, sqD) &&
                !square_attacked(board, opp, sqC)) {
                out |= bit(sqC);
            }
        }
    }

    return out;
}

} // namespace ch