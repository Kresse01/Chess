#include "chess/analysis/ch_attack.h"

namespace ch
{
    // -- existing attackers_to / in_check you already added --

    static inline BB white_pawns_attacking_to(int sq)
    {
        BB tgt = bit(sq);
        BB l = (tgt >> 7) & ~FILE_MASK[0];
        BB r = (tgt >> 9) & ~FILE_MASK[7];
        return l | r;
    }

    static inline BB black_pawns_attacking_to(int sq)
    {
        BB tgt = bit(sq);
        BB l = (tgt << 9) & ~FILE_MASK[0];
        BB r = (tgt << 7) & ~FILE_MASK[7];
        return l | r;
    }

    BB attackers_to(const Board& b, int sq, Color by)
    {
        BB occ = b.occ_all();
        BB attackers = 0;

        // Knights / Kings
        attackers |= KNIGHT_ATK[sq] & b.bb(by, PieceKind::Knight);
        attackers |= KING_ATK[sq] & b.bb(by,PieceKind::King);

        if (by == Color::White) attackers |= white_pawns_attacking_to(sq) & b.bb(by, PieceKind::Pawn);
        else                    attackers |= black_pawns_attacking_to(sq) & b.bb(by, PieceKind::Pawn);

        // Sliders: ray from target outward; first blocker of the right type attacks sq
        BB bishop = b.bb(by, PieceKind::Bishop);
        BB rooks = b.bb(by, PieceKind::Rook);
        BB queens = b.bb(by, PieceKind::Queen);

        // Diagonals
        {
            BB rays = ray_attacks_from(sq, NE, occ) | ray_attacks_from(sq, NW, occ)
                    | ray_attacks_from(sq, SE, occ) | ray_attacks_from(sq, SW, occ);
            attackers |= rays & (bishop | queens);
        }

        // Orthogonals
        {
            BB rays = ray_attacks_from(sq, N, occ) | ray_attacks_from(sq, S, occ)
                    | ray_attacks_from(sq, E, occ) | ray_attacks_from(sq, W, occ);
            attackers |= rays & (rooks | queens);
        }

        return attackers;
    }

    bool in_check(const Board& b, Color side)
    {
        BB kbb = b.bb(side, PieceKind::King);
        if (!kbb) return false;
        int ks = lsb(kbb);
        return attackers_to(b, ks, opposite(side)) != 0;
    }

    // --------- NEW: attacks_from / attacks_side ------------------------
    BB attacks_from(const Board& b, Color by, PieceKind kind, int fromSq)
    {
        // For control/attacks, castling/EP don't matter; keep zeroed opts.

        MoveOpts o;
        switch (kind)
        {
            case PieceKind::Pawn:
                // Only the capture directions count as "attacks"
                return move(Pawn, by, fromSq, b, MovePhase::Attacks, o);
            case PieceKind::Knight:
                return move(Knight, by, fromSq, b, MovePhase::All, o);
            case PieceKind::Bishop:
                return move(Bishop, by, fromSq, b, MovePhase::All, o);
            case PieceKind::Rook:
                return move(Rook, by, fromSq, b, MovePhase::All, o);
            case PieceKind::Queen:
                return move(Queen, by, fromSq, b, MovePhase::All, o);
            case PieceKind::King:
                // King attacks = adjacent squares; do NOT include castling
                return KING_ATK[fromSq] & ~b.occ(by);
        }
        return 0;
    }

    BB attacks_side(const Board& b, Color by)
    {
        BB all = 0;
        //Knights
        for (BB pcs = b.bb(by,PieceKind::Knight); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Knight, s);
        }
        //Bishops
        for (BB pcs = b.bb(by, PieceKind::Bishop); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Bishop, s);
        }
        // Rooks
        for (BB pcs = b.bb(by, PieceKind::Rook); pcs; )
        {
            int s = lsb(pcs); pcs^= bit(s);
            all |= attacks_from(b, by, PieceKind::Rook, s);
        }
        // Queens
        for (BB pcs = b.bb(by, PieceKind::Queen); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Queen, s);
        }
        // Pawns (capture directions only)
        for (BB pcs = b.bb(by, PieceKind::Pawn); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Pawn, s);
        }
        // King (no castling)
        {
            BB k = b.bb(by, PieceKind::King);
            if (k) {
                int s = lsb(k);
                all |= attacks_from(b, by, PieceKind::King, s);
            }
        }
        return all;
    }
}