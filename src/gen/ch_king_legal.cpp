#include "chess/gen/ch_king_legal.h"
#include "chess/core/ch_bitboard.h"

namespace ch
{
    
    BB legal_king_moves(const Board& b, Color side)
    {
        BB kbb = b.bb(side, PieceKind::King);
        if(!kbb) return 0;
        int ks = lsb(kbb);

        // Geometric king moves (no castling)
        BB geo = KING_ATK[ks] & ~b.occ(side);

        // Squares controlled by the opponent
        BB enemy_ctrl = attacks_side(b, opposite(side));

        BB legal = geo & ~enemy_ctrl;

        // --- Castling ---
        const int r = (side == Color::White) ? 0 : 7;
        const int e = idx(4, r);
        const int f = idx(5, r);
        const int g = idx(6, r);
        const int d = idx(3, r);
        const int c = idx(2, r);
        const int b = idx(1, r);
        const int a = idx(0, r);

        // Don't allow castling if king not on starting square
        if (ks == e)
        {
            auto empty = b.occ_all();

            // King side: squares, f, g must be empty; e, f, g not attacked
            if (b.castle_k(side))
            {
                if (!(empty & (bit(f)|bit(g))) && !(enemy_ctrl & (bit(e)|bit(f)|bit(g))))
                {
                    legal |= bit(g);
                }
            }

            // Queen side: squares d, c, (and usually b) empty; e, d, c not attacked
            if (b.castle_q(side))
            {
                if (!(empty & (bit(d)|bit(c)|bit(d))) && !(enemy_ctrl & (bit(e)|bit(d)|bit(c))))
                {
                    legal |= bit(c);
                }
            }
        }

        // legal = geometric minus enemy control
        return legal;
    }
}