#pragma once
/**
 * @file ch_movegen.h
 * @brief Public interface for generating fully legal moves
 * 
 * The move generator returns *fully legal* moves:
 *  - No moves leaving own king in check
 *  - Castling included when legal
 *  - En-passant included only when legal (king safety after EP)
 * 
 * Implementation lives in src/gen/ch_movegen.cpp and uses internal helpers:
 *  - pins /check state
 *  - king legality
 *  - non-king legalization
*/

#include <vector>
#include "chess/core/ch_move.h"
#include "chess/core/ch_move.h"


namespace ch
{
    class Board; // Forward declaration

    /**
     * @brief Generate all fully legal moves for @p side in position @p b.
     * @param b current position
     * @param side side for which to generate moves (usually b.side_to_move())
     * @param out output vector (cleared and then filled)
     */
    void generate_legal_moves(const Board& b, Color side, std::vector<Move>& out);
} // namespace ch