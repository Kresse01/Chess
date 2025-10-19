#pragma once
#include <vector>
#include "chess/core/ch_types.h"

namespace ch {

    class board; // forward declaration

    struct ipiece {
        virtual ~ipiece() = default;
        virtual PieceKind kind() const = 0;
        virutal Color color() const = 0;

        // For now instead of generating moves, just describe what piece *can do*.
        virtual bool can_move_to(const board& b, Square from, Square to) const = 0;
        virtual bool is_valid_starting
    }
}