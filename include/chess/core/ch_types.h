#pragma once
/**
 * @file ch_types.h
 * @brief Fundamental engine types: bitboard alias, colors, piece kinds, and helpers.
 * 
 * These are minimal POD types and used throughout the engine. Keep them lightweight
 * and header-only to enable inlining across the codebase.
 */
#include <cstdint>

namespace ch
{
    /**
     * @brief 64-bit bitboard type
     * 
     * A bitboard uses 64 bits to represent the 64 squares of a chessboard.
     * By convention here: a1 = bit 0, h1 = bit 7, a8 = bit 56, h8 = bit 63.
     */
    using BB = uint64_t;

    /**
     * @brief Side to move / piece color.
     */
    enum class Color : uint8_t {White, Black};

    /**
     * @brief Logical kind of a chess piece.
     * 
     * We seperate PieceKind from Color so that (Kind, Color) forms the concrete type.
     * Use PieceKind to select movement rules; combine with Color to query occupancy.
     */
    enum class PieceKind : uint8_t{Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5};

    /**
     * @brief Utility: return the opposite color.
     */
    inline constexpr Color opposite(Color c)
    {
        return c == Color::White ? Color::Black : Color::White;
    }

    /**
     * @brief Convert file+rank to a 0..63 square index.
     * 
     * @param file 0..7 (0=a, 7=h)
     * @param rank 0..7 (0=rank1, 7=rank8)
     * @return square index 0..63 (a1=0, h8=63)
     */
    inline constexpr int idx(int file, int rank)
    {
        return (rank << 3) | file;
    }
}