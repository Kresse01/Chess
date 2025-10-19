#include "chess/gen/ch_legalize.h"
#include "chess/core/ch_bitboard.h"
#include "chess/analysis/ch_attack.h"

namespace ch
{
    static inline BB white_pawns_attacking_to(int sq)
    {
        BB t = bit(sq);
        return ((t >> 7) & ~FILE_MASK[0]) | ((t >> 9) & ~FILE_MASK[7]);
    }

    static inline BB black_pawns_attacking_to(int sq)
    {
        BB t = bit(sq);
        return ((t << 9) & ~FILE_MASK[0]) | ((t << 7) & ~FILE_MASK[7]);
    }

    static bool king_safe_after_ep(const Board& b, Color side, int fromSq, int ep_to)
    {
        const Color them = opposite(side);
        BB kbb = b.bb(side, PieceKind::King);
        if (!kbb) return false;
        int ks = lsb(kbb);

        // Captured pawn square is behind EP target
        const int cap_sq = (side == Color::White) ? (ep_to - 8) : (ep_to + 8);

        BB occ = b.occ_all();
        occ &= ~bit(fromSq); // moving pawn leaves from
        occ &= ~bit(cap_sq); // captured pawn dissappears
        occ |= bit(ep_to); // our pawn lands on ep_to

        BB e_pawns = b.bb(them, PieceKind::Pawn) & ~bit(cap_sq);

        BB attackers = 0;
        attackers |= KNIGHT_ATK[ks] & b.bb(them, PieceKind::Knight);
        attackers |= KING_ATK[ks] & b.bb(them,PieceKind::King);
        if (them == Color::White) attackers |= white_pawns_attacking_to(ks) & e_pawns;
        else                      attackers |= black_pawns_attacking_to(ks) & e_pawns;

        // sliders using modified occupancy
        BB bishops = b.bb(them, PieceKind::Bishop);
        BB rooks = b.bb(them, PieceKind::Rook);
        BB queens = b.bb(them, PieceKind::Queen);

        BB diag = ray_attacks_from(ks, NE, occ) | ray_attacks_from(ks, NW, occ)
                | ray_attacks_from(ks, SE, occ) | ray_attacks_from(ks, SW, occ);
        BB ortho = ray_attacks_from(ks, N, occ) | ray_attacks_from(ks, S, occ)
                 | ray_attacks_from(ks, E, occ) | ray_attacks_from(ks, W, occ);

        attackers |= diag & (bishops | queens);
        attackers |= ortho& (rooks | queens);

        return attackers == 0;
    }

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

        // EP: keep it only if the king remains after EP
        if (kind == PieceKind::Pawn)
        {
            int ep = b.ep_target();
            if (ep != -1)
            {
                BB epb = bit(ep);
                if (pseudo & epb)
                {
                    if (!king_safe_after_ep(b, side, fromSq, ep))
                        pseudo &= ~epb;
                }
            }
        }

        return pseudo;
    }
}