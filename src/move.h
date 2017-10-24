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
  Move(Square from, Square to, int8_t captured_piece = 0, int8_t promoted_piece = 0, bool en_passant = false)
      : from(from), to(to), captured_piece(captured_piece), promoted_piece(promoted_piece), en_passant(en_passant) {}
  Move() : Move((Square)255, (Square)255) {}
  Square get_from() const { return from; }
  Square get_to() const { return to; }
  int8_t get_captured_piece() { return captured_piece; }
  void set_captured_piece(int8_t new_piece) { captured_piece = new_piece; }
  int8_t get_promoted_piece() { return promoted_piece; }
  void set_promoted_piece(int8_t new_piece) { promoted_piece = new_piece; }
  bool get_en_passant() { return en_passant; }
  void set_en_passant(bool new_en_passant) { en_passant = new_en_passant; }
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
  bool operator==(Move &other)
  {
    return from == other.from && to == other.to;
  }
  Move(string move_str)
  {
    if (move_str.size() == 4)
    {
      move_str += ' ';
    }
    from = (Square)((move_str[0] - 'a') << 3 | (move_str[1] - '1')); 
    to = (Square)((move_str[2] - 'a') << 3 | (move_str[3] - '1'));
    promoted_piece = string(" pnbrqk").find(move_str[4]);
    if (move_str[3] == '1') promoted_piece = -promoted_piece;
  }

private:
  Square from, to;
  int8_t captured_piece;
  int8_t promoted_piece;
  bool en_passant;
};

class MoveGenerator
{
public:
  static vector<Move> generate_moves_pseudo_legal(const Board &board, bool captures_only = false);
  static vector<Move> generate_moves_legal(Board &board, bool captures_only = false);
  static bool detect_check(const Board &board);

private:
  template <int piece>
  static void generate_moves_single_piece(const Board &board, Square square, vector<Move> &target, bool captures_only = false);
};

#endif