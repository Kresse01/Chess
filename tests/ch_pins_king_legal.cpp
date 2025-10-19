#include "chess/core/ch_board.h"
#include "chess/analysis/ch_pins.h"
#include "chess/gen/ch_king_legal.h"
#include "chess/core/ch_square.h"
#include <cassert>
#include <iostream>

int main()
{
    ch::init_bitboards();
    ch::Board b;
    
    b.clear();
    b.set_piece(ch::Color::White, ch::PieceKind::King, ch::sq_from_str("e1"));
    b.set_piece(ch::Color::White, ch::PieceKind::Knight, ch::sq_from_str("f2"));
    b.set_piece(ch::Color::Black, ch::PieceKind::Bishop, ch::sq_from_str("g3"));

    auto pins = ch::compute_pins(b, ch::Color::White);
    assert(pins.pinned & ch::bit(ch::sq_from_str("f2")));

    ch::BB ray = pins.ray_to_enemy[ch::sq_from_str("f2")];
    assert(ray & ch::bit(ch::sq_from_str("e1")));
    assert(ray & ch::bit(ch::sq_from_str("g3")));

    ch::BB kleg = ch::legal_king_moves(b, ch::Color::White);

    assert((kleg & ch::bit(ch::sq_from_str("e2"))) != 0);
    assert((kleg & ch::bit(ch::sq_from_str("f2"))) == 0);

    std::cout << "pins + king legal OK\n";
    return 0;
}