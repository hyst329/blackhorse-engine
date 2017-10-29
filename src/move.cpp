#include "move.h"
#include "board.h"

#include <iomanip>
#include <omp.h>

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
#endif // _MSC_VER

#include "magic.h"

//#pragma omp declare reduction(merge : std::vector<Move> :
//omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))

const uint64_t KING_PATTERNS_TABLE[64] = {
    0x0000000000000302, 0x0000000000000705, 0x0000000000000e0a,
    0x0000000000001c14, 0x0000000000003828, 0x0000000000007050,
    0x000000000000e0a0, 0x000000000000c040, 0x0000000000030203,
    0x0000000000070507, 0x00000000000e0a0e, 0x00000000001c141c,
    0x0000000000382838, 0x0000000000705070, 0x0000000000e0a0e0,
    0x0000000000c040c0, 0x0000000003020300, 0x0000000007050700,
    0x000000000e0a0e00, 0x000000001c141c00, 0x0000000038283800,
    0x0000000070507000, 0x00000000e0a0e000, 0x00000000c040c000,
    0x0000000302030000, 0x0000000705070000, 0x0000000e0a0e0000,
    0x0000001c141c0000, 0x0000003828380000, 0x0000007050700000,
    0x000000e0a0e00000, 0x000000c040c00000, 0x0000030203000000,
    0x0000070507000000, 0x00000e0a0e000000, 0x00001c141c000000,
    0x0000382838000000, 0x0000705070000000, 0x0000e0a0e0000000,
    0x0000c040c0000000, 0x0003020300000000, 0x0007050700000000,
    0x000e0a0e00000000, 0x001c141c00000000, 0x0038283800000000,
    0x0070507000000000, 0x00e0a0e000000000, 0x00c040c000000000,
    0x0302030000000000, 0x0705070000000000, 0x0e0a0e0000000000,
    0x1c141c0000000000, 0x3828380000000000, 0x7050700000000000,
    0xe0a0e00000000000, 0xc040c00000000000, 0x0203000000000000,
    0x0507000000000000, 0x0a0e000000000000, 0x141c000000000000,
    0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000,
    0x40c0000000000000,
};
const uint64_t KNIGHT_PATTERNS_TABLE[64] = {
    0x0000000000020400, 0x0000000000050800, 0x00000000000a1100,
    0x0000000000142200, 0x0000000000284400, 0x0000000000508800,
    0x0000000000a01000, 0x0000000000402000, 0x0000000002040004,
    0x0000000005080008, 0x000000000a110011, 0x0000000014220022,
    0x0000000028440044, 0x0000000050880088, 0x00000000a0100010,
    0x0000000040200020, 0x0000000204000402, 0x0000000508000805,
    0x0000000a1100110a, 0x0000001422002214, 0x0000002844004428,
    0x0000005088008850, 0x000000a0100010a0, 0x0000004020002040,
    0x0000020400040200, 0x0000050800080500, 0x00000a1100110a00,
    0x0000142200221400, 0x0000284400442800, 0x0000508800885000,
    0x0000a0100010a000, 0x0000402000204000, 0x0002040004020000,
    0x0005080008050000, 0x000a1100110a0000, 0x0014220022140000,
    0x0028440044280000, 0x0050880088500000, 0x00a0100010a00000,
    0x0040200020400000, 0x0204000402000000, 0x0508000805000000,
    0x0a1100110a000000, 0x1422002214000000, 0x2844004428000000,
    0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
    0x0400040200000000, 0x0800080500000000, 0x1100110a00000000,
    0x2200221400000000, 0x4400442800000000, 0x8800885000000000,
    0x100010a000000000, 0x2000204000000000, 0x0004020000000000,
    0x0008050000000000, 0x00110a0000000000, 0x0022140000000000,
    0x0044280000000000, 0x0088500000000000, 0x0010a00000000000,
    0x0020400000000000,
};

const int ROOK_BITS[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12,
};

const int BISHOP_BITS[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7,
    5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7,
    7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6,
};

const Square UP[64] = {
    A2, A3, A4, A5, A6, A7, A8, INVALID, B2, B3, B4, B5, B6, B7, B8, INVALID,
    C2, C3, C4, C5, C6, C7, C8, INVALID, D2, D3, D4, D5, D6, D7, D8, INVALID,
    E2, E3, E4, E5, E6, E7, E8, INVALID, F2, F3, F4, F5, F6, F7, F8, INVALID,
    G2, G3, G4, G5, G6, G7, G8, INVALID, H2, H3, H4, H5, H6, H7, H8, INVALID,
};

const Square DOWN[64] = {
    INVALID, A1, A2, A3, A4, A5, A6, A7, INVALID, B1, B2, B3, B4, B5, B6, B7,
    INVALID, C1, C2, C3, C4, C5, C6, C7, INVALID, D1, D2, D3, D4, D5, D6, D7,
    INVALID, E1, E2, E3, E4, E5, E6, E7, INVALID, F1, F2, F3, F4, F5, F6, F7,
    INVALID, G1, G2, G3, G4, G5, G6, G7, INVALID, H1, H2, H3, H4, H5, H6, H7,
};

const Square RIGHT[64] = {
    B1,      B2,      B3,      B4,      B5,      B6,      B7,      B8,
    C1,      C2,      C3,      C4,      C5,      C6,      C7,      C8,
    D1,      D2,      D3,      D4,      D5,      D6,      D7,      D8,
    E1,      E2,      E3,      E4,      E5,      E6,      E7,      E8,
    F1,      F2,      F3,      F4,      F5,      F6,      F7,      F8,
    G1,      G2,      G3,      G4,      G5,      G6,      G7,      G8,
    H1,      H2,      H3,      H4,      H5,      H6,      H7,      H8,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
};

const Square LEFT[64] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    A1,      A2,      A3,      A4,      A5,      A6,      A7,      A8,
    B1,      B2,      B3,      B4,      B5,      B6,      B7,      B8,
    C1,      C2,      C3,      C4,      C5,      C6,      C7,      C8,
    D1,      D2,      D3,      D4,      D5,      D6,      D7,      D8,
    E1,      E2,      E3,      E4,      E5,      E6,      E7,      E8,
    F1,      F2,      F3,      F4,      F5,      F6,      F7,      F8,
    G1,      G2,      G3,      G4,      G5,      G6,      G7,      G8,
};

const Square UP_RIGHT[64] = {
    B2,      B3,      B4,      B5,      B6,      B7,      B8,      INVALID,
    C2,      C3,      C4,      C5,      C6,      C7,      C8,      INVALID,
    D2,      D3,      D4,      D5,      D6,      D7,      D8,      INVALID,
    E2,      E3,      E4,      E5,      E6,      E7,      E8,      INVALID,
    F2,      F3,      F4,      F5,      F6,      F7,      F8,      INVALID,
    G2,      G3,      G4,      G5,      G6,      G7,      G8,      INVALID,
    H2,      H3,      H4,      H5,      H6,      H7,      H8,      INVALID,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
};

const Square UP_LEFT[64] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    A2,      A3,      A4,      A5,      A6,      A7,      A8,      INVALID,
    B2,      B3,      B4,      B5,      B6,      B7,      B8,      INVALID,
    C2,      C3,      C4,      C5,      C6,      C7,      C8,      INVALID,
    D2,      D3,      D4,      D5,      D6,      D7,      D8,      INVALID,
    E2,      E3,      E4,      E5,      E6,      E7,      E8,      INVALID,
    F2,      F3,      F4,      F5,      F6,      F7,      F8,      INVALID,
    G2,      G3,      G4,      G5,      G6,      G7,      G8,      INVALID,
};

const Square DOWN_RIGHT[64] = {
    INVALID, B1,      B2,      B3,      B4,      B5,      B6,      B7,
    INVALID, C1,      C2,      C3,      C4,      C5,      C6,      C7,
    INVALID, D1,      D2,      D3,      D4,      D5,      D6,      D7,
    INVALID, E1,      E2,      E3,      E4,      E5,      E6,      E7,
    INVALID, F1,      F2,      F3,      F4,      F5,      F6,      F7,
    INVALID, G1,      G2,      G3,      G4,      G5,      G6,      G7,
    INVALID, H1,      H2,      H3,      H4,      H5,      H6,      H7,
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
};

const Square DOWN_LEFT[64] = {
    INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
    INVALID, A1,      A2,      A3,      A4,      A5,      A6,      A7,
    INVALID, B1,      B2,      B3,      B4,      B5,      B6,      B7,
    INVALID, C1,      C2,      C3,      C4,      C5,      C6,      C7,
    INVALID, D1,      D2,      D3,      D4,      D5,      D6,      D7,
    INVALID, E1,      E2,      E3,      E4,      E5,      E6,      E7,
    INVALID, F1,      F2,      F3,      F4,      F5,      F6,      F7,
    INVALID, G1,      G2,      G3,      G4,      G5,      G6,      G7,
};

extern uint64_t ROOK_ATTACKS_TABLE[64][4096];
extern uint64_t BISHOP_ATTACKS_TABLE[64][512];

template <>
void MoveGenerator::generate_moves_single_piece<PAWN>(const Board &board,
                                                      Square square,
                                                      vector<Move> &target,
                                                      bool captures_only) {
  int8_t color = board.get_side_to_move();
  Square forward = (Square)(square + color);
  Square en_passant = board.get_en_passant_square();
  if (!captures_only && board.get_piece(forward) == NONE) {
    if ((forward & 7) == (color == WHITE ? 7 : 0)) {
      target.emplace_back(Move(square, forward, NONE, color * QUEEN));
      target.emplace_back(Move(square, forward, NONE, color * ROOK));
      target.emplace_back(Move(square, forward, NONE, color * BISHOP));
      target.emplace_back(Move(square, forward, NONE, color * KNIGHT));
    } else {
      target.emplace_back(Move(square, forward));
    }
  }
  Square left_forward = LEFT[forward], right_forward = RIGHT[forward];
  if (left_forward != INVALID) {
    if (board.get_color(left_forward) == -color) {
      int8_t piece = board.get_piece(left_forward);
      if ((left_forward & 7) == (color == WHITE ? 7 : 0)) {
        target.emplace_back(Move(square, left_forward, piece, color * QUEEN));
        target.emplace_back(Move(square, left_forward, piece, color * ROOK));
        target.emplace_back(Move(square, left_forward, piece, color * BISHOP));
        target.emplace_back(Move(square, left_forward, piece, color * KNIGHT));
      } else {
        target.emplace_back(Move(square, left_forward, piece));
      }
    }
  }
  if (right_forward != INVALID) {
    if (board.get_color(right_forward) == -color) {
      int8_t piece = board.get_piece(right_forward);
      if ((right_forward & 7) == (color == WHITE ? 7 : 0)) {
        target.emplace_back(Move(square, right_forward, piece, color * QUEEN));
        target.emplace_back(Move(square, right_forward, piece, color * ROOK));
        target.emplace_back(Move(square, right_forward, piece, color * BISHOP));
        target.emplace_back(Move(square, right_forward, piece, color * KNIGHT));
      } else {
        target.emplace_back(Move(square, right_forward, piece));
      }
    }
  }
  // En passant captures NEED WORK!!!
  if (en_passant != INVALID) {
    if (left_forward == en_passant) {
      target.emplace_back(
          Move(square, left_forward, -color * PAWN, NONE, true));
    }
    if (right_forward == en_passant) {
      target.emplace_back(
          Move(square, right_forward, -color * PAWN, NONE, true));
    }
  }
  if (!captures_only && (square & 7) == (color == WHITE ? 1 : 6)) {
    // pawn is on her starting position
    Square forward_two = (Square)(square + 2 * color);
    if (board.get_piece(forward) == NONE &&
        board.get_piece(forward_two) == NONE) {
      target.emplace_back(Move(square, forward_two));
    }
  }
}
template <>
void MoveGenerator::generate_moves_single_piece<KNIGHT>(const Board &board,
                                                        Square square,
                                                        vector<Move> &target,
                                                        bool captures_only) {
  uint64_t needed_squares = board.combined_bitboard(-board.get_side_to_move());
  if (!captures_only) {
    needed_squares |= board.get_bitboard(NONE);
  }
  uint64_t pattern = KNIGHT_PATTERNS_TABLE[square] & needed_squares;
  for (int i = 0; i < SQUARES_COUNT; i++) {
    if (pattern & (1ULL << i)) {
      target.emplace_back(Move(square, (Square)i, board.get_piece((Square)i)));
    }
  }
}
template <>
void MoveGenerator::generate_moves_single_piece<BISHOP>(const Board &board,
                                                        Square square,
                                                        vector<Move> &target,
                                                        bool captures_only) {
  // TODO (untested): Generate moves for bishop
  // int8_t color = board.get_side_to_move();
  // vector<Move> res;
  // const Square *directions[4] = {UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT};
  // for (const Square *direction : directions)
  // {
  //     Square current = direction[square];
  //     while (current != INVALID)
  //     {
  //         int8_t piece = board.get_piece(current);
  //         if (piece != NONE)
  //         {
  //             if (piece * color < 0) // opposite colors
  //             {
  //                 res.emplace_back(Move(square, current, piece));
  //             }
  //             break;
  //         }
  //         else
  //         {
  //             res.emplace_back(Move(square, current));
  //         }
  //         current = direction[current];
  //     }
  // }
  uint64_t occupied = ~board.get_bitboard(NONE);
  occupied &= BISHOP_MAGIC[square].mask;
  occupied *= BISHOP_MAGIC[square].magic;
  occupied >>= (64 - BISHOP_BITS[square]);
  uint64_t possible_bitboard = BISHOP_ATTACKS_TABLE[square][occupied];
  possible_bitboard &= ~board.combined_bitboard(
      board.get_side_to_move()); // masking out own pieces
  if (captures_only) {
    possible_bitboard &= ~board.get_bitboard(NONE); // masking out free squares
  }
  for (int dest = 0; dest < 64; dest++) {
    if (possible_bitboard & (1ULL << dest)) {
      target.emplace_back(
          Move(square, (Square)dest, board.get_piece((Square)dest)));
    }
  }
}
template <>
void MoveGenerator::generate_moves_single_piece<ROOK>(const Board &board,
                                                      Square square,
                                                      vector<Move> &target,
                                                      bool captures_only) {
  // TODO (untested): Generate moves for rook
  // int8_t color = board.get_side_to_move();
  // vector<Move> res;
  // const Square *directions[4] = {UP, DOWN, RIGHT, LEFT};
  // for (const Square *direction : directions)
  // {
  //     Square current = direction[square];
  //     while (current != INVALID)
  //     {
  //         int8_t piece = board.get_piece(current);
  //         if (piece != NONE)
  //         {
  //             if (piece * color < 0) // opposite colors
  //             {
  //                 res.emplace_back(Move(square, current, piece));
  //             }
  //             break;
  //         }
  //         else
  //         {
  //             res.emplace_back(Move(square, current));
  //         }
  //         current = direction[current];
  //     }
  // }
  uint64_t occupied = ~board.get_bitboard(NONE);
  occupied &= ROOK_MAGIC[square].mask;
  occupied *= ROOK_MAGIC[square].magic;
  occupied >>= (64 - ROOK_BITS[square]);
  uint64_t possible_bitboard = ROOK_ATTACKS_TABLE[square][occupied];
  possible_bitboard &= ~board.combined_bitboard(
      board.get_side_to_move()); // masking out own pieces
  if (captures_only) {
    possible_bitboard &= ~board.get_bitboard(NONE); // masking out free squares
  }
  for (int dest = 0; dest < 64; dest++) {
    if (possible_bitboard & (1ULL << dest)) {
      target.emplace_back(
          Move(square, (Square)dest, board.get_piece((Square)dest)));
    }
  }
}
template <>
void MoveGenerator::generate_moves_single_piece<QUEEN>(const Board &board,
                                                       Square square,
                                                       vector<Move> &target,
                                                       bool captures_only) {
  // TODO (untested): Generate moves for queen
  // int8_t color = board.get_side_to_move();
  // vector<Move> res;
  // const Square *directions[8] = {UP, DOWN, RIGHT, LEFT, UP_RIGHT, UP_LEFT,
  // DOWN_RIGHT, DOWN_LEFT}; for (const Square *direction : directions)
  // {
  //     Square current = direction[square];
  //     while (current != INVALID)
  //     {
  //         int8_t piece = board.get_piece(current);
  //         if (piece != NONE)
  //         {
  //             if (piece * color < 0) // opposite colors
  //             {
  //                 res.emplace_back(Move(square, current, piece));
  //             }
  //             break;
  //         }
  //         else
  //         {
  //             res.emplace_back(Move(square, current));
  //         }
  //         current = direction[current];
  //     }
  // }
  uint64_t occupied_rook = ~board.get_bitboard(NONE);
  occupied_rook &= ROOK_MAGIC[square].mask;
  occupied_rook *= ROOK_MAGIC[square].magic;
  occupied_rook >>= (64 - ROOK_BITS[square]);
  uint64_t possible_bitboard_rook = ROOK_ATTACKS_TABLE[square][occupied_rook];
  uint64_t occupied_bishop = ~board.get_bitboard(NONE);
  occupied_bishop &= BISHOP_MAGIC[square].mask;
  occupied_bishop *= BISHOP_MAGIC[square].magic;
  occupied_bishop >>= (64 - BISHOP_BITS[square]);
  uint64_t possible_bitboard_bishop =
      BISHOP_ATTACKS_TABLE[square][occupied_bishop];
  uint64_t possible_bitboard =
      possible_bitboard_rook | possible_bitboard_bishop;
  possible_bitboard &= ~board.combined_bitboard(
      board.get_side_to_move()); // masking out own pieces
  if (captures_only) {
    possible_bitboard &= ~board.get_bitboard(NONE); // masking out free squares
  }
  for (int dest = 0; dest < 64; dest++) {
    if (possible_bitboard & (1ULL << dest)) {
      target.emplace_back(
          Move(square, (Square)dest, board.get_piece((Square)dest)));
    }
  }
}
template <>
void MoveGenerator::generate_moves_single_piece<KING>(const Board &board,
                                                      Square square,
                                                      vector<Move> &target,
                                                      bool captures_only) {
  uint64_t needed_squares = board.combined_bitboard(-board.get_side_to_move());
  if (!captures_only) {
    needed_squares |= board.get_bitboard(NONE);
  }
  uint64_t pattern = KING_PATTERNS_TABLE[square] & needed_squares;
  for (int i = 0; i < SQUARES_COUNT; i++) {
    if (pattern & (1ULL << i)) {
      target.emplace_back(Move(square, (Square)i, board.get_piece((Square)i)));
    }
  }
}

vector<Move> MoveGenerator::generate_moves_pseudo_legal(const Board &board,
                                                        bool captures_only) {
  uint64_t pieces = board.combined_bitboard(board.get_side_to_move());
  uint64_t free_squares = board.get_bitboard(NONE);
  vector<Move> res;
  res.reserve(256);
  //#pragma omp parallel num_threads(8)
  //#pragma omp for nowait reduction(merge : res)
  for (int i = 0; i < SQUARES_COUNT; i++) {
    if (pieces & (1ULL << i)) {
      int8_t piece = board.get_piecename((Square)i);
      switch (piece) {
      case PAWN:
        generate_moves_single_piece<PAWN>(board, (Square)i, res, captures_only);
        break;
      case KNIGHT:
        generate_moves_single_piece<KNIGHT>(board, (Square)i, res,
                                            captures_only);
        break;
      case BISHOP:
        generate_moves_single_piece<BISHOP>(board, (Square)i, res,
                                            captures_only);
        break;
      case ROOK:
        generate_moves_single_piece<ROOK>(board, (Square)i, res, captures_only);
        break;
      case QUEEN:
        generate_moves_single_piece<QUEEN>(board, (Square)i, res,
                                           captures_only);
        break;
      case KING:
        generate_moves_single_piece<KING>(board, (Square)i, res, captures_only);
        break;
      }
    }
  }
  if (!captures_only) {
    int8_t castling = board.get_current_castling();
    if (castling & QUEENSIDE) {
      bool white_to_move = board.get_side_to_move() == WHITE;
      Square king_square = white_to_move ? E1 : E8;
      Square target_square = white_to_move ? C1 : C8;
      uint64_t mask = white_to_move
                          ? 0x0000000001010100
                          : 0x0000000080808000; // squares between king and rook
      if ((free_squares & mask) == mask) {
        res.emplace_back(Move(king_square, target_square));
      }
    }
    if (castling & KINGSIDE) {
      bool white_to_move = board.get_side_to_move() == WHITE;
      Square king_square = white_to_move ? E1 : E8;
      Square target_square = white_to_move ? G1 : G8;
      uint64_t mask = white_to_move
                          ? 0x0001010000000000
                          : 0x0080800000000000; // squares between king and rook
      if ((free_squares & mask) == mask) {
        res.emplace_back(Move(king_square, target_square));
      }
    }
  }
  return res;
}

vector<Move> MoveGenerator::generate_moves_legal(Board &board,
                                                 bool captures_only) {
  // TODO: Something more effective
  int8_t king = board.get_side_to_move() * KING;
  // Detect check
  // board.switch_sides();
  // vector<Move> replies = MoveGenerator::generate_moves_pseudo_legal(board);
  // bool checked = false;
  // for (Move &reply : replies)
  // {
  //     if (reply.get_captured_piece() == king)
  //     {
  //         checked = true;
  //         break;
  //     }
  // }
  // board.switch_sides();
  bool checked = detect_check(board);
  // if (checked) cout << "CHECK!!!\n" << board << endl;
  // Generate all moves
  vector<Move> all_moves =
      MoveGenerator::generate_moves_pseudo_legal(board, captures_only);
  bool kingside_through_check = false, queenside_through_check = false;
  for (auto it = all_moves.begin(); it != all_moves.end();) {
    board.make_move(*it);
    board.switch_sides();
    // vector<Move> replies = MoveGenerator::generate_moves_pseudo_legal(board);
    // bool legal = true;
    // for (Move &reply : replies)
    // {
    //     if (board.get_piece(reply.get_to()) == king)
    //     {
    //         legal = false;
    //         break;
    //     }
    // }
    // cout << m << " ";
    bool legal = !detect_check(board);
    board.switch_sides();
    board.unmake_move();
    kingside_through_check =
        kingside_through_check ||
        ((it->get_from() == ((king > 0) ? E1 : E8) &&
          it->get_to() == ((king > 0) ? F1 : F8)) &&
         (board.get_current_castling() & KINGSIDE) && !legal);
    queenside_through_check =
        queenside_through_check ||
        ((it->get_from() == ((king > 0) ? E1 : E8) &&
          it->get_to() == ((king > 0) ? D1 : D8)) &&
         (board.get_current_castling() & QUEENSIDE) && !legal);
    if ((it->get_from() == ((king > 0) ? E1 : E8) &&
         it->get_to() == ((king > 0) ? G1 : G8)) &&
        board.get_piece(it->get_from()) == king &&
        (kingside_through_check || checked)) {
      legal = false;
    }
    if ((it->get_from() == ((king > 0) ? E1 : E8) &&
         it->get_to() == ((king > 0) ? C1 : C8)) &&
        board.get_piece(it->get_from()) == king &&
        (queenside_through_check || checked)) {
      legal = false;
    }
    if (legal) {
      it++;
    } else {
      it = all_moves.erase(it);
    }
    // cout << legal << endl;
  }
  return all_moves;
}

bool MoveGenerator::detect_check(const Board &board) {
  int8_t color = board.get_side_to_move();
  uint64_t king_bitboard = board.get_bitboard(color * KING);
  uint64_t temp = king_bitboard;
  int8_t king_sq_int = 0;
/*while (temp >>= 1)
    {
        king_sq_int++;
    }*/
#ifdef _MSC_VER
  unsigned long idx;
  _BitScanForward64(&idx, king_bitboard);
  king_sq_int = idx;
#else
  king_sq_int = __builtin_ctzll(king_bitboard);
#endif // _MSC_VER

  Square king_sq = (Square)king_sq_int;
  uint64_t occupied = 0;
  uint64_t enemy_rooks_bitboard = board.get_bitboard(-color * ROOK);
  uint64_t enemy_bishops_bitboard = board.get_bitboard(-color * BISHOP);
  uint64_t enemy_queens_bitboard = board.get_bitboard(-color * QUEEN);
  // detect orthogonal check (by rooks and/or queens)
  // const Square *ortho_dirs[4] = { UP, DOWN, RIGHT, LEFT };
  // for (const Square *direction : ortho_dirs)
  // {
  // 	Square current = direction[king_sq];
  // 	while (current != INVALID)
  // 	{
  // 		int8_t piece = board.get_piece(current);
  // 		if (piece != NONE)
  // 		{
  // 			if ((piece == -color * ROOK) || (piece == -color * QUEEN)) // rook
  // or queen of opposite color
  // 			{
  // 				return true;
  // 			}
  // 			break;
  // 		}
  // 		current = direction[current];
  // 	}
  // }
  occupied = ~board.get_bitboard(NONE);
  occupied &= ROOK_MAGIC[king_sq].mask;
  occupied *= ROOK_MAGIC[king_sq].magic;
  occupied >>= (64 - ROOK_BITS[king_sq]);
  if (ROOK_ATTACKS_TABLE[king_sq][occupied] &
      (enemy_rooks_bitboard | enemy_queens_bitboard)) {
    return true;
  }
  // detect diagonal check (by bishops and/or queens)
  // const Square *dia_dirs[4] = {UP_RIGHT, UP_LEFT, DOWN_RIGHT, DOWN_LEFT};
  // for (const Square *direction : dia_dirs)
  // {
  // 	Square current = direction[king_sq];
  // 	while (current != INVALID)
  // 	{
  // 		int8_t piece = board.get_piece(current);
  // 		if (piece != NONE)
  // 		{
  // 			if ((piece == -color * BISHOP) || (piece == -color * QUEEN)) //
  // bishop or queen of opposite color
  // 			{
  // 				return true;
  // 			}
  // 			break;
  // 		}
  // 		current = direction[current];
  // 	}
  // }
  occupied = ~board.get_bitboard(NONE);
  occupied &= BISHOP_MAGIC[king_sq].mask;
  occupied *= BISHOP_MAGIC[king_sq].magic;
  occupied >>= (64 - BISHOP_BITS[king_sq]);
  if (BISHOP_ATTACKS_TABLE[king_sq][occupied] &
      (enemy_bishops_bitboard | enemy_queens_bitboard)) {
    return true;
  }
  // detect check by enemy king (actually impossible but helps for legality
  // check)
  uint64_t enemy_king_bitboard = board.get_bitboard(-color * KING);
  int8_t eking_sq_int = 0;
// while (enemy_king_bitboard >>= 1)
//{
//    eking_sq_int++;
//}
#ifdef _MSC_VER
  unsigned long eidx;
  _BitScanForward64(&eidx, enemy_king_bitboard);
  eking_sq_int = eidx;
#else
  eking_sq_int = __builtin_ctzll(enemy_king_bitboard);
#endif // _MSC_VER
  Square eking_sq = (Square)eking_sq_int;
  if (KING_PATTERNS_TABLE[eking_sq] & king_bitboard) {
    return true;
  }
  // detect check by enemy knights and pawns
  uint64_t enemy_knights_bitboard = board.get_bitboard(-color * KNIGHT);
  uint64_t enemy_pawns_bitboard = board.get_bitboard(-color * PAWN);
  // for (int i = 0; i < SQUARES_COUNT; i++)
  //{
  //    /*if ((enemy_knights_bitboard & (1ULL << i)) &&
  //    (KNIGHT_PATTERNS_TABLE[i] & king_bitboard))
  //    {
  //        return true;
  //    }*/
  //    if (enemy_pawns_bitboard & (1ULL << i))
  //    {
  //        Square forward = (Square)(i - color);
  //        Square left_forward = LEFT[forward], right_forward = RIGHT[forward];
  //        uint64_t lf_bitboard = (left_forward != INVALID) ? (1ULL <<
  //        left_forward) : 0; uint64_t rf_bitboard = (right_forward != INVALID)
  //        ? (1ULL << right_forward) : 0; if ((lf_bitboard | rf_bitboard) &
  //        king_bitboard)
  //        {
  //            return true;
  //        }
  //    }
  //}
  if (enemy_knights_bitboard & KNIGHT_PATTERNS_TABLE[king_sq]) {
    return true; // knight checks
  }
  Square backward = (Square)(king_sq + color);
  Square left_backward = LEFT[backward], right_backward = RIGHT[backward];
  uint64_t lb_bitboard =
      (left_backward != INVALID) ? (1ULL << left_backward) : 0;
  uint64_t rb_bitboard =
      (right_backward != INVALID) ? (1ULL << right_backward) : 0;
  if ((lb_bitboard | rb_bitboard) & enemy_pawns_bitboard) {
    return true; // pawn checks
  }
  return false;
}
