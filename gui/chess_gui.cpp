#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <algorithm>
#include <string>
#include <cassert>
#include <cctype>

#include "chess/core/ch_board.h"
#include "chess/core/ch_move.h"
#include "chess/core/ch_state.h"
#include "chess/gen/ch_movegen.h"
#include "chess/core/ch_types.h"

namespace {

struct Theme {
    sf::Color light{240,217,181}, dark{181,136,99};
    sf::Color last{120,200,120,120}, dot{50,50,50,160}, check{255,80,80,140};
};

inline int file_of(int s){ return s & 7; }
inline int rank_of(int s){ return s >> 3; }
inline int idx(int f, int r){ return (r<<3) | f; }

static bool piece_at(const ch::Board& b, int sq, ch::Color& outC, ch::PieceKind& outK) {
    ch::BB mask = ch::bit(sq);
    if (!(b.occ_all() & mask)) return false;
    if (b.occ(ch::Color::White) & mask) {
        outC = ch::Color::White;
        for (int k=0;k<6;++k) if (b.bb(outC, ch::PieceKind(k)) & mask) { outK = ch::PieceKind(k); return true; }
    } else {
        outC = ch::Color::Black;
        for (int k=0;k<6;++k) if (b.bb(outC, ch::PieceKind(k)) & mask) { outK = ch::PieceKind(k); return true; }
    }
    return false;
}

static const char* letter(ch::PieceKind k) {
    switch (k) {
        case ch::PieceKind::Pawn:   return "P";
        case ch::PieceKind::Knight: return "N";
        case ch::PieceKind::Bishop: return "B";
        case ch::PieceKind::Rook:   return "R";
        case ch::PieceKind::Queen:  return "Q";
        case ch::PieceKind::King:   return "K";
        default: return "?";
    }
}

} // namespace

// ---------------- BoardView ----------------
class BoardView {
public:
    BoardView(float tile=96.f, float margin=20.f) : tile(tile), margin(margin) {}

    int squareAt(sf::Vector2f p) const {
        float x = p.x - margin, y = p.y - margin;
        if (x < 0 || y < 0) return -1;
        int f = int(x / tile), rtop = int(y / tile);
        if (f < 0 || f > 7 || rtop < 0 || rtop > 7) return -1;
        int r = 7 - rtop;
        return idx(f,r);
    }

    sf::Vector2f topLeft(int sq) const {
        return { margin + float(file_of(sq))*tile, margin + float(7 - rank_of(sq))*tile };
    }

    void drawBoard(sf::RenderWindow& w, const Theme& t,
                   std::optional<std::pair<int,int>> lastMove = std::nullopt) const
    {
        for (int r=0;r<8;++r) for (int f=0;f<8;++f) {
            int sq = idx(f,r);
            sf::RectangleShape s({tile,tile});
            s.setPosition(topLeft(sq));
            s.setFillColor(((r+f)&1) ? t.dark : t.light);
            w.draw(s);
        }
        if (lastMove) {
            for (int sq: {lastMove->first, lastMove->second}) {
                sf::RectangleShape hl({tile,tile});
                hl.setPosition(topLeft(sq));
                hl.setFillColor(t.last);
                w.draw(hl);
            }
        }
    }

    void drawLegalDots(sf::RenderWindow& w, const Theme& t,
                       const std::vector<int>& targets) const
    {
        for (int sq : targets) {
            sf::CircleShape c(tile*0.12f);
            c.setOrigin(sf::Vector2f(c.getRadius(), c.getRadius()));
            auto TL = topLeft(sq);
            c.setPosition(sf::Vector2f(TL.x + tile/2.f, TL.y + tile/2.f));
            c.setFillColor(t.dot);
            w.draw(c);
        }
    }

    void drawPieces(sf::RenderWindow& w, const ch::Board& B,
                    std::optional<int> dragging, sf::Vector2f dragPos) const
    {
        static sf::Font font; static bool loaded=false;
        if (!loaded) loaded = font.openFromFile("C:/Users/gusta/Desktop/Chess/gui/DejaVuSans.ttf");

        auto draw_one = [&](int sq, sf::Vector2f pos){
            ch::Color c; ch::PieceKind k;
            if (!piece_at(B, sq, c, k)) return;

            // In SFML 3, provide font in constructor
            sf::Text t(font, letter(k), unsigned(tile*0.7f));
            bool white = (c == ch::Color::White);
            t.setFillColor(white ? sf::Color::White : sf::Color::Black);
            t.setOutlineThickness(2.f);
            t.setOutlineColor(white ? sf::Color::Black : sf::Color::White);

            // Center text inside tile using new bounds API: position + size
            const auto lb = t.getLocalBounds();
            const sf::Vector2f origin(lb.position.x + lb.size.x/2.f,
                                      lb.position.y + lb.size.y/2.f);
            t.setOrigin(origin);
            t.setPosition(sf::Vector2f(pos.x + tile/2.f, pos.y + tile/2.f));

            w.draw(t);
        };

        for (int sq=0; sq<64; ++sq) if (!(dragging && *dragging==sq)) {
            draw_one(sq, topLeft(sq));
        }
        if (dragging) {
            auto pos = sf::Vector2f(dragPos.x - tile/2.f, dragPos.y - tile/2.f);
            draw_one(*dragging, pos);
        }
    }

    float tile{96.f};
    float margin{20.f};
};

// ---------------- Controller ----------------
class Controller {
public:
    Controller(ch::Board& B) : board(B) {}

    void handle(const sf::Event& e) {
        if (const auto* mm = e.getIf<sf::Event::MouseMoved>()) {
            mouse = sf::Vector2f(float(mm->position.x), float(mm->position.y));
        } else if (const auto* mbp = e.getIf<sf::Event::MouseButtonPressed>()) {
            if (mbp->button == sf::Mouse::Button::Left) onDown();
        } else if (const auto* mbr = e.getIf<sf::Event::MouseButtonReleased>()) {
            if (mbr->button == sf::Mouse::Button::Left) onUp();
        } else if (const auto* kp = e.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::U) onUndo();
        }
    }

    void draw(sf::RenderWindow& w) {
        view.drawBoard(w, theme, lastMove);
        if (selected >= 0) view.drawLegalDots(w, theme, legalTargets);
        view.drawPieces(w, board, dragging? std::optional<int>(dragFrom) : std::nullopt, mouse);
    }

    void setFromFEN(const std::string& fen) {
        board.set_fen(fen.c_str());
        history.clear(); lastMove.reset(); played.clear();
    }
    std::string fen() const { return board.to_fen(); }

private:
    void onDown() {
        int sq = view.squareAt(mouse);
        if (sq < 0) return;

        ch::Color c; ch::PieceKind k;
        if (!piece_at(board, sq, c, k)) return;
        if (c != board.side_to_move()) return;

        cachedMoves.clear(); legalTargets.clear();
        ch::generate_legal_moves(board, board.side_to_move(), cachedMoves);
        for (auto& m : cachedMoves) if (m.from() == sq) legalTargets.push_back(m.to());

        selected = sq; dragging = true; dragFrom = sq;
    }

    void onUp() {
        if (!dragging) return;
        dragging = false;

        int to = view.squareAt(mouse);
        if (to < 0) { resetSel(); return; }

        std::vector<ch::Move> cands;
        for (auto& m : cachedMoves) if (m.from()==selected && m.to()==to) cands.push_back(m);

        if (!cands.empty()) {
            ch::Move mv = choosePromotion(cands);
            ch::State st{};
            history.push_back(st);
            ch::make_move(board, mv, history.back());
            played.push_back(mv);
            lastMove = std::pair{mv.from(), mv.to()};
        }

        resetSel();
    }

    void onUndo() {
        if (history.empty() || played.empty()) return;
        const ch::Move mv = played.back(); played.pop_back();
        const ch::State st = history.back(); history.pop_back();
        ch::unmake_move(board, mv, st);
        lastMove.reset();
    }

    ch::Move choosePromotion(const std::vector<ch::Move>& cands) {
        if (cands.size()==1) return cands[0];
        auto it = std::find_if(cands.begin(), cands.end(),
                               [](const ch::Move& m){ return m.promo_code()==3; }); // prefer Queen
        return (it!=cands.end()? *it : cands.front());
    }

    void resetSel() { selected=-1; legalTargets.clear(); cachedMoves.clear(); }

    BoardView view{};
    Theme theme{};

    sf::Vector2f mouse{0.f,0.f};
    bool dragging=false;
    int dragFrom=-1, selected=-1;

    ch::Board& board;
    std::vector<ch::State> history;
    std::vector<ch::Move>  played;

    std::vector<ch::Move> cachedMoves;
    std::vector<int> legalTargets;
    std::optional<std::pair<int,int>> lastMove;
};

int main() {
    ch::Board engineBoard;
    ch::Board guiBoard; guiBoard.set_fen("rnbaqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBAQKBNR w KQkq - 0 1"); // replace with your start FEN helper if you have one

    sf::RenderWindow win(sf::VideoMode(sf::Vector2u{860u, 860u}), "ch GUI (SFML)");
    win.setFramerateLimit(60);

    Controller controller(guiBoard);

    while (win.isOpen()) {
        while (auto ev = win.pollEvent()) {
            const sf::Event& e = *ev;
            if (e.is<sf::Event::Closed>()) { win.close(); break; }
            controller.handle(e);
        }
        win.clear(sf::Color(30,30,30));
        controller.draw(win);
        win.display();
    }
    return 0;
}