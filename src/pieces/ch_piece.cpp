#include "chess/pieces/ch_piece.h"

#include "chess/pieces/ch_knight.h"
#include "chess/pieces/ch_bishop.h"
#include "chess/pieces/ch_rook.h"
#include "chess/pieces/ch_queen.h"
#include "chess/pieces/ch_pawn.h"
#include "chess/pieces/ch_king.h"

namespace ch
{
    BB move(PieceKind k, Color c, int s, const Board& b, MovePhase ph, const MoveOpts& o)
    {
        switch(k)
        {
            case PieceKind::Pawn: return move(Pawn, c, s, b, ph, o);
            case PieceKind::Knight: return move(Knight, c, s, b, ph, o);
            case PieceKind::Bishop: return move(Bishop, c, s, b, ph, o);
            case PieceKind::Rook: return move(Rook, c, s, b, ph, o);
            case PieceKind::Queen: return move(Queen, c, s, b, ph, o);
            case PieceKind::King: return move(King, c, s, b, ph, o);
            
            case PieceKind::None:
            default:
                return 0;
        }
    }
}