#include "chess/core/ch_bitboard.h"

#include <cstdlib> // std::abs

namespace ch
{
    //=========== Storage for globals declared in the header ===============
    BB FILE_MASK[8];
    BB RANK_MASK[8];
    BB DIAG_A1H8[15], DIAG_A8H1[15];
    BB KNIGHT_ATK[64];
    BB KING_ATK[64];

    static inline bool on_board_fr(int f, int r)
    {
        return (unsigned)f < 8u && (unsigned)r < 8u;
    }

    static inline int to_sq(int f, int r) { return (r << 3) | f; }

    // For E/W and diagonals, ensure we didn't wrap across the board
    static inline bool step_respects_edge(int prev, int now, Dir dir)
    {
        // Vertical moves (N/S) don't change file; no wrap possible.
        if (dir == N || dir == S) return true;

        const int pf = file_of(prev), nf = file_of(now);
        const int df = std::abs(nf - pf);

        // Horizontal (E/W): file must change by exactly 1 each step.
        if (dir == E || dir == W) return df == 1;

        // Diagonals: file must also change by exactly 1 each step.
        return df == 1;
    }

    // ================= Precompute masks/tables ==================
    static void build_files_ranks()
    {
        for (int f = 0; f < 8; ++f)
        {
            BB m = 0;
            for (int r = 0; r < 8; ++r) m |= (1ull << to_sq(f, r));
            FILE_MASK[f] = m;
        }

        for (int r = 0; r < 8; ++r)
        {
            BB m = 0;
            for (int f = 0; f < 8; ++f) m |= (1ull << to_sq(f, r));
            RANK_MASK[r] = m;
        }
    }

    static void build_diagonals()
    {
        // A1-H8: index by (file + rank) in [0..14]
        for (int i = 0; i < 15; ++i) DIAG_A1H8[i] = 0;
        for (int r = 0; r < 8; ++r)
            for (int f = 0; f < 8; ++f)
                DIAG_A1H8[f + r] |= (1ull << to_sq(f, r));

        // A8-H1: index by (file - rank + 7) in [0..14]
        for (int i = 0; i < 15; ++i) DIAG_A8H1[i] = 0;
        for (int r = 0; r < 8; ++r)
            for (int f = 0; f < 8; ++f)
                DIAG_A8H1[f - r + 7] |= (1ull << to_sq(f, r));
    }

    static void build_knight_king()
    {
        // Knight moves are (+-1, +-2) and (+-2, +-1)
        const int kdf[8] = {+1, +2, +2, +1, -1, -2, -2, -1};
        const int kdr[8] = {+2, +1, -1, -2, -2, -1, +1, +2};

        for (int r = 0; r < 8; ++r)
        {
            for (int f = 0; f < 8; ++f)
            {
                const int s = to_sq(f, r);

                BB km = 0;
                for (int i = 0; i < 8; ++i)
                {
                    const int nf = f + kdf[i], nr = r + kdr[i];
                    if (on_board_fr(nf, nr)) km |= (1ull << to_sq(nf, nr));
                }
                KNIGHT_ATK[s] = km;

                // King: all (df, dr) with |df|<=1, |dr|<=1, not (0,0)
                BB gm = 0;
                for (int df = -1; df <= 1; ++df)
                {
                    for (int dr = -1; dr <= 1; ++dr)
                    {
                        if (df == 0 && dr == 0) continue;
                        const int nf = f + df, nr = r + dr;
                        if (on_board_fr(nf, nr)) gm |= (1ull << to_sq(nf, nr));
                    }
                }
                KING_ATK[s] = gm;
            }
        }
    }

    //=============== Public helpers ===================
    BB ray_attacks_from(int sq, Dir dir, BB occ)
    {
        BB attacks = 0;
        const int step = static_cast<int>(dir);

        // Step outward until edge or blocker; include the blocker square
        for (int t = sq + step; t >= 0 && t < 64; t += step)
        {
            // Prevent wrapping across files for E/W/diagonals
            if (!step_respects_edge(t - step, t, dir)) break;

            attacks |= bit(t);
            if (occ & bit(t)) break;
        }
        return attacks;
    }

    BB between_mask(int a, int b)
    {
        // If not aligned on file, rank, or diagonal, return 0
        const int af = file_of(a), ar = rank_of(a);
        const int bf = file_of(b), br = rank_of(b);

        const int dfile = bf - af;
        const int drank = br - ar;

        // Not aligned
        if (!(af == bf || ar == br || std::abs(dfile) == std::abs(drank)))
            return 0;

        // Step of -1, 0, or +1 per axis; translate to square delta
        const int sfile = (dfile > 0) - (dfile < 0);
        const int srank = (drank > 0) - (drank < 0);
        const int step = srank * 8 + sfile;

        // Walk from 'a' toward 'b' (exclusive) collecting squares
        BB mask = 0;
        for (int t = a + step; t != b; t += step)
        {
            // Safety check (should never trip for aligned squares)
            if (t < 0 || t >= 64) return 0;
            mask |= bit(t);
        }
        return mask;
    }

    void init_bitboards()
    {
        static bool initialized = false;
        if (initialized) return;
        initialized = true;

        build_files_ranks();
        build_diagonals();
        build_knight_king();
    }
} // namespace ch
