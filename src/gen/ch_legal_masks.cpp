#include "chess/gen/ch_legal_masks.h"
#include "chess/analysis/ch_legality.h"
#include "chess/analysis/ch_pins.h"
#include "chess/gen/ch_legalize.h"

namespace ch
{
    LegalMasks legal_masks_for_side(const Board& b, Color side)
    {
        LegalMasks out{};
        MoveOpts o; //default: no EP shaping or castling shaping here

        // Precompute context
        Pins pins = compute_pins(b, side);
        CheckState cs = compute_check_state(b, side);

        // King (no castling yet)
        {
            BB kbb = b.bb(side, PieceKind::King);
            if(kbb)
            {
                int ks = lsb(kbb);
                out.per_square[ks] = legal_king_moves(b, side); // already excludes enemy control and own occ
            }
        }

        if (cs.double_check) return out; // only king moves legal

        // For each non-king kind, legalize its pseudo mask
        auto do_kind = [&](PieceKind k, auto tag)
        {
            for (BB pcs = b.bb(side, k); pcs; )
            {
                int s = lsb(pcs); pcs ^= bit(s);
                BB pseudo = move(tag, side, s, b, MovePhase::All, o);
                out.per_square[s] = legalize_nonking_mask(b,pseudo, s, k, side, pins, cs);
            }
        };

        do_kind(PieceKind::Pawn, Pawn);
        do_kind(PieceKind::Knight, Knight);
        do_kind(PieceKind::Bishop, Bishop);
        do_kind(PieceKind::Rook, Rook);
        do_kind(PieceKind::Queen, Queen);

        return out;
    }   
}