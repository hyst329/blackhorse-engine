#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <string>
#include <stack>
#include <iostream>
#include <random>

#include "move.h"

using namespace std;

const int8_t NONE = 0;

const int8_t PAWN = 1;
const int8_t KNIGHT = 2;
const int8_t BISHOP = 3;
const int8_t ROOK = 4;
const int8_t QUEEN = 5;
const int8_t KING = 6;

const int8_t PIECE_OFFSET = KING;

const int8_t WHITE = 1;
const int8_t BLACK = -1;

const int8_t KINGSIDE = 1;
const int8_t QUEENSIDE = 2;
const int8_t BOTH = 3;

enum Square : uint8_t
{
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    A8,
    B1,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    C1,
    C2,
    C3,
    C4,
    C5,
    C6,
    C7,
    C8,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    E1,
    E2,
    E3,
    E4,
    E5,
    E6,
    E7,
    E8,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    G1,
    G2,
    G3,
    G4,
    G5,
    G6,
    G7,
    G8,
    H1,
    H2,
    H3,
    H4,
    H5,
    H6,
    H7,
    H8,
    INVALID = 255
};

const int PIECES_COUNT = 6;
const int TOTAL_PIECES_COUNT = 2 * PIECES_COUNT + 1;
const int COLORS_COUNT = 2;
const int SQUARES_COUNT = 64;

class Board
{
  public:
    Board();
    Board(string fen);
    int8_t get_piece(Square square) const { return mailbox_board[square]; }
    int8_t get_piecename(Square square) const { return abs(mailbox_board[square]); }
    int8_t get_color(Square square) const
    {
        return (mailbox_board[square] > 0) - (mailbox_board[square] < 0);
    }
    uint64_t &get_bitboard(int8_t piece)
    {
        return piece_bitboards[piece + PIECE_OFFSET];
    }

    const uint64_t &get_bitboard(int8_t piece) const
    {
        return piece_bitboards[piece + PIECE_OFFSET];
    }

    void set_piece(Square square, int piece)
    {
        int8_t old_piece = get_piece(square);
        get_bitboard(old_piece) &= ~(uint64_t(1) << square);
        get_bitboard(NONE) &= ~(uint64_t(1) << square);
        get_bitboard(piece) |= (uint64_t(1) << square);
        // update hash
        hash ^= zobrist_tables[square][old_piece + PIECE_OFFSET];
        hash ^= zobrist_tables[square][piece + PIECE_OFFSET];
        mailbox_board[square] = piece;
    }

    uint64_t get_hash() const { return hash; }

    int8_t get_side_to_move() const { return side_to_move; }

    uint64_t combined_bitboard(int8_t color) const
    {
        uint64_t res = 0;
        for (int piece = PAWN; piece <= KING; piece++)
        {
            res |= get_bitboard(color * piece);
        }
        return res;
    }

    int8_t get_castling(int8_t color) const { return color == WHITE ? white_castling : black_castling; }
    int8_t get_current_castling() const { return get_castling(side_to_move); }
    Square get_en_passant_square() const { return en_passant > 0 ? from_file_rank(en_passant, (side_to_move == WHITE) ? 6 : 3) : INVALID; }
    string get_fen() const;

    void make_move(Move move);
    void switch_sides() { side_to_move = -side_to_move; }
    Move unmake_move();

    static Square from_file_rank(uint8_t file, uint8_t rank)
    {
        return (Square)(file * 8 + rank - 9);
    }

    static mt19937_64& get_rng() { return rng; }

    friend ostream &operator<<(ostream &os, const Board &board);

  private:
    int8_t mailbox_board[SQUARES_COUNT];
    uint64_t piece_bitboards[TOTAL_PIECES_COUNT];
    int8_t white_castling, black_castling;
    int8_t en_passant;
    int8_t side_to_move;
    int move_number, halfmove_counter;
    stack<Move> move_history;
    stack<int> half_moves;
    stack<int8_t> en_passants, castlings;
    static uint64_t zobrist_tables[SQUARES_COUNT][TOTAL_PIECES_COUNT];
    static bool zobrist_initialised;
    static void zobrist_initialise();
    static mt19937_64 rng;
    uint64_t hash;
	stack<uint64_t> hashes;
};

#endif