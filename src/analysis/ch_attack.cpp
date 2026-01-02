#include "chess/analysis/ch_attack.h"

#include "chess/core/ch_board.h"
#include "chess/core/ch_bitboard.h"
#include "chess/pieces/ch_piece.h" // move(...), MoveOpts, tags

namespace ch
{
    namespace
    {
        // Return squares (as a bitboard) where a WHITE pawn would stand to attack 'sq'.
        // White pawn attacks are +7/+9, so attackers are at sq-7 and sq-9.
        inline BB white_pawns_attacking_to(int sq) noexcept
        {
            const BB tgt = bit(sq);
            const BB l = (tgt >> 7) & ~FILE_MASK[0]; // remove wrap source on file A
            const BB r = (tgt >> 9) & ~FILE_MASK[7]; // remove wrap source on file H
            return l | r;
        }

        // Return squares (as a bitboard) where a BLACK pawn would stand to attack 'sq'.
        // Black pawn attacks are -7/-9, so attackers are at sq+7 and sq+9.
        inline BB black_pawns_attacking_to(int sq) noexcept
        {
            const BB tgt = bit(sq);
            const BB l = (tgt << 9) & ~FILE_MASK[0]; // remove wrap source on file A
            const BB r = (tgt << 7) & ~FILE_MASK[7]; // remove wrap source on file H
            return l | r;
        }
    } // namespace

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

            case PieceKind::None:
            default:
                return 0;
        }
    }

    BB attacks_side(const Board& b, Color by)
    {
        BB all = 0;
        
        for (BB pcs = b.bb(by,PieceKind::Knight); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Knight, s);
        }
        
        for (BB pcs = b.bb(by, PieceKind::Bishop); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Bishop, s);
        }
        
        for (BB pcs = b.bb(by, PieceKind::Rook); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Rook, s);
        }
        
        for (BB pcs = b.bb(by, PieceKind::Queen); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Queen, s);
        }
        
        for (BB pcs = b.bb(by, PieceKind::Pawn); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            all |= attacks_from(b, by, PieceKind::Pawn, s);
        }
        
        {
            BB k = b.bb(by, PieceKind::King);
            if (k) {
                int s = lsb(k);
                all |= attacks_from(b, by, PieceKind::King, s);
            }
        }
        return all;
    }
} // namespace ch