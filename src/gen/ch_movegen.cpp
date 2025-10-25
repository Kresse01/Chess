#include "chess/gen/ch_movegen.h"
#include "chess/core/ch_bitboard.h"
#include <cassert>

namespace ch
{
    static inline void push_moves_from_mask(int from, BB mask, bool is_capture_mask, std::vector<Move>& out)
    {
        for (BB m = mask; m; )
        {
            int to = lsb(m); m ^= bit(to);
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
        BB ownOcc = b.occ(side);
        BB enemyOcc = b.occ(them);

        //--- King (with castling legality) ---
        {
            BB kbb = b.bb(side, PieceKind::King);
            if (kbb)
            {
                int ks = lsb(kbb);
                // Legal king steps (no castling yet)
                BB ksteps = legal_king_moves(b, side);

                // Emit each destination and mark castling as special
                for (BB m = ksteps; m; )
                {
                    int to =  lsb(m); m^= bit(to);
                    bool is_cap = (enemyOcc & bit(to)) != 0;
                    bool special = is_castle_to(side, ks, to); // castle if e->g/c
                    out.push_back(Move::make(ks, to, is_cap, 0, special));
                }
            }
        }
        
        if(cs.double_check) return; //only king moves are legal

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
        {
            // Use your pawn masks:
            // quiet pushes via MovePhase::Quiet
            // captures via MovePhase::Attacks
            for (BB pcs = b.bb(side, PieceKind::Pawn); pcs; )
            {
                int s = lsb(pcs); pcs ^= bit(s);

                BB pq = move(Pawn, side, s, b, MovePhase::Quiet, opts);
                BB pc = move(Pawn, side, s, b, MovePhase::Attacks, opts);

                // Merge to ALL (so the legality filter can apply check/pin constraints uniformly)
                BB pseudo_all = pc | pq;
                BB legal_all = legalize_nonking_mask(b, pseudo_all, s, PieceKind::Pawn, side, pins, cs);

                const int ep = opts.ep_sq;
                const BB ep_bit = (ep>=0 ? bit(ep) : BB(0));

                // Split captures/quiet after legalization
                BB legal_ep = legal_all & ep_bit;
                BB legal_capt = legal_all & enemyOcc;
                BB legal_quiet = legal_all & ~(b.occ_all() | ep_bit); // only empties

                // promotions (dest on last rank)
                auto emit_range = [&](BB mask, bool isCap)
                {
                    while (mask)
                    {
                        int to = lsb(mask); mask ^= bit(to);
                        if (on_last_rank(side, to))
                        {
                            push_promotions(s, to, isCap, out);
                        } else {
                            out.push_back(Move::make(s, to, isCap));
                        }
                    }
                };
                emit_range(legal_quiet, false);
                emit_range(legal_capt, true);

                while(legal_ep)
                {
                    int to = lsb(legal_ep); legal_ep ^= bit(to);
                    // EP can't be promoted square in standard chess
                    out.push_back(Move::make(s,to,/*capture=*/true,/*promo=*/0,/*special=*/true));
                }
            }
        }
    }
}