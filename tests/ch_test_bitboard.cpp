#include "chess/core/ch_bitboard.h"
#include "chess/core/ch_board.h"
#include "chess/core/ch_square.h"
#include "chess/pieces/ch_piece.h"
#include <cassert>
#include <iostream>

static void print_bb(ch::BB b)
{
    for (int r = 7; r >= 0; --r)
    {
        for (int f = 0; f < 8; ++f)
        {
            std::cout << ((b>>((r<<3)|f)) & 1ull);
        }
        std::cout<<'\n';
    }
    std::cout<<"------\n";
}

int main()
{
    ch::init_bitboards();
    ch::Board b;
    b.clear(); // empty board

    // Place a few pieces
    b.set_piece(ch::Color::White, ch::PieceKind::Knight, ch::sq_from_str("d4"));
    b.set_piece(ch::Color::White, ch::PieceKind::Bishop, ch::sq_from_str("c1"));
    b.set_piece(ch::Color::Black, ch::PieceKind::Pawn, ch::sq_from_str("e5"));

    ch::MoveOpts o; // defaults

    // Knight attacks from d4 on empty board
    {
        auto m = ch::move(ch::Knight, ch::Color::White, ch::sq_from_str("d4"), b, ch::MovePhase::All, o);
        // expect 8 moves; make a quick sanity check
        int cnt = __builtin_popcountll(m);
        assert(cnt==8 && "knight from d4 should have 8 taregts on empty board");
    }

    // Bishop masks (black pawn on e5 should be capturable by a white pawn from d4 if diagonally aligned? not here)
    // Instead, test a white pawn from a2 quiet pushes
    {
        b.set_piece(ch::Color::White, ch::PieceKind::Pawn, ch::sq_from_str("a2"));
        auto quiet = ch::move(ch::Pawn, ch::Color::White, ch::sq_from_str("a2"), b, ch::MovePhase::Quiet, o);
        // a3 must be set
        assert(quiet & ch::bit(ch::sq_from_str("a3")));
    }

    std::cout << "All piece-masks smoke tests passed. \n";
    return 0;
}