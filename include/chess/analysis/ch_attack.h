#pragma once
#include "chess/core/ch_board.h"
#include "chess/pieces/ch_piece.h"

namespace ch
{
    BB attackers_to(const Board& b, int sq, Color by);
    bool in_check(const Board& b, Color side);
    inline bool is_attacked(const Board& b, int sq, Color by) { return attackers_to(b, sq, by) != 0; }

    /**
     * @brief Squares attacked (controlled) by a *single piece* at 'fromSq'.
     * 
     * Definition notes:
     *  - Knights/Kings: standard leaper attacks, excluding own-occupied squares.
     *  - Pawns: capture directions only (they "attack" diagonals, not he push)
     *  - Sliders: ray until first blocker; include the blocker square only if it's the enemy.
     *    (Equivalent to span & ~ownOcc.)
     * 
     * Implementation uses your 'move()' overloads with the right MovePhase:
     *  - Pawns -> MovePhase::Attacks
     *  - Others -> MovePhase::All (but own-occupied squares are excluded by the piece impl)
     * 
     * Castling is **not** considered an attack; en-passant target doesn't change attack.
     */
    BB attacks_from(const Board& b, Color by, PieceKind kind, int fromSq);

    /**
     * @brief Union of all squares attacked by side 'by' (all pieces).
     * 
     * Typical use: build a fast "danger map" to filter king moves or check conditions.
     */
    BB attacks_side(const Board& b, Color by);
}