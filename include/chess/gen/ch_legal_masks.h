#pragma once
#include "chess/gen/ch_king_legal.h"
#include "chess/gen/ch_legalize.h"

namespace ch
{
    /**
     * @brief Compute legal destination masks for every piece of 'side'.
     * Results is an array of 64 bitboards; only indices with a piece of 'side' are set.
     * King moves exclude castling for now (add later).
     */
    struct LegalMasks
    {
        BB per_square[64]{}; //destination for piece sitting on index i
    };

    LegalMasks legal_masks_for_side(const Board& b, Color side);
}