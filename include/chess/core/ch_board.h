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
    
    private:
        BB bb_[2][6]{};         ///< per-(color,kind) bitboards
        BB occ_[2]{};           ///< cached per-color occupancy
        BB occ_all_{};          ///< cached all occupancy
        bool castle_[2][2]{{false,false},{false,false}}; ///< [color][0=K,1=Q]
        int ep_sq_{-1};         ///< en-passant target or -1
        Color stm_{Color::White}; ///< side to move

        /** @brief Recompute @ref occ_ and @ref occ_all_ from bb_ arrays. */
        void rebuild_occ();
    };
}