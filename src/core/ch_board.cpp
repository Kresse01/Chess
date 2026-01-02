#include "chess/core/ch_board.h"

#include <cstring>
#include <cctype>
#include <string>

namespace ch
{
    namespace
    {
        // Safer than returning Pawn on invalid input.
        inline bool try_char_to_kind(char p, PieceKind& out) noexcept
        {
            switch (std::tolower(static_cast<unsigned char>(p)))
            {
                case 'p': out = PieceKind::Pawn; return true;
                case 'n': out = PieceKind::Knight; return true;
                case 'b': out = PieceKind::Bishop; return true;
                case 'r': out = PieceKind::Rook;   return true;
                case 'q': out = PieceKind::Queen;  return true;
                case 'k': out = PieceKind::King;   return true;
                default:  return false;
            }
        }

        inline char kind_to_char(PieceKind k, Color c) noexcept
        {
            char ch = 'p';
            switch (k)
            {
                case PieceKind::Pawn: ch = 'p'; break;
                case PieceKind::Knight: ch = 'n'; break;
                case PieceKind::Bishop: ch = 'b'; break;
                case PieceKind::Rook:   ch = 'r'; break;
                case PieceKind::Queen:  ch = 'q'; break;
                case PieceKind::King:   ch = 'k'; break;
            }
            if (c == Color::White) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            return ch;
        }

        // Parse a non-negative integer starting at p; advances p.
        inline bool parse_uint(const char*& p, unsigned& out) noexcept
        {
            if (!p || !std::isdigit(static_cast<unsigned char>(*p)))
                return false;

            unsigned v = 0;
            while (*p && std::isdigit(static_cast<unsigned char>(*p)))
            {
                v = v * 10u + static_cast<unsigned>(*p - '0');
                ++p;
            }
            out = v;
            return true;
        }
    } // namespace

    void Board::clear()
    {
        std::memset(bb_, 0, sizeof(bb_));
        occ_[0] = occ_[1] = 0;
        occ_all_ = 0;

        castle_[0][0] = castle_[0][1] = false;
        castle_[1][0] = castle_[1][1] = false;

        ep_sq_ = -1;
        stm_ = Color::White;

        halfmove_clock_ = 0;
        fullmove_number_ = 1;
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

    void Board::set_startpos()
    {
        // Standard chess start position FEN.
        set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

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
                if (f > 7 || r < 0) return false;

                const Color col = std::isupper(static_cast<unsigned char>(c)) ? Color::White : Color::Black;

                PieceKind kind{};
                if (!try_char_to_kind(c, kind))
                    return false;

                // place piece
                bb_[static_cast<int>(col)][static_cast<int>(kind)] |= bit(idx(f, r));
                ++f;
                continue;
            }

            // invalid character in placement
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
                const char c = *p++;
                if (c == 'K') castle_[static_cast<int>(Color::White)][0] = true;
                else if (c == 'Q') castle_[static_cast<int>(Color::White)][1] = true;
                else if (c == 'k') castle_[static_cast<int>(Color::Black)][0] = true;
                else if (c == 'q') castle_[static_cast<int>(Color::Black)][1] = true;
                else return false;
            }
        }

        if (*p == ' ') ++p;

        // 4) En-passant square
        if (*p=='-') { ep_sq_ = -1; ++p;}
        else {
            if (!std::isalpha(static_cast<unsigned char>(p[0])) ||
                !std::isdigit(static_cast<unsigned char>(p[1])))
                return false;
            
            const int file = std::tolower(static_cast<unsigned char>(p[0])) - 'a';
            const int rank = p[1] - '1';

            if (static_cast<unsigned>(file) >= 8u || static_cast<unsigned>(rank) >= 8u)
                return false;
            
            ep_sq_ = idx(file, rank);
            p += 2;
        }

        // Optional: halfmove and fullmove if present
        if (*p == ' ') ++p;

        if (*p)
        {
            unsigned hm = 0;
            if (!parse_uint(p, hm)) return false;
            halfmove_clock_ = static_cast<std::uint16_t>(hm);

            if (*p == ' ') ++p;

            unsigned fm = 0;
            if(!parse_uint(p, fm)) return false;
            fullmove_number_ = static_cast<std::uint32_t>(fm);
        }

        return true;
    }

    std::string Board::to_fen() const
    {
        std::string fen;
        fen.reserve(80);

        for (int r = 7; r >= 0; --r)
        {
            int run = 0;
            for (int f = 0; f < 8; ++f)
            {
                const int sq = idx(f, r);
                const BB b = bit(sq);

                bool found = false;
                for (int c = 0; c < 2 && !found; ++c)
                {
                    for (int k = 0; k < 6 && !found; ++k)
                    {
                        if (bb_[c][k] & b)
                        {
                            if (run) { fen.push_back(char('0' + run)); run = 0; }
                            fen.push_back(kind_to_char(static_cast<PieceKind>(k), static_cast<Color>(c)));
                            found = true;
                        }
                    }
                }

                if (!found) ++run;
            }

            if (run) fen.push_back(char('0' + run));
            if (r) fen.push_back('/');
        }

        // 2) Side to move
        fen.push_back(' ');
        fen.push_back(stm_ == Color::White ? 'w' : 'b');

        // 3) Castling rights
        fen.push_back(' ');
        bool any = false;
        if (castle_[0][0]) { fen.push_back('K'); any = true; }
        if (castle_[0][1]) { fen.push_back('Q'); any = true; }
        if (castle_[1][0]) { fen.push_back('k'); any = true; }
        if (castle_[1][1]) { fen.push_back('q'); any = true; }
        if (!any) fen.push_back('-');

        // 4) En-passant target
        fen.push_back(' ');
        if (ep_sq_ < 0) fen.push_back('-');
        else
        {
            fen.push_back(static_cast<char>('a' + file_of(ep_sq_)));
            fen.push_back(static_cast<char>('1' + rank_of(ep_sq_)));
        }

        // 5) Halfmove / 6) Fullmove
        fen.push_back(' ');
        fen += std::to_string(halfmove_clock_);
        fen.push_back(' ');
        fen += std::to_string(fullmove_number_);

        return fen;
    }

    void Board::clear_square(int sq)
    {
        const BB b = bit(sq);
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
} // namespace ch