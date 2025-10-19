#include "chess/analysis/ch_legality.h"

namespace ch
{
    CheckState compute_check_state(const Board& b, Color side)
    {
        CheckState cs{};

        BB kbb = b.bb(side, PieceKind::King);
        if(!kbb) return cs; // degenerate
        cs.our_king_sq = lsb(kbb);

        // who attacks our king?
        cs.checkers = attackers_to(b, cs.our_king_sq, opposite(side));
        int n = popcount(cs.checkers);
        cs.in_check = (n > 0);
        cs.double_check = (n >= 2);

        if (n == 1)
        {
            int chk_sq = lsb(cs.checkers);
            // resolve squares = checker square plus squares between checker and king
            BB between = between_mask(cs.our_king_sq, chk_sq);
            cs.block_or_capture = between | bit(chk_sq);
        }
        return cs;
    }
}