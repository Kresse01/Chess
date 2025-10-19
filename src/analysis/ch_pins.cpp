#include "chess/analysis/ch_pins.h"
#include <cstdlib>

namespace ch
{
    static inline int file_of(int s){return s & 7; }
    static inline int rank_of(int s){return s >> 3; }

    static inline bool slider_matches_dir(PieceKind k, int dfile, int drank)
    {
        //Orthogonal if exactly one delta axis is 0; diagonal if |df|==|dr|
        bool ortho = (dfile == 0) ^ (drank == 0);
        bool diag = (dfile != 0) && (std::abs(dfile) == std::abs(drank));
        if (k == PieceKind::Rook) return ortho;
        if (k == PieceKind::Bishop) return diag;
        if (k == PieceKind::Queen) return ortho | diag;
        return false;
    }

    Pins compute_pins(const Board&b, Color side)
    {
        Pins out;
        BB kbb = b.bb(side, PieceKind::King);
        if(!kbb) return out; // degenerate
        int ks = lsb(kbb);
        
        const Color us = side;
        const Color them = opposite(side);
        BB occ = b.occ_all();

        // Explore 8 rays from the king. If first blocker is ours and second blocker
        // (along same ray) is enemy rook/bishop/queen matching the ray, the first blocker is pinned.
        constexpr Dir DIRS[8] = {N, S, E, W, NE, NW, SE, SW};

        for (Dir d : DIRS)
        {
            // Walk out: record first friendly, then see if the next blocker is a matching enemy slider.
            int step = static_cast<int>(d);
            int prev = ks;
            int firstFriend = -1;
            BB segment = bit(ks); // start closed segment at king

            for (int s = ks + step; s >= 0 && s < 64; s += step)
            {
                // edge wrap prevention (same as in ray_attacks_from)
                int pf = file_of(prev), nf = file_of(s);
                if (d == E || d == W || d == NE || d == NW || d == SE || d == SW)
                {
                    if(std::abs(nf-pf) != 1) break;
                }
                segment |= bit(s);

                BB sq = bit(s);
                if (!(occ & sq)) { prev = s; continue; } // empty, keep going

                // Blocker encountered
                if(b.occ(us) & sq)
                {
                    // first friendly along ray: only candidate can be pinned
                    if (firstFriend == -1)
                    {
                        firstFriend = s;
                        prev = s;
                        continue;
                    } else {
                        // two friendless before any enemy slider => nobody pinned
                    }
                } else {
                    // enemy piece - if we have a friendly before and enemy is a matching slider -> pin
                    if (firstFriend != -1)
                    {
                        // classify direction kind of (df, dr)
                        int df = file_of(s) - file_of(ks);
                        int dr = rank_of(s) - rank_of(ks);

                        PieceKind enemyK = PieceKind::Pawn; // init non-matching
                        if (b.bb(them, PieceKind::Rook) & sq) enemyK = PieceKind::Rook;
                        else if (b.bb(them, PieceKind::Bishop) & sq) enemyK = PieceKind::Bishop;
                        else if (b.bb(them, PieceKind::Queen) & sq) enemyK = PieceKind::Queen;

                        if (slider_matches_dir(enemyK, df, dr))
                        {
                            out.pinned |= bit(firstFriend);
                            out.ray_to_enemy[firstFriend] = segment; // closed king..enemy
                        }
                    }
                    break;
                }
            }
        }
        return out;
    }
}