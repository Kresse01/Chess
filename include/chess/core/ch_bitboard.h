#pragma once
/**
 * @file ch_Bitboard.h
 * @brief Bitboard constants, masks, precomputed tables, and ray helpers.
 * 
 * This header declears:
 *  - Per-file and per-rank masks (FILE_MASK, RANK_MASK)
 *  - Precomputed leaper attack tables (KNIGHT_ATK, KING_ATK)
 *  - Diagonal masks (optional helpers for debugging/analysis)
 *  - Ray stepping helpers for sliding pieces
 *  - A one-time initializer (init_bitboards)
 * 
 * Implementations are provided in ch_bitboard.cpp
 */

#include <cstdint>
#include "ch_types.h"

namespace ch
{
    /**
     * @brief Single-bit mask for a given square;
     */
    constexpr BB ONE = 1ull;

    /**
     * @brief Single-bit mask for a given square.
     * @param sq square index 0..63
     * @return bitboard with only @p sq set
     */
    inline BB bit(int sq) { return ONE << sq; }

    /** @brief Population count (number of set bits). */
    inline int popcount(BB b) { return __builtin_popcountll(b); }

    /** @brief Index of least-significant 1 bit (undefined if b==0). */
    inline int lsb(BB b) { return __builtin_ctzll(b); }

    /**
     * @brief Pop the least-significant 1 bit and return it as a one-hot bitboard.
     * @param b in/out: original bitboard; will have the LSB cleared
     * @return a one-shot bitboard corresponding to the popped bit
     */
    inline BB poplsb(BB& b) { BB x=b&-b; b^=x; return x; }

    /**
     * @name Precomputed board-wide masks
     * @{
     */

    /**
     * @brief Mask for each file (column).
     * FILE_MASK[0] = file A, ..., FILE_MASK[7] = file H.
     */
    extern BB FILE_MASK[8];

    /**
     * @brief Mask for each rank (row).
     * RANK_MASK[0] = rank 1, ..., RANK_MASK[7] = rank 8.
     */
    extern BB RANK_MASK[8];

    /**
     * @brief (Optional) Diagonal masks for A1-H8 and A8-H1 diagonals
     * Indexed 0..14; useful for analysis and "between" computations.
     */
    extern BB DIAG_A1H8[15], DIAG_A8H1[15];
    /**@} */

    /**
     * @name Leaper precomputed attack tables
     * @{
     */

    /**
     * @brief Knight attack masks for each square, independent of occupancy.
     * KNIGHT_ATK[s] gives all squares a knight on s could attack (capture or quiet),
     * before intersecting with empty/enemy/own occupancy.
     */
    extern BB KNIGHT_ATK[64];

    /**
     * @brief King attack masks for each square, independent of occupancy.
     */
    extern BB KING_ATK[64];
    /**@} */

    /**
     * @brief Direction codes for ray stepping
     * 
     * Values correspond to square index deltas. For example, N=+8 moves one rank up,
     * E=+1 moves one file right, NE=+9 moves one file right and one rank up, etc.
     */
    enum Dir { N=8, S=-8, E=1, W=-1, NE=9, NW=7, SE=-7, SW=-9};

    /**
     * @brief Compute sliding attacks from a starting square in one direction.
     * 
     * Walks from @p sq in direction @p dir until the board edge or the first blocker
     * (a set bit in @p occ). The returned mask includes the blocker square (so that
     * captures are represented) and all empty squares up to it.
     * 
     * @param sq start square 0..63
     * @param dir direction (one of N,S,E,W,NE,NW,SE,SW)
     * @param occ occupancy bitboard of all pieces (both colors)
     * @return bitboard of attacked squares in that direction
     */
    BB ray_attacks_from(int sq, Dir dir, BB occ);

    /**
     * @brief Mask of squares strictly between @p a and @p b if aligned (same file, rank,
     * or diagonal). Return 0 if not aligned.
     */
    BB between_mask(int a, int b);

    /**
     * @brief Initialize all precomputed tables and masks in this header.
     * Call once at program startup before using any tables.
     */
    void init_bitboards();
}