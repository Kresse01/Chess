#pragma once
/**
 * @file ch_types.h
 * @brief Fundamental chess types used across the rules implementation.
 *
 * Keep this header lightweight: POD types + tiny constexpr helpers only.
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
    using BB = std::uint64_t;

    /**
     * @brief Side to move / piece color.
     */
    enum class Color : std::uint8_t {White = 0, Black = 1};

    /**
     * @brief Logical kind of a chess piece.
     * 
     * We seperate PieceKind from Color so that (Kind, Color) forms the concrete type.
     * Use PieceKind to select movement rules; combine with Color to query occupancy.
     */
    enum class PieceKind : std::uint8_t{Pawn = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4, King = 5, None = 6};

    // Board constants (kept here to avoid magic numbers elsewhere)
    inline constexpr int BoardFiles = 8;
    inline constexpr int BoardRanks = 8;
    inline constexpr int BoardSquars = 64;

    template <typename E>
    [[nodiscard]] inline constexpr int to_int(E e) noexcept
    {
        return static_cast<int>(e);
    }



    /**
     * @brief Utility: return the opposite color.
     */
    [[nodiscard]] inline constexpr Color opposite(Color c) noexcept
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
    [[nodiscard]] inline constexpr int idx(int file, int rank) noexcept
    {
        return (rank << 3) | file;
    }

    /// Extract file from square index [0..63]
    [[nodiscard]] inline constexpr int file_of(int sq) noexcept
    {
        return sq & 7;
    }

    /// Extract file from square index [0..63]
    [[nodiscard]] inline constexpr int rank_of(int sq) noexcept
    {
        return sq >> 3;
    }
}