#include "chess/core/ch_board.h"
#include "chess/gen/ch_legal_masks.h"
#include "chess/analysis/ch_pins.h"
#include "chess/gen/ch_king_legal.h"
#include "chess/core/ch_square.h"
#include "chess/gen/ch_legalize.h"
#include "chess/gen/ch_movegen.h"
#include <cassert>
#include <iostream>

int main()
{
    using namespace ch;
    init_bitboards();

    Board b;
    b.set_startpos();

    std::vector<Move> mv;
    generate_legal_moves(b, Color::White,mv);
    std::cout << "white legal moves: " << mv.size() << "\n";
    assert(mv.size() == 20);

    // Check case: Ke1, Nf3 vs ...Bb4+ -> only Nf3-d2 plus king moves if legal
    b.clear();
    b.set_piece(Color::White, PieceKind::King,   sq_from_str("e1"));
    b.set_piece(Color::White, PieceKind::Knight, sq_from_str("f3"));
    b.set_piece(Color::Black, PieceKind::Bishop, sq_from_str("b4"));

    mv.clear();
    generate_legal_moves(b, Color::White, mv);

    bool foundNf3d2 = false;
    for (auto m : mv) {
        if (m.from() == sq_from_str("f3") && m.to() == sq_from_str("d2"))
            foundNf3d2 = true;
    }
    assert(foundNf3d2);
    std::cout << "movegen smoke ok\n";
    
    return 0;
}