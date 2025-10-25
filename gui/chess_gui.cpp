#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <algorithm>
#include <string>
#include <cassert>
#include <cctype>
#include <array>

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

struct PieceAtlas {
    // order [color][kind] -> texture
    // color 0 = white, 1 = black
    // kind index 1..6 -> P, N, B, R, Q, K; 0 is unused
    std::array<std::array<sf::Texture,7>,2> tex{};
    bool loaded = false;
    static const char* kindName(int kind)
    {
        switch(kind) {
            case 1: return "Pawn";
            case 2: return "Knight";
            case 3: return "Bishop";
            case 4: return "Rook";
            case 5: return "Queen";
            case 6: return "King";
            default: return "Unknown";
        }
    }

    bool loadFrom(const std::string& baseDir)
    {
        if (loaded) return true;
        for (int col = 0; col < 2; ++col)
        {
            const char cw = (col==0 ? 'W' : 'B');
            for (int k = 1; k <= 6; ++k)
            {
                std::string path = baseDir + "/" + kindName(k) + cw + ".png";
                if (!tex[col][k].loadFromFile(path)) {
                    std::string path2 = baseDir + "/" + std::string(1, std::tolower(kindName(k)[0])) + (kindName(k)+1) + cw + ".png";
                    if (!tex[col][k].loadFromFile(path2)) return false;
                }
                tex[col][k].setSmooth(true);
            }
        }
        loaded = true;
        return true;
    }

    const sf::Texture& get(int color, int kind) const
    {
        return tex[color > 0 ? 0 : 1][kind];
    }
};

} // namespace

// ---------------- BoardView ----------------
class BoardView {
public:
    BoardView(float tile=192.f, float margin=40.f, std::string assets="gui/figures") : tile(tile), margin(margin), assetsDir(std::move(assets)) {
        if(!atlas.loadFrom(assetsDir)) {
            atlas.loadFrom("../" + assetsDir);
        }
    }

    const PieceAtlas& getAtlas() const { return atlas; }

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
        auto draw_one = [&](int sq, sf::Vector2f pos){
            ch::Color c; ch::PieceKind k;
            if (!piece_at(B, sq, c, k)) return;
            int color = (c == ch::Color::White) ? +1 : -1;
            int kindIndex = 0;
            switch (k) {
                case ch::PieceKind::Pawn:   kindIndex = 1; break;
                case ch::PieceKind::Knight: kindIndex = 2; break;
                case ch::PieceKind::Bishop: kindIndex = 3; break;
                case ch::PieceKind::Rook:   kindIndex = 4; break;
                case ch::PieceKind::Queen:  kindIndex = 5; break;
                case ch::PieceKind::King:   kindIndex = 6; break;
                default: return;
            }

            const sf::Texture& tex = atlas.get(color, kindIndex);
            sf::Sprite sprite(tex);

            // scale to exactly one tile (assumes square images or transparent margin)
            auto sz = tex.getSize();
            float sx = tile / static_cast<float>(sz.x);
            float sy = tile / static_cast<float>(sz.y);
            sprite.setScale(sf::Vector2f(sx, sy));
            sprite.setPosition(pos); // top-left of the square

            w.draw(sprite);
        };

        for (int sq=0; sq<64; ++sq) if (!(dragging && *dragging==sq)) {
            draw_one(sq, topLeft(sq));
        }
        if (dragging) {
            auto pos = sf::Vector2f(dragPos.x - tile/2.f, dragPos.y - tile/2.f);
            draw_one(*dragging, pos);
        }
    }

    float tile{192.f};
    float margin{40.f};
private:
    std::string assetsDir;
    PieceAtlas atlas;
};

// --- Sprite-based promotion popup ------------------------------------------
class PromotionPopup {
public:
    // promoCode mapping must match your Move::promo_code(): 3=Q, 2=R, 1=B, 0=N
    struct Choice { int kindIndex; int promoCode; }; // kindIndex: 2=N,3=B,4=R,5=Q

    explicit PromotionPopup(const PieceAtlas* atlas = nullptr) : atlas(atlas) {
        // Default order (left→right): Q R B N
        choices = {
            Choice{5, 3}, // Queen
            Choice{4, 2}, // Rook
            Choice{3, 1}, // Bishop
            Choice{2, 0}, // Knight
        };
    }

    void setAtlas(const PieceAtlas* a) { atlas = a; }

    // Open near 'screenPos' (top-left of destination square), colored by 'side'
    void open(sf::Vector2f squareTL, ch::Color side, float tile, sf::Vector2u winSize) {
        visible = true;
        winner.reset();
        whiteMove = (side == ch::Color::White);

        float pad = tile*0.12f;
        btnSize   = sf::Vector2f(tile*0.85f, tile*0.85f); // a bit larger for sprites
        gap       = tile*0.08f;
        boxSize   = sf::Vector2f(4 * btnSize.x + 3 * gap + 2*pad, btnSize.y + 2*pad);

        // Base position: below for White, above for Black
        const float edgeGap = std::max(8.f, tile * 0.05f);
        const float desiredY = whiteMove
            ? (squareTL.y + tile + edgeGap)                    // below
            : (squareTL.y - boxSize.y - edgeGap);              // above

        // Center horizontally over the square
        float x = squareTL.x + (tile - boxSize.x) * 0.5f;
        float y = desiredY;

        // Clamp inside the window
        auto clampf = [](float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); };
        x = clampf(x, edgeGap, std::max(edgeGap, float(winSize.x) - boxSize.x - edgeGap));
        y = clampf(y, edgeGap, std::max(edgeGap, float(winSize.y) - boxSize.y - edgeGap));

        boxPos = sf::Vector2f(x, y);

        btns.clear();
        sf::Vector2f p = boxPos + sf::Vector2f(pad, pad);
        for (int i=0; i<4; ++i) {
            btns.emplace_back(sf::FloatRect(p, btnSize));
            p.x += btnSize.x + gap;
        }
    }

    void close() { visible = false; winner.reset(); }

    void draw(sf::RenderWindow& w) const {
        if (!visible || !atlas) return;

        // dim background
        sf::RectangleShape dim(sf::Vector2f(float(w.getSize().x), float(w.getSize().y)));
        dim.setFillColor(sf::Color(0,0,0,120));
        w.draw(dim);

        // box
        sf::RectangleShape box(boxSize);
        box.setPosition(boxPos);
        box.setFillColor(sf::Color(245,245,245));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color(40,40,40));
        w.draw(box);

        // buttons with sprites
        for (int i=0; i<4; ++i) {
            // button chrome
            sf::RectangleShape r(sf::Vector2f(btnSize.x, btnSize.y));
            r.setPosition(sf::Vector2f(btns[i].position.x, btns[i].position.y));
            r.setFillColor(sf::Color(230,230,230));
            r.setOutlineThickness(1.5f);
            r.setOutlineColor(sf::Color(90,90,90));
            w.draw(r);

            // sprite for (side, kindIndex)
            int color = whiteMove ? +1 : -1;
            const sf::Texture& tex = atlas->get(color, choices[i].kindIndex);
            sf::Sprite sp(tex);

            // scale to fit nicely inside button (with a little padding)
            auto sz = tex.getSize();
            const float pad = 6.f;
            const float targetW = btnSize.x - 2*pad;
            const float targetH = btnSize.y - 2*pad;
            float sx = targetW / float(sz.x);
            float sy = targetH / float(sz.y);
            float s  = std::min(sx, sy);
            sp.setScale(sf::Vector2f(s, s));

            // center sprite in button
            const float bx = btns[i].position.x;
            const float by = btns[i].position.y;
            const float sw = float(sz.x) * s;
            const float sh = float(sz.y) * s;
            sp.setPosition(sf::Vector2f(bx + (btnSize.x - sw)/2.f,
                                        by + (btnSize.y - sh)/2.f));

            w.draw(sp);
        }
    }

    bool handleClick(sf::Vector2f mouse) {
        if (!visible) return false;
        for (int i=0; i<4; ++i) {
            if (btns[i].contains(mouse)) {
                winner = choices[i].promoCode; // 3/2/1/0
                visible = false;
                return true;
            }
        }
        // Consume clicks while visible (prevents interacting with board under it)
        return true;
    }

    bool hasWinner() const { return winner.has_value(); }
    int  winnerPromoCode() const { return *winner; }
    bool isVisible() const { return visible; }

private:
    const PieceAtlas* atlas = nullptr; // provided by BoardView

    bool visible = false;
    bool whiteMove = true;

    sf::Vector2f boxPos{}, boxSize{}, btnSize{};
    float gap = 8.f;

    std::vector<sf::FloatRect> btns;
    std::array<Choice, 4> choices;
    std::optional<int> winner;
};

// ---------------- Controller ----------------
class Controller {
public:
    Controller(ch::Board& B) : board(B) {
        promo.setAtlas(&view.getAtlas());
    }
    
    void handle(const sf::Event& e) {
        if (const auto* mm = e.getIf<sf::Event::MouseMoved>()) {
            mouse = sf::Vector2f(float(mm->position.x), float(mm->position.y));
            return;
        }

        // If a promotion choice is pending, consume mouse clicks for the popup
        if (awaitingPromotion) {
            if (const auto* mbp = e.getIf<sf::Event::MouseButtonPressed>()) {
                if (mbp->button == sf::Mouse::Button::Left) {
                    promo.handleClick(sf::Vector2f(float(mbp->position.x), float(mbp->position.y)));
                    if (promo.hasWinner()) {
                        // choose the candidate with that promo_code()
                        int want = promo.winnerPromoCode(); // 3=Q,2=R,1=B,0=N
                        ch::Move mv = pendingPromoMoves.front();
                        for (const auto& m : pendingPromoMoves) if (m.promo_code() == want) { mv = m; break; }

                        ch::State st{};
                        history.push_back(st);
                        ch::make_move(board, mv, history.back());
                        played.push_back(mv);
                        lastMove = std::pair{mv.from(), mv.to()};

                        awaitingPromotion = false;
                        pendingPromoMoves.clear();
                        pendingFrom = pendingTo = -1;
                    }
                }
                return; // block board input while popup shown
            }

            // Block other inputs too while the popup is visible
            return;
        }

        // Normal input path when no popup is showing
        if (const auto* mbp = e.getIf<sf::Event::MouseButtonPressed>()) {
            if (mbp->button == sf::Mouse::Button::Left) onDown();
            return;
        }
        if (const auto* mbr = e.getIf<sf::Event::MouseButtonReleased>()) {
            if (mbr->button == sf::Mouse::Button::Left) onUp();
            return;
        }
        if (const auto* kp = e.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::U) onUndo();
            return;
        }
    }

    void draw(sf::RenderWindow& w) {
        windowSize = w.getSize();
        view.drawBoard(w, theme, lastMove);
        if (selected >= 0) view.drawLegalDots(w, theme, legalTargets);
        view.drawPieces(w, board, dragging? std::optional<int>(dragFrom) : std::nullopt, mouse);

        if(awaitingPromotion)
            promo.draw(w);
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

        // Collect all legal moves that match the drag from->to
        std::vector<ch::Move> cands;
        for (const auto& m : cachedMoves)
            if (m.from() == selected && m.to() == to)
                cands.push_back(m);

        if (cands.empty()) { resetSel(); return; }

        // ---- Promotion case: show popup and defer making the move ----
        // If there are multiple candidates differentiated by promo_code (0..3),
        // we open the popup and let the user choose N/B/R/Q.
        bool hasPromotionVariants = false;
        for (const auto& m : cands) {
            // Your Move::promo_code() returns 0..3 for N/B/R/Q, and something else for non-promo
            if (m.promo_code() >= 0 && m.promo_code() <= 3) { hasPromotionVariants = true; break; }
        }

        if (hasPromotionVariants && cands.size() > 1) {
            awaitingPromotion = true;
            pendingPromoMoves = cands;
            pendingFrom = selected;
            pendingTo   = to;

            promoMovesForButtons = {};
            const int wantKindIdx[4] = {5, 4, 3, 2};

            for (const auto& m : pendingPromoMoves) {
                ch::Board tmp = board;
                ch::State st{};
                ch::make_move(tmp,m,st);
                ch::Color rc; ch::PieceKind rk;
                if(!piece_at(tmp,pendingTo,rc,rk)) continue;

                int kindIdx = 0;
                switch(rk) {
                    case ch::PieceKind::Pawn: kindIdx = 1; break;
                    case ch::PieceKind::Knight: kindIdx = 2; break;
                    case ch::PieceKind::Bishop: kindIdx = 3; break;
                    case ch::PieceKind::Rook: kindIdx = 4; break;
                    case ch::PieceKind::Queen: kindIdx = 5; break;
                    default: continue;
                }

                for (int i = 0; i < 4; ++i) if (kindIdx == wantKindIdx[i] && !promoMovesForButtons[i]) {
                    promoMovesForButtons[i] = m;
                    break;
                }
            }

            // Place popup near the destination square
            auto TL = view.topLeft(to);
            promo.open(sf::Vector2f(TL.x, TL.y), board.side_to_move(),view.tile,windowSize);

            resetSel();              // clear highlight/drag state
            return;                  // wait for user to click a sprite in the popup
        }

        // ---- Normal move (no promotion choice required) ----
        // Prefer a special move (e.g., castling) if there are multiple
        ch::Move mv = cands.front();
        for (const auto& m : cands) if (m.is_special()) { mv = m; break; }

        ch::State st{};
        history.push_back(st);
        ch::make_move(board, mv, history.back());
        played.push_back(mv);
        lastMove = std::pair{ mv.from(), mv.to() };

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

    bool awaitingPromotion = false;
    std::vector<ch::Move> pendingPromoMoves;
    int pendingFrom = -1, pendingTo = -1;
    PromotionPopup promo;
    std::array<std::optional<ch::Move>,4> promoMovesForButtons;

    sf::Vector2u windowSize{0,0};

    ch::Board& board;
    std::vector<ch::State> history;
    std::vector<ch::Move>  played;

    std::vector<ch::Move> cachedMoves;
    std::vector<int> legalTargets;
    std::optional<std::pair<int,int>> lastMove;
};

int main() {
    ch::init_bitboards();
    ch::Board engineBoard;
    ch::Board guiBoard; guiBoard.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // replace with your start FEN helper if you have one

    sf::RenderWindow win(sf::VideoMode(sf::Vector2u{1720u, 1720u}), "ch GUI (SFML)");
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