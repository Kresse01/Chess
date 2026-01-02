#include "chess/gen/ch_movegen.h"

#include "chess/core/ch_bitboard.h"
#include "chess/core/ch_board.h"

#include "chess/analysis/ch_pins.h"
#include "chess/analysis/ch_legality.h"

#include "chess/gen/ch_legalize.h"
#include "chess/gen/ch_king_legal.h"

#include "chess/pieces/ch_piece.h"

#include <cassert>

namespace ch
{
    static inline void push_moves_from_mask(int from, BB mask, bool is_capture_mask, std::vector<Move>& out)
    {
        for (BB m = mask; m; )
        {
            int to = lsb(m);
            m ^= bit(to);
            out.push_back(Move::make(from, to, is_capture_mask));
        }
    }

    // Emit 4 promotion moves for a single (from, to) pawn move
    static inline void push_promotions(int from, int to, bool capture, std::vector<Move>& out)
    {
        out.push_back(Move::make(from, to, capture, 0));
        out.push_back(Move::make(from, to, capture, 1));
        out.push_back(Move::make(from, to, capture, 2));
        out.push_back(Move::make(from, to, capture, 3));
    }

    static inline bool on_last_rank(Color side, int sq)
    {
        int r = sq >> 3;
        return side == Color::White ? (r == 7) : (r == 0);
    }

    static inline bool is_castle_to(Color side, int from, int to)
    {
        // Require king to start on e-file (e1, e8)
        int r = (side == Color::White) ? 0 : 7;
        int e = (r << 3) | 4;
        int g = (r << 3) | 6;
        int c = (r << 3) | 2;
        return (from == e) && (to == g || to == c);
    }

    void generate_legal_moves(const Board& b, Color side, std::vector<Move>& out)
    {
        out.clear();

        // Precompute context for non-king pieces
        Pins pins = compute_pins(b, side);
        CheckState cs = compute_check_state(b, side);

        MoveOpts opts; // default: no explicit castle shaping; we add castle seperately
        opts.ep_sq = b.ep_target();

        const Color them = opposite(side);
        BB enemyOcc = b.occ(them);

        //--- King (with castling legality) ---
        {
            BB kbb = b.bb(side, PieceKind::King);
            if (kbb)
            {
                int ks = lsb(kbb);
                
                // legal_king_moves() already:
                //  - excludes stepping onto attacked squares
                //  - excludes own-occupied squares
                //  - includes castling destinations if legal
                BB ksteps = legal_king_moves(b, side);

                for (BB m = ksteps; m; )
                {
                    int to =  lsb(m);
                    m^= bit(to);

                    bool is_cap = (enemyOcc & bit(to)) != 0;
                    bool special = is_castle_to(side, ks, to); // mark castle for make/unmake
                    out.push_back(Move::make(ks, to, is_cap, 0, special));
                }
            }
        }
        
        // Double check: only king moves are legal
        if(cs.double_check) return;

        // --- Knights ---
        for (BB pcs = b.bb(side, PieceKind::Knight); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Knight, side, s, b, MovePhase::All, opts);
            BB legal = legalize_nonking_mask(b, pseudo, s, PieceKind::Knight, side, pins, cs);

            BB cap = legal & enemyOcc;
            BB qui = legal & ~enemyOcc;
            push_moves_from_mask(s, qui, false, out);
            push_moves_from_mask(s, cap, true, out);
        }

        // --- Bishops ---
        for (BB pcs = b.bb(side, PieceKind::Bishop); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Bishop, side, s, b, MovePhase::All, opts);
            BB legal = legalize_nonking_mask(b, pseudo, s, PieceKind::Bishop, side, pins, cs);
            BB cap = legal & enemyOcc;
            BB qui = legal & ~enemyOcc;
            push_moves_from_mask(s, qui, false, out);
            push_moves_from_mask(s, cap, true, out);
        }

        // --- Rooks ---
        for (BB pcs = b.bb(side, PieceKind::Rook); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Rook, side, s, b, MovePhase::All, opts);
            BB legal = legalize_nonking_mask(b, pseudo, s, PieceKind::Rook, side, pins, cs);

            BB cap = legal & enemyOcc;
            BB qui = legal & ~enemyOcc;
            push_moves_from_mask(s, qui, false, out);
            push_moves_from_mask(s, cap, true, out);
        }

        // --- Queens ---
        for (BB pcs = b.bb(side, PieceKind::Queen); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Queen, side, s, b, MovePhase::All, opts);
            BB legal = legalize_nonking_mask(b, pseudo, s, PieceKind::Queen, side, pins, cs);

            BB cap = legal & enemyOcc;
            BB qui = legal & ~enemyOcc;
            push_moves_from_mask(s, qui, false, out);
            push_moves_from_mask(s, cap, true, out);
        }

        // --- Pawns ---
        for (BB pcs = b.bb(side, PieceKind::Pawn); pcs; )
        {
            int s = lsb(pcs); pcs ^= bit(s);
            BB pseudo = move(Pawn, side, s, b, MovePhase::All, opts);
            BB legal = legalize_nonking_mask(b, pseudo, s, PieceKind::Pawn, side, pins, cs);

            const int ep = b.ep_target();
            const bool has_ep = (ep != -1);

            BB cap = legal & enemyOcc;
            BB qui = legal & ~enemyOcc;

            // Promotions: if destination is last rank, emit 4 promotions instead of a normal pawn move.
            for (BB q = qui; q; )
            {
                int to = lsb(q); q ^= bit(to);
                const bool is_ep = has_ep && (to == ep);
                if (is_ep)
                {
                    // En-passent: capture + special
                    out.push_back(Move::make(s,to,/*capture*/true, /*promo*/ 0, /*special*/ true));
                    continue;
                }
                if (on_last_rank(side, to))
                    push_promotions(s, to, /*capture*/false, out);
                else
                    out.push_back(Move::make(s, to, /*capture*/false));
            }

            for (BB c = cap; c; )
            {
                int to = lsb(c); c ^= bit(to);
                if (on_last_rank(side, to))
                    push_promotions(s, to, /*capture*/true, out);
                else
                    out.push_back(Move::make(s, to, /*capture*/true));
            }

            // NOTE: en-passant moves are already included in `legal` if:
            //  - pseudo included EP target (via move(Pawn,...,MovePhase::All, opts))
            //  - legalize_nonking_mask kept it (king_safe_after_ep)
            //
            // Those EP moves will appear either in `cap` or `qui` depending on how pawn move()
            // encodes them. In your current design, EP is treated as a "capture" at the mask level,
            // and make_move checks (pawn && capture && special) to detect EP.
        }
    }
} // namespace ch