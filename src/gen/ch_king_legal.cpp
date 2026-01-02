#include "chess/gen/ch_king_legal.h"

#include "chess/core/ch_board.h"
#include "chess/core/ch_bitboard.h"
#include "chess/analysis/ch_attack.h"

namespace ch
{
    static inline int king_home_square(Color side) noexcept
    {
        const int r = (side == Color::White) ? 0 : 7;
        return idx(4, r); // e1/e8
    }

    static inline bool rook_on_square(const Board& b, Color side, int sq) noexcept
    {
        return (b.bb(side, PieceKind::Rook) & bit(sq)) != 0;
    }

    BB legal_king_moves(const Board& b, Color side)
    {
        const Color them = opposite(side);

        const BB kbb = b.bb(side, PieceKind::King);
        if (!kbb) return 0;

        const int ks = lsb(kbb);

        // 1) Normal king steps (geometry), excluding own-occupied.
        BB moves = KING_ATK[ks] & ~b.occ(side);

        // 2) Filter out squares attacked by the opponent.
        BB legal = 0;
        for (BB m = moves; m; )
        {
            const int to = lsb(m);
            m ^= bit(to);

            Board tmp = b;

            // If king captures something on 'to', remove it
            tmp.clear_square(to);

            // Move king
            tmp.clear_piece(side, PieceKind::King, ks);
            tmp.set_piece(side, PieceKind::King, to);

            if (!is_attacked(tmp, to, them))
                legal |= bit(to);
        }

        // 3) Castling (fully legal):
        // Requirements (standard):
        //  - King is on e1/e8
        //  - Not currently in check
        //  - Squares between are empty
        //  - Squares king crosses and lands on are not attacked
        //  - Castling right flag is present
        //  - (Optional but good) rook exists on the expected corner square
        //
        // We add destination square (g1/c1 or g8/c8) to the returned mask if legal.

        if (ks == king_home_square(side) && !is_attacked(b, ks, them))
        {
            const int r = (side == Color::White) ? 0 : 7;

            // King-side: e -> g, rook h -> f
            if (b.castle_k(side))
            {
                const int f = idx(5, r);
                const int g = idx(6, r);
                const int h = idx(7, r);

                const BB empty_needed = bit(f) | bit(g);
                const bool empty_ok = (b.occ_all() & empty_needed) == 0;

                const bool rook_ok = rook_on_square(b, side, h);

                if (empty_ok && rook_ok)
                {
                    // Squares king traverses: f and g must not be attacked
                    if (!is_attacked(b, f, them) && !is_attacked(b, g, them))
                        legal |= bit(g);
                }
            }

            // Queen-side: e -> c, rook a -> d
            if (b.castle_q(side))
            {
                const int d = idx(3, r);
                const int c = idx(2, r);
                const int bq = idx(1, r);
                const int a = idx(0, r);

                // Between squares must be empty: d, c, b
                const BB empty_needed = bit(d) | bit(c) | bit(bq);
                const bool empty_ok = (b.occ_all() & empty_needed) == 0;

                const bool rook_ok = rook_on_square(b, side, a);

                if (empty_ok && rook_ok)
                {
                    // Squares king traverses: d and c must not be attacked
                    if (!is_attacked(b, d, them) && !is_attacked(b, c, them))
                        legal |= bit(c);
                }
            }
        }

        return legal;
    }
} // namespace ch