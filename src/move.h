#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <vector>
#include <iostream>

using namespace std;

enum Square : uint8_t;
class Board;

class Move
{
public:
  Move(Square from, Square to, int8_t captured_piece = 0, int8_t promoted_piece = 0)
      : from(from), to(to), captured_piece(captured_piece), promoted_piece(promoted_piece) {}
  Square get_from() { return from; }
  Square get_to() { return to; }
  int8_t get_captured_piece() { return captured_piece; }
  void set_captured_piece(int8_t new_piece) { captured_piece = new_piece; }
  int8_t get_promoted_piece() { return promoted_piece; }
  void set_promoted_piece(int8_t new_piece) { promoted_piece = new_piece; }
  friend ostream &operator<<(ostream &os, Move move)
  {
    os << (char)('a' + (move.from >> 3)) << (char)('1' + (move.from & 7))
       << (char)('a' + (move.to >> 3)) << (char)('1' + (move.to & 7));
    if (move.promoted_piece)
    {
        os << "pnbrqk"[abs(move.promoted_piece) - 1];
    }
    return os;
  }

private:
  Square from, to;
  int8_t captured_piece;
  int8_t promoted_piece;
};

class MoveGenerator
{
public:
  static vector<Move> generate_moves_pseudo_legal(const Board &board);
  static vector<Move> generate_moves_legal(Board &board);
  static bool detect_check(const Board& board);

private:
  template <int piece>
  static vector<Move> generate_moves_single_piece(const Board &board, Square square);
};

#endif