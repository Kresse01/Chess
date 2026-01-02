#include "chess/analysis/ch_pins.h"

#include "chess/core/ch_board.h"
#include "chess/core/ch_bitboard.h"

namespace ch
{
    namespace
    {
        // Step from sq by dir, but stop if we wrap across files.
        // Returns next square index or -1 if stepping leaves the board.
        inline int step_sq(int sq, Dir dir) noexcept
        {
            const int to = sq + static_cast<int>(dir);

            if (to < 0 || to >= 64) return -1;

            // File-wrap prevention for E/W and diagonals
            const int f0 = sq & 7;
            const int f1 = to & 7;
            const int df = f1 - f0;

            switch (dir)
            {
                case E:  if (df != 1)  return -1; break;
                case W:  if (df != -1) return -1; break;
                case NE: if (df != 1)  return -1; break;
                case NW: if (df != -1) return -1; break;
                case SE: if (df != 1)  return -1; break;
                case SW: if (df != -1) return -1; break;
                case N:
                case S:
                default: break;
            }
            return to;
        }

        inline bool is_slider_pinner(PieceKind k, Dir dir) noexcept
        {
            const bool diag = (dir == NE || dir == NW || dir == SE || dir == SW);
            if (k == PieceKind::Queen) return true;
            if (diag) return k == PieceKind::Bishop;
            return k == PieceKind::Rook;
        }

        inline bool piece_on(const Board& b, int sq, Color c, PieceKind k) noexcept
        {
            return (b.bb(c, k) & bit(sq)) != 0;
        }
    } // namespace

    Pins compute_pins(const Board&b, Color side)
    {
        Pins out{};

        const Color them = opposite(side);
        const BB kingBB  = b.bb(side, PieceKind::King);
        if (!kingBB) return out;

        const int ks = lsb(kingBB);

        const Dir dirs[8] = { N, S, E, W, NE, NW, SE, SW };

        for (Dir dir : dirs)
        {
            int sq = ks;
            int first_friend = -1;

            // Walk outwards
            while (true)
            {
                sq = step_sq(sq, dir);
                if (sq < 0) break;

                const BB m = bit(sq);
                if (!(b.occ_all() & m))
                    continue; // empty

                // Occupied:
                if (b.occ(side) & m)
                {
                    // First friendly piece could be pinned; second friendly breaks the ray.
                    if (first_friend == -1) first_friend = sq;
                    else break;
                }
                else
                {
                    // Enemy piece. If we have a candidate pinned piece and this enemy is a pinner, record.
                    if (first_friend != -1)
                    {
                        PieceKind enemyKind = PieceKind::None;
                        for (int k = 0; k < 6; ++k)
                        {
                            if (piece_on(b, sq, them, static_cast<PieceKind>(k)))
                            {
                                enemyKind = static_cast<PieceKind>(k);
                                break;
                            }
                        }

                        if (enemyKind != PieceKind::None && is_slider_pinner(enemyKind, dir))
                        {
                            out.pinned |= bit(first_friend);

                            // Closed segment king..enemy (includes endpoints)
                            BB seg = between_mask(ks, sq) | bit(ks) | bit(sq);
                            out.ray_to_enemy[first_friend] = seg;
                        }
                    }
                    break; // ray blocked by enemy piece regardless
                }
            }
        }

        return out;
    }
} // namespace ch