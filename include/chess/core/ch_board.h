#pragma once
/**
 * @file ch_board.h
 * @brief Board state stored as bitboards and flags; no move enumeration.
 * 
 * The board tracks:
 *   - Per-(Color, PieceKind) bitboards
 *   - Cached occupancies (per color and all)
 *   - Side to move
 *   - Castling rights (per side, K/Q)
 *   - En-passant target square (index or -1)
 *   - Halfmove clock + fullmove number (for FEN / 50-move rule)
 * 
 * This class provides:
 *   - Queries used by attack generation / legality
 *   - Low-level mutation helpers used by make/unmake and test setups
 */

#include <cstdint>
#include <string>

#include "chess/core/ch_types.h"
#include "chess/core/ch_bitboard.h"

namespace ch
{
    class Board
    {
    public:
        Board() { clear(); }

        // -------- setup --------
        void clear();

        /** @brief Initialize to the standard chess starting position. */
        void set_startpos();

        /**
         * @brief Parse FEN into bitboard and flags.
         * @return true on success, false on invalid FEN
         */
        bool set_fen(const char* fen);

        /** @brief Export current board state to a FEN string. */
        std::string to_fen() const;

        // -------- queries --------

         /** @brief Current side to move. */
        [[nodiscard]] Color side_to_move() const noexcept { return stm_; }

        /** @brief Bitboard for (color, kind). */
        [[nodiscard]] BB bb(Color c, PieceKind k) const noexcept
        {
            return bb_[static_cast<int>(c)][static_cast<int>(k)];
        }

        /** @brief Occupancy of a color (OR of all piece kinds for that color). */
        [[nodiscard]] BB occ(Color c) const noexcept { return occ_[static_cast<int>(c)]; }

        /** @brief Occupancy of all pieces. */
        [[nodiscard]] BB occ_all() const noexcept { return occ_all_; }

        /** @brief En-passant target square index (0..63) or -1 if none. */
        [[nodiscard]] int ep_target() const noexcept { return ep_sq_; }

        /** @brief Castling right flags.
         * Internal layout: castle_[color][0=K-side, 1=Q-side]
        */
        [[nodiscard]] bool castle_k(Color c) const noexcept { return castle_[static_cast<int>(c)][0]; }
        [[nodiscard]] bool castle_q(Color c) const noexcept { return castle_[static_cast<int>(c)][1]; }

        [[nodiscard]] std::uint16_t halfmove_clock() const noexcept { return halfmove_clock_; }
        [[nodiscard]] std::uint32_t fullmove_number() const noexcept { return fullmove_number_; }

        /**
         * @brief Packed castling rights in the usual 4-bit format:
         * bit0=WK, bit1=WQ, bit2=BK, bit3=BQ
         */
        [[nodiscard]] std::uint8_t castle_rights_mask() const noexcept
        {
            std::uint8_t m = 0;
            if (castle_[0][0]) m |= 1u << 0; // WK
            if (castle_[0][1]) m |= 1u << 1; // WQ
            if (castle_[1][0]) m |= 1u << 2; // BK
            if (castle_[1][1]) m |= 1u << 3; // BQ
            return m;
        }

        // -------- low-level mutation helpers --------
        // These are intentionally simple and are used by:
        //  - FEN setup
        //  - make/unmake
        //  - tests
        //
        // Note: set_piece/clear_piece rebuild cached occupancies immediately.

        void set_ep_target(int sq) noexcept { ep_sq_ = sq; }

        void set_castle(Color c, bool kside, bool value) noexcept
        {
            castle_[static_cast<int>(c)][kside ? 0 : 1] = value;
        }

        void set_side_to_move(Color c) noexcept { stm_ = c; }

        void set_halfmove_clock(std::uint16_t v) noexcept { halfmove_clock_ = v; }
        void set_fullmove_number(std::uint32_t v) noexcept { fullmove_number_ = v; }

        void set_piece(Color c, PieceKind k, int sq)
        {
            bb_[static_cast<int>(c)][static_cast<int>(k)] |= bit(sq);
            rebuild_occ();
        }

        void clear_piece(Color c, PieceKind k, int sq)
        {
            bb_[static_cast<int>(c)][static_cast<int>(k)] &= ~bit(sq);
            rebuild_occ();
        }


        /** @brief Remove any piece on sq (if any). */
        void clear_square(int sq);
       
        // -- Convenience queries (used by GUI / movegen sometimes)
        [[nodiscard]] bool occupied(int sq) const noexcept { return (occ_all_ & bit(sq)) != 0; }
        [[nodiscard]] bool occupied_by(int sq, Color c) const noexcept { return (occ_[static_cast<int>(c)] & bit(sq)) != 0; }
    
    private:
        BB bb_[2][6]{};         ///< per-(color,kind) bitboards; kind indices 0..5
        BB occ_[2]{};           ///< cached per-color occupancy
        BB occ_all_{};          ///< cached all occupancy

        bool castle_[2][2]{{false,false},{false,false}}; ///< [color][0=K,1=Q]
        int ep_sq_{-1};         ///< en-passant target, or -1
        Color stm_{Color::White}; ///< side to move

        std::uint16_t halfmove_clock_{0}; ///< for 50-move rule / FEN
        std::uint32_t fullmove_number_{1}; ///< increments after Black's move

        /** @brief Recompute @ref occ_ and @ref occ_all_ from bb_ arrays. */
        void rebuild_occ();
    };
} // namespace ch