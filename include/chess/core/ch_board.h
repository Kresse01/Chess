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
 * 
 * This class provides ready-on queries plus low-level mutation helpers that move
 * headers may rely on later (e.g., when you make/unmake). For now, the focus
 * is on queries used by movement mask computation.
 */

#include "ch_types.h"
#include "ch_bitboard.h"
#include <string>

namespace ch
{
    //State purely as bitboards; no make/unmake yet
    class Board
    {
    public:
        Board() { clear(); }

        void clear();

        /** @brief Initialize to the standard chess starting position. */
        void set_startpos();

        /**
         * @brief Parse FEN into bitboard and flags.
         * @return true on success, false on invalid FEN
         */
        bool set_fen(const char* fen);

        // --- FEN exporter to pair with set_fen()
        std::string to_fen() const;

        /** @name Queries
         * @{
         */

         /** @brief Current side to move. */
        Color side_to_move() const { return stm_; }

        /** @brief Bitboard for (color, kind). */
        BB bb(Color c, PieceKind k) const { return bb_[int(c)][int(k)]; }

        /** @brief Occupancy of a color (OR of all piece kinds for that color). */
        BB occ(Color c) const { return occ_[int(c)]; }

        /** @brief Occupancy of all pieces (both colors). */
        BB occ_all() const { return occ_all_; }

        /** @brief En-passant target square index (0..63) or -1 if none. */
        int ep_target() const { return ep_sq_; } // -1 if none

        /** @brief Castling right flags. */
        bool castle_k(Color c) const { return castle_[static_cast<int>(c)][0]; }
        bool castle_q(Color c) const { return castle_[static_cast<int>(c)][1]; }
        /** @} */

        /**@name (Optional) mutation helpers for future make/unmake
        * These are declared for completeness; you may implement when needed.
        * @{
        */
        void set_ep_target(int sq) {ep_sq_ = sq; }
        void set_castle(Color c, bool kside, bool value) { castle_[int(c)][kside?0:1] = value; }
        void set_side_to_move(Color c){ stm_ = c; }
        void set_piece(Color c, PieceKind k, int sq)
        {
         bb_[int(c)][int(k)] |= bit(sq); rebuild_occ();
        }
        void clear_piece(Color c, PieceKind k, int sq)
        {
         bb_[int(c)][int(k)] &= ~bit(sq); rebuild_occ();
        }
        /** @} */

        uint16_t halfmove_clock() const { return halfmove_clock_; }
        void set_halfmove_clock(uint16_t v) { halfmove_clock_ = v; }

        uint32_t fullmove_number() const { return fullmove_number_; }
        void set_fullmove_number(uint32_t v) { fullmove_number_ = v; }

        uint8_t castle_rights_mask() const
        {
         uint8_t m = 0;
         if (castle_[0][0]) m |= 1u; // WK
         if (castle_[0][1]) m |= 1u<<1; // WQ
         if (castle_[1][0]) m |= 1u<<2; // BK
         if (castle_[1][1]) m |= 1u<<3; // BQ
         return m;
        }

        // --- Generic square mutators (color-agnostic)
        void clear_square(int sq); // remove any piece on sq (if any)
       
        // -- Convenience queries
        bool occupied(int sq) const { return (occ_all_ & bit(sq)) != 0; }
        bool occupied_by(int sq, Color c) const { return (occ_[int(c)] & bit(sq)) != 0; }
    
    private:
        BB bb_[2][6]{};         ///< per-(color,kind) bitboards
        BB occ_[2]{};           ///< cached per-color occupancy
        BB occ_all_{};          ///< cached all occupancy
        bool castle_[2][2]{{false,false},{false,false}}; ///< [color][0=K,1=Q]
        int ep_sq_{-1};         ///< en-passant target or -1
        Color stm_{Color::White}; ///< side to move

        uint16_t halfmove_clock_{0}; // for 50-move rule
        uint32_t fullmove_number_{1}; // increments after Black's move

        /** @brief Recompute @ref occ_ and @ref occ_all_ from bb_ arrays. */
        void rebuild_occ();
    };
}