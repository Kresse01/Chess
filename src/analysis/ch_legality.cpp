#include "chess/analysis/ch_legality.h"

#include "chess/core/ch_board.h"
#include "chess/core/ch_bitboard.h"
#include "chess/analysis/ch_attack.h"

namespace ch
{
    CheckState compute_check_state(const Board& b, Color side)
    {
        CheckState cs{};

        BB kbb = b.bb(side, PieceKind::King);
        if(!kbb) return cs; // degenerate

        cs.king_sq = lsb(kbb);

        // who attacks our king?
        BB checkers = attackers_to(b,cs.king_sq, opposite(side));
        const int n = popcount(checkers);
        cs.in_check = (n > 0);
        cs.double_check = (n >= 2);

        if (n == 1)
        {
            cs.checker_sq = lsb(checkers);
            
            // Squares that either capture the checker or block the checking line
            cs.block_mask =
                between_mask(cs.king_sq, cs.checker_sq)
                | bit(cs.checker_sq);
        }
        return cs;
    }
} // namespace ch