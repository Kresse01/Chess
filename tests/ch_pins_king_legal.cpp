#include "chess/core/ch_board.h"
#include "chess/gen/ch_legal_masks.h"
#include "chess/analysis/ch_pins.h"
#include "chess/gen/ch_king_legal.h"
#include "chess/core/ch_square.h"
#include "chess/gen/ch_legalize.h"
#include <cassert>
#include <iostream>

int main()
{
    ch::init_bitboards();
    ch::Board b;
    
    b.clear();
    b.set_piece(ch::Color::White, ch::PieceKind::King, ch::sq_from_str("e1"));
    b.set_piece(ch::Color::White, ch::PieceKind::Knight, ch::sq_from_str("f3"));
    b.set_piece(ch::Color::Black, ch::PieceKind::Bishop, ch::sq_from_str("b4"));

    auto L = ch::legal_masks_for_side(b, ch::Color::White);

    auto nf3 = L.per_square[ch::sq_from_str("f3")];

    assert((nf3 & ch::bit(ch::sq_from_str("d2"))) != 0);
    assert((nf3 & ch::bit(ch::sq_from_str("h4"))) == 0);

    auto ke1 = L.per_square[ch::sq_from_str("e1")];

    assert((ke1 & ch::bit(ch::sq_from_str("d2"))) == 0);

    std::cout << "legal masks OK\n";
    return 0;
}