#pragma once
#include <vector>
#include "chess/core/ch_board.h"
#include "chess/core/ch_move.h"
#include "chess/gen/ch_legalize.h"
#include "chess/gen/ch_king_legal.h"
#include "chess/analysis/ch_pins.h"
#include "chess/analysis/ch_legality.h"
#include "chess/pieces/ch_piece.h"

namespace ch
{
    // Generate *fully legal* moves for 'side' (no make/unmake yet)
    void generate_legal_moves(const Board& b, Color side, std::vector<Move>& out);

    // Optional: castling legality helper (K/Q) - returns extra king destinations (g/c)
    // We fold this into king generation below; expose if you prefer seperate testing.
}