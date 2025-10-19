#include "chess/core/ch_board.h"
#include "chess/analysis/ch_attack.h"
#include "chess/core/ch_square.h"
#include <cassert>
#include <iostream>

int main()
{
    ch::init_bitboards();
    ch::Board b;

    b.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // 1) White knight on g1 attacks e2/f3/h3 (but f3/h3 only if not blocked by own? leaper ignores blockers)
    {
        int g1 = ch::sq_from_str("g1");
        ch::BB nAtt = ch::attacks_from(b, ch::Color::White, ch::PieceKind::Knight, g1);
        // In startpos, e2 is occupied by white pawn -> still "attacked"? Knights exclude own-occupied squares in our def
        // So only f3 and h3 should be set as control squares.
        assert((nAtt & ch::bit(ch::sq_from_str("f3"))) != 0);
        assert((nAtt & ch::bit(ch::sq_from_str("h3"))) != 0);
        assert((nAtt & ch::bit(ch::sq_from_str("e2"))) == 0);
    }

    // 2) Side attacks: from startpos, black attacks include e4 (by d5 pawn after push? not yet) but d4 is attacked by c5 pawn? No.
    // Simpler: white attacks should include b5 from a4 knight? Let's alter a simple custom position:
    b.set_fen("8/8/8/8/2B5/8/8/4K3 w - - 0 1");
    {
        ch::BB wAll = ch::attacks_side(b, ch::Color::White);
        // Bishop on c4 attacks h9 offboard; check a diagonal square like e6 and a2:
        assert((wAll & ch::bit(ch::sq_from_str("e6"))) != 0);
        assert((wAll & ch::bit(ch::sq_from_str("a2"))) != 0);
        assert((wAll & ch::bit(ch::sq_from_str("d1"))) != 0);
    }

    std::cout << "attack maps OK\n";
    return 0;
}