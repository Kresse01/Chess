#include "chess/core/ch_board.h"
#include <cstring>
#include <cctype>
#include <string>

namespace ch
{
    static inline PieceKind char_to_kind(char p) {
        switch(std::tolower(static_cast<unsigned char>(p)))
        {
            case 'p' : return PieceKind::Pawn;
            case 'n' : return PieceKind::Knight;
            case 'b' : return PieceKind::Bishop;
            case 'r' : return PieceKind::Rook;
            case 'q' : return PieceKind::Queen;
            case 'k' : return PieceKind::King;
        }

        // not reached for valid inputs
        return PieceKind::Pawn;
    }

    void Board::rebuild_occ()
    {
        occ_[0] = occ_[1] = 0;
        for (int k = 0; k < 6; ++k)
        {
            occ_[0] |= bb_[0][k];
            occ_[1] |= bb_[1][k];
        }
        occ_all_ = occ_[0] | occ_[1];
    }

    void Board::clear()
    {
        for (int c = 0; c < 2; ++c)
            for (int k = 0; k < 6; ++k)
                bb_[c][k] = 0;
        occ_[0] = occ_[1] = occ_all_ = 0;
        castle_[0][0] = castle_[0][1] = castle_[1][0] = castle_[1][1] = false;
        ep_sq_ = -1;
        stm_ = Color::White;
    }

    void Board::set_startpos()
    {
        clear();
        // White
        bb_[int(Color::White)][int(PieceKind::Pawn)] = RANK_MASK[1];
        bb_[int(Color::White)][int(PieceKind::Rook)] = bit(idx(0,0)) | bit(idx(7,0));
        bb_[int(Color::White)][int(PieceKind::Knight)] = bit(idx(1,0)) | bit(idx(6,0));
        bb_[int(Color::White)][int(PieceKind::Bishop)] = bit(idx(2,0)) | bit(idx(5,0));
        bb_[int(Color::White)][int(PieceKind::Queen)] = bit(idx(3,0));
        bb_[int(Color::White)][int(PieceKind::King)] = bit(idx(4,0));
        // Black
        bb_[int(Color::Black)][int(PieceKind::Pawn)] = RANK_MASK[6];
        bb_[int(Color::Black)][int(PieceKind::Rook)] = bit(idx(0,7)) | bit(idx(7,7));
        bb_[int(Color::Black)][int(PieceKind::Knight)] = bit(idx(1,7)) | bit(idx(6,7));
        bb_[int(Color::Black)][int(PieceKind::Bishop)] = bit(idx(2,7)) | bit(idx(5,7));
        bb_[int(Color::Black)][int(PieceKind::Queen)] = bit(idx(3,7));
        bb_[int(Color::Black)][int(PieceKind::King)] = bit(idx(4,7));
        castle_[int(Color::White)][0] = castle_[int(Color::White)][1] = true;
        castle_[int(Color::Black)][0] = castle_[int(Color::Black)][1] = true;
        ep_sq_ = -1;
        stm_ = Color::White;
        rebuild_occ();
    }

    //Minimial, robust FEN parser: pieces / side / castling /ep. (Half/fullmove ignored.)
    bool Board::set_fen(const char* fen)
    {
        clear();
        if (!fen) return false;

        // 1) Piece placement
        int f = 0, r = 7;
        const char* p = fen;
        while (*p && *p != ' ')
        {
            char c = *p++;
            if (c == '/') { --r; f = 0; continue; }
            if (c >= '1' && c <= '8') { f += (c - '0'); continue; }
            if (std::isalpha(static_cast<unsigned char>(c)))
            {
                if (f>7 || r<0) return false;
                Color col = std::isupper(static_cast<unsigned char>(c)) ? Color::White : Color::Black;
                set_piece(col, char_to_kind(c), idx(f, r));
                ++f;
                continue;
            }
            return false;
        }
        if (*p != ' ') return false;
        ++p;
        rebuild_occ();

        // 2) side to move
        if (*p == 'w') { stm_ = Color::White; }
        else if (*p == 'b') { stm_ =  Color::Black; }
        else return false;
        while (*p && *p != ' ') ++p;
        if (*p == ' ') ++p;

        // 3) castling rights
        if (*p == '-') { /* none */ ++p; }
        else {
            while (*p && *p != ' ') {
                char c = *p++;
                if (c == 'K') castle_[int(Color::White)][0] = true;
                else if (c == 'Q') castle_[int(Color::White)][1] = true;
                else if (c == 'k') castle_[int(Color::Black)][0] = true;
                else if (c == 'q') castle_[int(Color::Black)][1] = true;
                else return false;
            }
        }
        if (*p == ' ') ++p;

        // 4) En-passant square
        if (*p=='-') { ep_sq_ = -1; ++p;}
        else {
            if (!std::isalpha(static_cast<unsigned char>(p[0])) || !std::isdigit(static_cast<unsigned char>(p[1])))
                return false;
            int file = std::tolower(static_cast<unsigned char>(p[0])) - 'a';
            int rank = (p[1] - '1');
            if (file < 0 || file > 7 || rank < 0 || rank > 7) return false;
            ep_sq_ = idx(file, rank);
            p += 2;
        }

        // Ignore halfmove/fullmove if present
        return true;
    }

    static inline PieceKind char_to_kind(char p)
    {
        switch (std::tolower(static_cast<unsigned char>(p)))
        {
            case 'p' : return PieceKind::Pawn;
            case 'n' : return PieceKind::Knight;
            case 'b' : return PieceKind::Bishop;
            case 'r' : return PieceKind::Rook;
            case 'q' : return PieceKind::Queen;
            case 'k' : return PieceKind::King;
        }
        // not reached for valid inputs
        return PieceKind::Pawn;
    }

    static inline char kind_to_char(PieceKind k, Color c)
    {
        char ch = 'p';
        switch (k)
        {
            case PieceKind::Pawn: ch = 'p'; break;
            case PieceKind::Knight: ch = 'n'; break;
            case PieceKind::Bishop: ch = 'b'; break;
            case PieceKind::Rook: ch = 'r'; break;
            case PieceKind::Queen: ch = 'q'; break;
            case PieceKind::King: ch = 'k'; break;
            default: ch = 'p'; break;
        }

        if (c == Color::White) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        return ch;
    }

    void Board::rebuild_occ()
    {
        occ_[0] = occ_[1] = 0;
        for (int k = 0; k < 6; ++k)
        {
            occ_[0] |= bb_[0][k];
            occ_[1] |= bb_[1][k];
        }
        occ_all_ = occ_[0] | occ_[1];
    }

    std::string Board::to_fen() const
    {
        std::string fen;
        fen.reserve(80);

        // 1) Piece placement (rank 8 to 1)
        for (int r = 7; r >= 0; --r)
        {
            int run = 0;
            for (int f = 0; f < 8; ++f)
            {
                int s = idx(f, r);
                char out = 0;

                // Find which piece sits on s
                if (occ_all_ & bit(s))
                {
                    // Flush run of empties
                    if (run) { fen.push_back(char('0'+run)); run = 0; }
                    // Determine color & kind (white first)
                    if (occ_[int(Color::White)] & bit(s))
                    {
                        // Find which kind
                        for (int k = 0; k < 6; ++k)
                            if (bb_[int(Color::White)][k] & bit(s)) { out = kind_to_char(static_cast<PieceKind>(k),Color::White); break; }
                    } else {
                        for (int k = 0; k < 6; ++k)
                            if (bb_[int(Color::Black)][k] & bit(s)) { out = kind_to_char(static_cast<PieceKind>(k), Color::Black); break; }
                    }
                    fen.push_back(out ? out: '?');
                } else {
                    ++run;
                }
            }
            if (run) fen.push_back(char('0' + run));
            if (r) fen.push_back('/');
        }

        // 2) Side to move
        fen.push_back(' ');
        fen.push_back(stm_ == Color::White ? 'w' : 'b');

        // 3) Castling right
        fen.push_back(' ');
        bool any = false;
        if (castle_[0][0]) { fen.push_back('K'); any = true; }
        if (castle_[0][1]) { fen.push_back('Q'); any = true; }
        if (castle_[1][0]) { fen.push_back('k'); any = true; }
        if (castle_[1][1]) { fen.push_back('q'); any = true; }
        if (!any) fen.push_back('-');

        // 4) En-passant target
        fen.push_back(' ');
        if(ep_sq_ == -1)
        {
            fen.push_back('-');
        } else {
            int file = ep_sq_ & 7;
            int rank = ep_sq_ >> 3;
            fen.push_back(static_cast<char>('a'+file));
            fen.push_back(static_cast<char>('1'+rank));
        }

        // 5) Halfmove clock
        fen.push_back(' ');
        fen += std::to_string(halfmove_clock_);

        // 6) Fullmove number
        fen.push_back(' ');
        fen += std::to_string(fullmove_number_);

        return fen;
    }

    // Remove any piece on sq (if any)
    void Board::clear_square(int sq)
    {
        BB b = bit(sq);
        for (int c = 0; c < 2; ++c)
        {
            for (int k = 0; k < 6; ++k)
            {
                if (bb_[c][k] & b)
                {
                    bb_[c][k] &= ~b;
                    rebuild_occ();
                    return;
                }
            }
        }
    }
}