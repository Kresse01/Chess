#pragma once
#include <cstdint>

#include "chess/core/ch_types.h"
#include "chess/core/ch_move.h"

namespace ch
{
    class Board; // forward declaration; definition in ch_board.h

    /**
     * @brief Reversible snapshot of everything a move can change.
     * 
     * This is an "undo record": it stores ONLY what is necessary to restore the
     * exact pre-move position (including rule state like castling/EP/50-move).
     * 
     * The contract is:
     *  make_move(b, m, st) -> mutates b and fills st with the previous state
     *  unmake_move(b, m, st) -> restores b exactly to the pre-move state
     */
    struct State
    {
        // Side to move BEFORE the move was made.
        Color stm{Color::White};

        // Rule-state BEFORE the move:
        // bitmask 1=WK, 2=WQ, 4=BK, 8=BQ
        std::uint8_t castle_mask{0};
        std::int8_t ep_sq{-1}; // en-passant square, or -1 if none
        std::uint16_t halfmove{0}; // 50-move clock BEFORE move
        std::uint16_t fullmove{1}; // fullmove number BEFORE move

        // For unmake (details about what the move actually did):
        PieceKind moved : 4; // moved piece kind (pre-promo for pawns)
        PieceKind captured : 4; // captured piece kind (None if no capture)
        std::uint8_t promo_code : 2; // 0..3 (N,B,R,Q) if promotion else 0
        bool was_ep : 1; // this move was an en-passant capture
        bool was_castle : 1; // this move was a castling move (king e->g/c)
    };

    /// Apply move @p m to board @p b, writing the undo snapshot into @p st.
    void make_move(Board& b, Move m, State& st);

    /// Undo move @p m on board @p b using the previously saved snapshot @p st.
    void unmake_move(Board& b, Move m, const State& st);

    /// Castling bit helpers (keep here because they're used by make/unmake and board helpers)
    inline constexpr std::uint8_t WK = 1u << 0;
    inline constexpr std::uint8_t WQ = 1u << 1;
    inline constexpr std::uint8_t BK = 1u << 2;
    inline constexpr std::uint8_t BQ = 1u << 3;
} // namespace ch