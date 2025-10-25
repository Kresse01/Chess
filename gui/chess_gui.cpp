#include <SFML/Graphics.hpp>
#include <algorithm>
#include <optional>
#include <vector>
#include <cmath>
#include <array>
#include <string>
#include <cassert>

// Helpers for coordinates and visuals
struct BoardTheme
{
    sf::Color ligh{240, 217, 181};
    sf::Color dark{181, 136, 99};
    sf::Color highlight{255, 255, 0, 120};
    sf::Color moveDot{50, 50, 50, 160};
    sf::Color lastMove{100, 200, 100, 120};
    sf::Color check{255, 100, 100, 140};
};

static inline int sq_from_fr(int file, int rank) { return rank * 8 + file; }
static inline int file_of(int sq) { return sq % 8; }
static inline int rank_of(int sq) { return sq / 8; }

// BoardView draws the board and pieces, and supports simple animation
class BoardView
{
public:
    BoardView(float title = 96.f, float margin = 20.f)
        : tileSize(tile), margin(margin), boardSize(8*tile) {}

    sf::vector2f squareTopLeft(int sq) const
    {
        return {margin + file_of(sq)*tileSize, margin+(7-rank_of(sq))*tileSize };
        // Note: GUI draws rank 8 at top, If you want white at bottom, invert rank as above
    }

    int squareAtPixel(sf::Vector2f p) const
    {
        float x = p.x -margin, y=p.y - margin;
        if(x < 0 || y < 0) return -1;
        int f = int(x/tileSize), r_from_top = int(y/tileSize);
        if (f < 0 || f > 7 || r_from_top < 0 || r_from_top > 7) return -1;
        int r = 7 - r_from_top; // invert
        return sq_from_fr(f, r);
    }

    void drawBoard(sf::RenderWindow& win, const BoardTheme& theme,
                   std::optional<std::pair<int,int>> lastMove = std::nullopt,
                   std::optional<int> inCheckSq = std::nullopt) const
    {
        // Squares
        for (int r = 0; r < 8; ++r)
        {
            for (int f = 0; f < 8; ++f)
            {
                int sq = sq_from_fr(f,r);
                sf::RectangleShape rect({tileSize,tileSize});
                rect.setPosition(squareTopLeft(sq));
                bool dark = ((r+f)%2) != 0;
                rect.setFillColor(dark ? theme.dark : theme.light);
                win.draw(rect);
            }
        }
        // Last move highlight
        if (lastMove)
        {
            for (int sq : { lastMove->first, lastMove->second })
            {
                sf::RectangleShape hl({tileSize, tileSize});
                hl.setPosition(squareTopLeft(sq));
                hl.setFillColor(theme.lastMove);
                win.draw(hl);
            }
        }

        // check square highlight
        if (inCheckSq) {
            sf::RectangleShape hl({tileSize,tileSize});
            hl.setPosition(squareTopLeft(*inCheckSq));
            hl.setFillColor(theme.check);
            win.draw(hl);
        }
    }

    void drawLegalMoves(sf::RenderWindow& win, const BoardTheme& theme,
                        const std::vector<int>& targets) const
    {
        for (int sq : targets)
        {
            auto tl = squareTopLeft(sq);
            sf::CircleShape dot(tileSize * 0.12f);
            dot.setFillColor(theme.moveDot);
            dot.setOrigin(dot.getRadius(), dot.getRadius());
            dot.setPosition(tl.x + tileSize/2.f, tl.y + tileSize/2.f);
            win.draw(dot);
        }
    }

    // Very simple piece rendering: letters. Replace with sprites when you add artwork
    void drawPieces(sf::RenderWindow& win, const ch::State& st, std::optional<int> draggingSq, sf::Vector2f dragPos) const
    {
        sf::Font font;
        // Load a system font path if you like; here we use SFML default if available.
        // For reliability, ship a font file in your repo and load it once.
        static bool loaded = font.loadFromFile("DejaVuSans.ttf"); (void) loaded;
        for (int sq = 0; sq < 64; ++sq)
        {
            if (draggingSq && *draggingSq == sq) continue; // draw dragged piece
            auto p = st.piece_at(sq);
            if (p.kind == ch::EMPTY) continue;
            drawPieceAt(win, font, p, squareTopLeft(sq));
        }

        if (draggingSq)
        {
            auto p = st.piece_at(*draggingSq);
            if (p.kind != chess::EMPTY)
            {
                sf::Vector2f topLeft = { dragPos.x - tileSize/2.f, dragPos.y - tileSize/2.f };
                drawPieceAt(win, font, p, topLeft);
            }
        }
    }

private:
    void drawPieceAt(sf::RenderWindow& win, sf::Font& font,
                     const ch::Piece& pc, sf::Vector2f topLeft) const
    {
        sf::RectangleShape bg({tileSize, tileSize});
        bg.setPosition(topLeft);
        bg.setFillColor(sf::Color(0,0,0,0)); // transparent (kept for hitbox if needed)
        win.draw(bg);

        sf::Text t;
        t.setFont(font);
        t.setCharacterSize(static_cast<unsigned>(tileSize*0.7f));
        t.setFillColor(pc.color == chess::White ? sf::Color::White : sf::Color::Black);
        t.setOutlineThickness(2.f);
        t.setOutlineColor(pc.color == chess::White ? sf::Color::Black : sf::Color::White);
        t.setString(pieceLetter(pc));

        // center in the square
        st::FloatRect bounds = t.getLocalBounds();
        t.setPosition(topLeft.x + (tileSize - bounds.width)/2.f - bounds.left,
                      topLeft.y + (tileSize - bounds.height)/2.f - bounds.top);
        win.draw(t);
    }

    sf::String pieceLetter(const ch::Piece& pc) const
    {
        switch(pc.kind)
        {
            case ch::PAWN: return "P";
            case ch::KNIGHT: return "N";
            case ch::BISHOP: return "B";
            case ch::ROOK: return "R";
            case ch::QUEEN: return "Q";
            case ch::KING: return "K";
            default: return "";
        }
    }
public:
    float tileSize;
    float margin;
    float boardSize;
};

// Controller: handles input, selection, generating legal moves, and committing engine moves
class GuiController
{
public:
    GuiController(ch::State& s, BoardView& v) : st(s), view(v) {}
    
    void handleEvent(const sf::Event& e, sf::RenderWindow& win)
    {
        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            onMouseDown(win);
        } else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
            onMouseUp(win);
        } else if (e.type == sf::Event::MouseMoved) {
            mousePos = sf::Vector2f(static_cast<float>(e.mouseMove.x), static_cast<float>(e.mouseMove.y));
        }
    }

    void draw(sf::RenderWindow& win)
    {
        std::optional<int> checkSq = std::nullopt; // you can compute if king in check
        view.drawBoard(win, theme, lastMove, checkSq);

        if (selectSq >= 0)
        {
            view.drawLegalMoves(win,theme,legalTargets);
        }
        view.drawPieces(win, st, dragging ? std::optional<int>(dragFromSq) : std::nullopt, mousePos);
    }

private:
    void onMouseDown(sf::RenderWindow& win)
    {
        int sq = view.squareAtPixel(mousePos);
        int (sq < 0) return;
        auto p = st.piece_at(sq);
        if (p.color != st.side_to_move())
        {
            // Allow picking own side only (you can relax for analysis)
            return;
        }

        selectedSq = sq;
        dragging = true;
        dragFromSq = sq;

        //cache legal moves targeting from selected square
        legalTargets.clear();
        cachedMoves = st.legal_moves(); // call your generator once
        for(auto& m : cachedMoves) if (m.from == selectedSq) legalTargets.push_back(m.to);
    }

    void onMouseUp(sf::RenderWindow& win)
    {
        if (!dragging) return;
        dragging = false;
        int targetSq = view.squareAtPixel(mousePos);
        if (targetSq < 0) {selectedSq = -1; return }

        // find a legal move matching from->to
        auto it = std::find_if(cachedMoves.begin(), cachedMoves.end(),
                    [&](const ch::Move& m) {return m.from == selectedSq && m.to == targetSq; });

        if (it != cachedMoves.end()) {
            // promotion UI: if needed, pop a small modal to pick a piece, then set it->promotion
            // For now we just pass the move through
            if (st.make_move(*it)) {
                lastMove = std::pair<int,int>{ it->from, it->to };
            }
        }

        selectedSq = -1;
        legalTargets.clear();
        cachedMoves.clear();
    }

    ch::State& st;
    BoardView& view;
    BoardTheme theme;

    sf::Vector2f mousePos(0.f,0.f);

    bool dragging = false;
    int drawFromSq = -1;
    int selectedSq = -1;
    std::vector<ch::Move> cachedMoves;
    std::vector<int> legalTargets;
    std::optional<std::pair<int,int>> lastMove = std::nullopt;
};

int main()
{
    const int windowW = 800, windowH = 800;
    sf::RenderWindow window(sf::VideoMode(windowW, windowH), "Chess GUI (C++ / SFML)");
    window.setFramerateLimit(60);

    ch::State state; // <---- use your real state (initial FEN etc.)
    BoardView view(90.f, 30.f);
    GuiController gui(state,view);

    while(window.isOpen())
    {
        sf::Event e;
        while(window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) window.close();
            gui.handleEvent(e,window);
        }

        window.clear(sf::Color(30,30,30));
        gui.draw(window);
        window.display();
    }
    return 0;
}