#include "chess/core/ch_state.h"
#include "chess/core/ch_board.h"
#include "chess/gen/ch_movegen.h"
#include <cassert>
#include <vector>
#include <algorithm>

namespace ch
{
    static inline int file_of(int s) { return s & 7; }
    static inline int rank_of(int s) { return s >> 3; }

    static inline bool is_promotion_dest(Color side, int to)
    {
        return side == Color::White ? (rank_of(to)==7) : (rank_of(to) == 0);
    }

    /* Promotion code mapping:
    Matches the order you used when emitting promotions in movegen:
        push_promotions(...): codes 0, 1, 2, 3
    We'll interpret as: 1=N, 2=B, 3=R, 4=Q.
    */
    static inline PieceKind promo_code_to_kind(uint8_t code)
    {
        switch (code)
        {
            case 0: return PieceKind::Knight;
            case 1: return PieceKind::Bishop;
            case 2: return PieceKind::Rook;
            case 3: return PieceKind::Queen;
            default: return PieceKind::Queen;
        }
    }

    // read castling rights into a compact mask (WK|WQ|BK|BQ)
    static inline uint8_t get_castle_mask(const Board& b)
    {
        uint8_t m = 0;
        if(b.castle_k(Color::White)) m |= 1u;
        if(b.castle_q(Color::White)) m |= 1u<<1;
        if(b.castle_k(Color::Black)) m |= 1u<<2;
        if(b.castle_q(Color::Black)) m |= 1u<<3;
        return m;
    }

    // restore castling rights from mask using your set_castle accessor
    static inline void set_castle_mask(Board& b, uint8_t m)
    {
        b.set_castle(Color::White, true, (m & (1u   )) != 0); // WK
        b.set_castle(Color::White, false, (m & (1u<<1)) != 0); // WQ
        b.set_castle(Color::Black, true, (m & (1u<<2)) != 0); // BK
        b.set_castle(Color::Black, false, (m & (1u<<3)) != 0); // BQ
    }

    static inline void clear_castle_for_rook_square(Board& b, Color side, int rook_sq)
    {
        const int r = (side == Color::White) ? 0 : 7;
        if (rook_sq == idx(7,r)) b.set_castle(side,/*K-side*/true, false); // h-file rook
        if (rook_sq == idx(0,r)) b.set_castle(side, /*Q-side*/false, false); // a-file rook
    }

    // Clear both rights if the king of 'side' moved
    static inline void clear_castle_for_king(Board& b, Color side)
    {
        b.set_castle(side, true, false);
        b.set_castle(side, false, false);
    }

    static inline int castle_rook_from(Color side, bool kingSide)
    {
        int r = (side == Color::White) ? 0 : 7;
        return kingSide ? idx(7,r) : idx(0,r);
    }

    static inline int castle_rook_to(Color side, bool kingSide)
    {
        int r = (side == Color::White) ? 0 : 7;
        return kingSide ? idx(5,r) : idx(3,r);
    }

    // find piece (color, kind) on a square by scanning bitboards
    static inline bool piece_at(const Board& b, int sq, Color& outC, PieceKind& outK)
    {
        BB mask = bit(sq);
        if (!(b.occ_all() & mask)) return false;
        if (b.occ(Color::White) & mask) {
            outC = Color::White;
            for (int k=0; k<6; ++k) if (b.bb(outC, PieceKind(k)) & mask) { outK = PieceKind(k); return true; }
        } else {
            outC = Color::Black;
            for (int k=0; k<6; ++k) if (b.bb(outC, PieceKind(k)) & mask) { outK = PieceKind(k); return true; }
        }
        return false;
    }

    // --------------- public API ----------------------
    void make_move(Board& b, Move m, State& st)
    {
        const int from = m.from();
        const int to = m.to();
        const Color side = b.side_to_move();
        const Color them = (side == Color::White ? Color::Black : Color::White);

        //Snapshot (for unmake)
        st.stm = side;
        st.castle_mask = get_castle_mask(b);
        st.ep_sq = static_cast<int8_t>(b.ep_target());
        st.halfmove = b.halfmove_clock();
        st.fullmove = b.fullmove_number();
        st.promo_code = m.promo_code();
        st.was_ep = false;
        st.was_castle = false;
        st.captured = PieceKind::None;
        st.moved = PieceKind::None;

        // Identify moved piece kind (scan our bitboards at 'from');
        for (int k = 0; k<6; ++k)
        {
            if (b.bb(side, PieceKind(k)) & bit(from)) { st.moved = PieceKind(k); break; }
        }

        assert(st.moved != PieceKind::None && "No moving piece on 'from' for side_to_move");

        const bool isPawn = (st.moved == PieceKind::Pawn);
        bool didCapture = false;
        if (m.is_capture())
        {
            if(isPawn && m.is_special())
            {
                // En-passant capture: captured pawn sits behind 'to'
                const int cap_sq = (side==Color::White) ? (to-8) : (to+8);
                b.clear_piece(them,PieceKind::Pawn,cap_sq);
                st.captured = PieceKind::Pawn;
            } else {
                //Normal capture on 'to'
                Color c2; PieceKind k2;
                const bool ok = piece_at(b,to,c2,k2);
                assert(ok && c2 == them);
                b.clear_piece(them,k2,to);
                st.captured = k2;
            }
            didCapture = true;
        }

        // EP target: clear by default; may set on a double pawn push
        b.set_ep_target(-1);
        
        // ---- Move the piece (promotion / castling handled here) ---
        if (isPawn)
        {
            if(st.promo_code && is_promotion_dest(side,to))
            {
                // Promotion: from pawn -> to promoted piece
                b.clear_piece(side, PieceKind::Pawn, from);
                b.set_piece(side, promo_code_to_kind(st.promo_code), to);
            } else {
                // Normal pawn move
                b.clear_piece(side, PieceKind::Pawn, from);
                b.set_piece(side, PieceKind::Pawn, to);

                // Double push -> set ep target (square jumped over)
                if (side == Color::White && rank_of(from) == 1 && rank_of(to) == 3)
                {
                    b.set_ep_target(from + 8);
                } else if (side == Color::Black && rank_of(from) == 6 && rank_of(to) == 4)
                {
                    b.set_ep_target(from - 8);
                }
            }
        } else if (st.moved == PieceKind::King) {
            // Detect castling by geometry from e-file to c/g-file
            const int r = (side == Color::White) ? 0 : 7;
            const int e = idx(4,r);
            const bool kingSide = (from == e && to == idx(6, r));
            const bool queenSide = (from == e && to == idx(2, r));

            if (kingSide || queenSide)
            {
                st.was_castle = true;
                // King
                b.clear_piece(side, PieceKind::King, from);
                b.set_piece(side, PieceKind::King, to);
                // Rook
                const int rf = castle_rook_from(side, kingSide);
                const int rt = castle_rook_to(side, kingSide);
                b.clear_piece(side, PieceKind::Rook, rf);
                b.set_piece(side,PieceKind::Rook, rt);

                clear_castle_for_king(b,side);
            } else {
                // Normal king move
                b.clear_piece(side, PieceKind::King, from);
                b.set_piece(side, PieceKind::King, to);
                clear_castle_for_king(b,side);
            }
        } else {
            // Knight / Bishop / Rook / Queen
            b.clear_piece(side, st.moved, from);
            b.set_piece(side, st.moved, to);

            // If a rook moved off its original square, clear that right
            if (st.moved == PieceKind::Rook)
            {
                clear_castle_for_rook_square(b, side, from);
            }
        }

        // If we captured an enemy rook on a corner, clear their right
        if (didCapture && st.captured == PieceKind::Rook)
        {
            clear_castle_for_rook_square(b, them, to);
        }

        // Halfmove clock: reset on pawn move or any capture; else increment
        if (isPawn || didCapture) b.set_halfmove_clock(0);
        else b.set_halfmove_clock(b.halfmove_clock() + 1);

        // Fullmove number is increased after Black's move
        if (side == Color::Black) b.set_fullmove_number(b.fullmove_number() + 1);

        // Flip side to move
        b.set_side_to_move(them);
    }

    void unmake_move(Board& b, Move m, const State& st)
    {
        const int from = m.from();
        const int to = m.to();
        const Color side = st.stm;
        const Color them = (side == Color::White ? Color::Black : Color::White);

        // Flip back first (mirror make move)
        b.set_side_to_move(side);

        // Reverse the move
        if (st.moved == PieceKind::King && st.was_castle)
        {
            // Move king back
            b.clear_piece(side, PieceKind::King, to);
            b.set_piece(side, PieceKind::King, from);

            //Move rook back
            const bool kingSide = (to > from); // e->g is ks, e->c is qs
            const int rf = castle_rook_from(side, kingSide);
            const int rt = castle_rook_to(side,kingSide);
            b.clear_piece(side,PieceKind::Rook, rt);
            b.set_piece(side,PieceKind::Rook, rf);
        } else if (st.moved == PieceKind::Pawn && st.promo_code && is_promotion_dest(side, to)) {
            // Was a promotion: remove promoted piece, restore pawn on from
            b.clear_piece(side, promo_code_to_kind(st.promo_code),to);
            b.set_piece(side, PieceKind::Pawn, from);
            // Restore captured on 'to' if any
            if (st.captured != PieceKind::None)
            {
                b.set_piece(them,st.captured, to);
            }
        } else {
            b.clear_piece(side,st.moved,to);
            b.set_piece(side,st.moved,from);

            // Restore if captured piece
            if (st.captured != PieceKind::None)
            {
                if (st.was_ep)
                {
                    const int cap_sq = (side == Color::White) ? (to - 8) : (to + 8);
                    b.set_piece(them, PieceKind::Pawn, cap_sq);
                } else {
                    b.set_piece(them, st.captured, to);
                }
            }
        }

        //Restore flags/counters exactly
        set_castle_mask(b,st.castle_mask);
        b.set_ep_target(st.ep_sq);
        b.set_halfmove_clock(st.halfmove);
        b.set_fullmove_number(st.fullmove);
    }

    // Convenience: validate with movegen, then apply
    // Returns true if applied; false if 'm' is not legal in the current position.
    bool apply_if_legal(Board& b, Move m, State& st)
    {
        std::vector<Move> legal;
        generate_legal_moves(b,b.side_to_move(),legal);

        auto same = [](const Move& a, const Move& b)
        {
            return a.from() == b.from() && a.to() == b.to()
            && a.is_capture()==b.is_capture()
            && a.promo_code()==b.promo_code()
            && a.is_special()==b.is_special();
        };

        auto it = std::find_if(legal.begin(), legal.end(),
                                [&](const Move& x) {return same(x,m); });

        if (it == legal.end()) return false;

        make_move(b,*it,st);
        return true;
    }

}