#include "chess/gen/ch_legalize.h"
#include "chess/core/ch_bitboard.h"
#include "chess/analysis/ch_attack.h"

namespace ch
{
    BB legalize_nonking_mask(const Board& b,
                             BB pseudo,
                             int fromSq,
                             PieceKind kind,
                             Color side,
                             const Pins& pins,
                             const CheckState& cs)
    {
        // 1) Double check: only the king can move
        if (cs.double_check) return 0;

        // 2) If pinned, the piece may move along the closed king<->pinner segment
        if (pins.pinned & bit(fromSq))
        {
            pseudo &= pins.ray_to_enemy[fromSq];
        }

        // 3) If in single check, non-king moves must block or capture the checker
        //    compute_check_state() must set: block_or_capture = between(king, checker) U {checker}
        if (cs.in_check)
        {
            pseudo &= cs.block_or_capture;
        }

        return pseudo;
    }
}