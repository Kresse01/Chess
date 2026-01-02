#include "chess/gen/ch_legal_masks.h"

#include "chess/core/ch_bitboard.h"
#include "chess/core/ch_board.h"

#include "chess/analysis/ch_legality.h"
#include "chess/analysis/ch_pins.h"

#include "chess/gen/ch_legalize.h"
#include "chess/gen/ch_king_legal.h"

#include "chess/pieces/ch_piece.h"

namespace ch
{
    LegalMasks legal_masks_for_side(const Board& b, Color side)
    {
        LegalMasks out{};

        // Precompute context
        Pins pins = compute_pins(b, side);
        CheckState cs = compute_check_state(b, side);

        MoveOpts opts;
        opts.ep_sq = b.ep_target();

        // King (no castling yet)
        {
            BB kbb = b.bb(side, PieceKind::King);
            if(kbb)
            {
                int ks = lsb(kbb);
                out.per_square[ks] = legal_king_moves(b, side);
            }
        }

        // If double-check: only king moves are legal
        if (cs.double_check) return out;

        // Knights
        for (BB pcs = b.bb(side, PieceKind::Knight); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Knight, side, s, b, MovePhase::All, opts);
            out.per_square[s] = legalize_nonking_mask(b, pseudo, s, PieceKind::Knight, side, pins, cs);
        }

        // Bishops
        for (BB pcs = b.bb(side, PieceKind::Bishop); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Bishop, side, s, b, MovePhase::All, opts);
            out.per_square[s] = legalize_nonking_mask(b, pseudo, s, PieceKind::Bishop, side, pins, cs);
        }

        // Rooks
        for (BB pcs = b.bb(side, PieceKind::Rook); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Rook, side, s, b, MovePhase::All, opts);
            out.per_square[s] = legalize_nonking_mask(b, pseudo, s, PieceKind::Rook, side, pins, cs);
        }

        // Queens
        for (BB pcs = b.bb(side, PieceKind::Queen); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Queen, side, s, b, MovePhase::All, opts);
            out.per_square[s] = legalize_nonking_mask(b, pseudo, s, PieceKind::Queen, side, pins, cs);
        }

        // Pawns
        for (BB pcs = b.bb(side, PieceKind::Pawn); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Pawn, side, s, b, MovePhase::All, opts);
            out.per_square[s] = legalize_nonking_mask(b, pseudo, s, PieceKind::Pawn, side, pins, cs);
        }

        return out;
    }   
} // namespace ch