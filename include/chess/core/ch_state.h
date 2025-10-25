#pragma once
#include <cstdint>
#include "chess/core/ch_types.h"
#include "chess/core/ch_board.h"
#include "chess/core/ch_move.h"

namespace ch
{
    /**
     * @brief Reversible snapshot of everything a move can change.
     * We store only what we need to undo quickly and safely.
     */
    struct State
    {
        // Side to move before the move
        Color stm;

        // Flags
        uint8_t castle_mask; //bitmask 1=WK, 2=WQ, 4=BK, 8=BQ
        int8_t ep_sq; // -1 if none
        uint16_t halfmove; // 50-move rule counter before the move
        uint16_t fullmove;

        // For unmake
        PieceKind moved : 4; // piece kind moves (pre-promo for pawns)
        PieceKind captured : 4; // captured piece kind (None if none)
        uint8_t promo_code : 2; // 0..3 (N,B,R,Q) if promotion else 0
        bool was_ep : 1; // this move was an en-passant capture
        bool was_castle : 1; // this move was a castling move (king e->g/c)
    };

    void make_move(Board& b, Move m, State& st);
    void unmake_move(Board& b, Move m, const State& st);

    /// Castling bit helpers (keep in one header for reuse)
    constexpr uint8_t WK = 1u << 0;
    constexpr uint8_t WQ = 1u << 1;
    constexpr uint8_t BK = 1u << 2;
    constexpr uint8_t BQ = 1u << 3;
}