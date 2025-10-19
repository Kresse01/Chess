#pragma once
#include "chess/analysis/ch_attack.h"
#include "chess/analysis/ch_pins.h"
#include "chess/pieces/ch_piece.h"

namespace ch
{
    struct CheckState {
        bool in_check = false;
        bool double_check = false;
        BB checkers = 0; // Squares for checking pieces
        BB block_or_capture = 0;  // for single check: squares that resolve the the check (checkers square U between)
        int our_king_sq = -1;
    };

    /**
     * @brief Compute check information for 'side':
     *  - who checks us,
     *  - if it's double check,
     *  - and the ray squares that resolve a single check.
     */
    CheckState compute_check_state(const Board& b, Color side);
}