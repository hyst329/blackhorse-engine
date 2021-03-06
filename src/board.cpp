#include "board.h"
#ifdef WIN32
#include <windows.h>
// this line left blank to prevent clang-format import sorting
#include <wincrypt.h>
#endif
#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <sstream>

uint64_t Board::zobrist_tables[SQUARES_COUNT][TOTAL_PIECES_COUNT]{};
bool Board::zobrist_initialised = false;
mt19937_64 Board::rng;

uint64_t ROOK_ATTACKS_TABLE[64][4096];
uint64_t BISHOP_ATTACKS_TABLE[64][512];

void Board::zobrist_initialise() {
  // currently we ignore castling ability and en passant

  array<uint8_t, 2496> seedData;
#ifdef WIN32
  HCRYPTPROV p = 0;
  CryptAcquireContextW(&p, 0, 0, PROV_RSA_FULL,
                       CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
  CryptGenRandom(p, seedData.size(), seedData.data());
  CryptReleaseContext(p, 0);
#else
  std::random_device rd;
  std::generate_n(seedData.data(), seedData.size(), std::ref(rd));
#endif
  std::seed_seq seq(std::begin(seedData), std::end(seedData));
  rng = std::mt19937_64(seq);

  uniform_int_distribution<uint64_t> d(
      0, (1ULL << 63) - 1); // the MSB will be used for side to move
  for (int i = 0; i < SQUARES_COUNT; i++) {
    for (int j = 0; j < TOTAL_PIECES_COUNT; j++) {
      zobrist_tables[i][j] = d(rng);
    }
  }
  // initialise bishop and rook attacks squares
  ifstream rattack("blackhorse_rattacks.dat", ios::binary);
  ifstream battack("blackhorse_battacks.dat", ios::binary);
  for (int i = 0; i < 64; i++) {
    rattack.read((char *)ROOK_ATTACKS_TABLE[i], 4096 * sizeof(uint64_t));
    battack.read((char *)BISHOP_ATTACKS_TABLE[i], 512 * sizeof(uint64_t));
  }
}

Board::Board() {
  if (!zobrist_initialised)
    zobrist_initialise();
  hash = 0;
  for (int i = 0; i < 64; i++) {
    mailbox_board[i] = NONE;
    hash ^= zobrist_tables[i][NONE + PIECE_OFFSET];
  }
  hash |= (1ULL << 63);
  for (int i = 0; i < TOTAL_PIECES_COUNT; i++)
    piece_bitboards[i] = 0;
  piece_bitboards[PIECE_OFFSET] = UINT64_MAX;
  move_number = 1;
  halfmove_counter = 0;
  side_to_move = WHITE;
  white_castling = NONE;
  black_castling = NONE;
  en_passant = NONE;
  const int LIMIT = 2048;
  move_history.reserve(LIMIT);
  half_moves.reserve(LIMIT);
  castlings.reserve(LIMIT);
  en_passants.reserve(LIMIT);
}

Board::Board(string fen) : Board() {
  int rank = 8, file = 1;
  istringstream fen_ss(fen);
  string pos, side_to_move_char, castling, en_passant_square;
  fen_ss >> pos >> side_to_move_char >> castling >> en_passant_square >>
      halfmove_counter >> move_number;
  for (char c : pos) {
    switch (c) {
    case 'K':
      set_piece(from_file_rank(file, rank), WHITE * KING);
      file++;
      break;
    case 'k':
      set_piece(from_file_rank(file, rank), BLACK * KING);
      file++;
      break;
    case 'Q':
      set_piece(from_file_rank(file, rank), WHITE * QUEEN);
      file++;
      break;
    case 'q':
      set_piece(from_file_rank(file, rank), BLACK * QUEEN);
      file++;
      break;
    case 'R':
      set_piece(from_file_rank(file, rank), WHITE * ROOK);
      file++;
      break;
    case 'r':
      set_piece(from_file_rank(file, rank), BLACK * ROOK);
      file++;
      break;
    case 'B':
      set_piece(from_file_rank(file, rank), WHITE * BISHOP);
      file++;
      break;
    case 'b':
      set_piece(from_file_rank(file, rank), BLACK * BISHOP);
      file++;
      break;
    case 'N':
      set_piece(from_file_rank(file, rank), WHITE * KNIGHT);
      file++;
      break;
    case 'n':
      set_piece(from_file_rank(file, rank), BLACK * KNIGHT);
      file++;
      break;
    case 'P':
      set_piece(from_file_rank(file, rank), WHITE * PAWN);
      file++;
      break;
    case 'p':
      set_piece(from_file_rank(file, rank), BLACK * PAWN);
      file++;
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      file += (c - '0');
      break;
    case '/':
      rank--;
      file = 1;
      break;
    }
  }
  side_to_move = (side_to_move_char == "w") ? WHITE : BLACK;
  if (side_to_move == BLACK) {
    hash ^= (1ULL << 63);
  }
  for (char c : castling) {
    switch (c) {
    case 'K':
      white_castling |= KINGSIDE;
      break;
    case 'Q':
      white_castling |= QUEENSIDE;
      break;
    case 'k':
      black_castling |= KINGSIDE;
      break;
    case 'q':
      black_castling |= QUEENSIDE;
      break;
    }
  }
  en_passant =
      en_passant_square[0] == '-' ? 0 : (en_passant_square[0] - 'a' + 1);
  en_passants.push_back(en_passant);
  castlings.push_back(white_castling << 2 | black_castling);
  hashes.push_back(hash);
}

ostream &operator<<(ostream &os, const Board &board) {
  for (int rank = 8; rank >= 1; rank--) {
    os << rank;
    for (int file = 1; file <= 8; file++) {
      int sq = Board::from_file_rank(file, rank);
      os << " "
         << "kqrbnp.PNBRQK"[board.mailbox_board[sq] + KING];
    }
    os << endl;
  }
  os << "  a b c d e f g h" << endl;
  // for (int i = BLACK * KING; i <= WHITE * KING; i++)
  // {
  //     os << hex << setfill('0') << setw(16) << board.piece_bitboards[i +
  //     KING] << endl;
  // }
  os << board.move_number << (board.side_to_move == WHITE ? ". " : "...") << "?"
     << endl;
  return os;
}

void Board::make_move(Move move) {
  int8_t piece = get_piece(move.get_from());
  if (abs(piece) == KING && abs(move.get_from() - move.get_to()) == 16) {
    // it's a castling
    Square rook_square =
        (Square)((move.get_to() / 8 > 4) * 56 + move.get_from() % 8);
    set_piece(move.get_from(), NONE);
    set_piece(move.get_to(), piece);
    int8_t rook = get_piece(rook_square);
    Square new_rook_square =
        (Square)(rook_square + ((move.get_to() / 8 > 4) ? -16 : 24));
    set_piece(rook_square, NONE);
    set_piece(new_rook_square, rook);
    move_history.push_back(move);
    (side_to_move == WHITE ? white_castling : black_castling) = NONE;
    side_to_move = -side_to_move;
    hash ^= (1ULL << 63);
    move_number += (side_to_move == WHITE);
    halfmove_counter++;
    en_passant = 0;
  } else {
    int8_t captured_piece = get_piece(move.get_to());
    int8_t promoted_piece = move.get_promoted_piece();
    set_piece(move.get_from(), NONE);
    set_piece(move.get_to(), promoted_piece ? promoted_piece : piece);
    move.set_captured_piece(captured_piece);
    move_history.push_back(move);
    // remove castling rights
    if (abs(piece) == KING) {
      (side_to_move == WHITE ? white_castling : black_castling) = NONE;
    }
    if ((abs(piece) == ROOK && move.get_from() == A1) || move.get_to() == A1) {
      white_castling &= KINGSIDE;
    }
    if ((abs(piece) == ROOK && move.get_from() == H1) || move.get_to() == H1) {
      white_castling &= QUEENSIDE;
    }
    if ((abs(piece) == ROOK && move.get_from() == A8) || move.get_to() == A8) {
      black_castling &= KINGSIDE;
    }
    if ((abs(piece) == ROOK && move.get_from() == H8) || move.get_to() == H8) {
      black_castling &= QUEENSIDE;
    }
    side_to_move = -side_to_move;
    hash ^= (1ULL << 63);
    move_number += (side_to_move == WHITE);
    if (/*abs(piece) == PAWN && en_passant*/ move.get_en_passant()) {
      // check for en passant capture
      // if (from_file_rank(en_passant, side_to_move == WHITE ? 3 : 6) ==
      // move.get_to())
      //{
      Square actual_pawn_square = (Square)(move.get_to() + side_to_move);
      set_piece(actual_pawn_square, NONE);
      //}
    }
    if (abs(piece) == PAWN && abs(move.get_from() - move.get_to()) == 2) {
      // it's a two-square pawn push, so we set the en-passant file
      en_passant = (move.get_from() >> 3) + 1;
    } else {
      en_passant = 0;
    }
    if (abs(piece) == PAWN || captured_piece) {
      half_moves.push_back(halfmove_counter);
      halfmove_counter = 0;
    } else
      halfmove_counter++;
  }
  en_passants.push_back(en_passant);
  castlings.push_back(white_castling << 2 | black_castling);
  hashes.push_back(hash);
}

Move Board::unmake_move() {
  en_passants.pop_back();
  en_passant = en_passants.back();
  castlings.pop_back();
  white_castling = castlings.back() >> 2;
  black_castling = castlings.back() & 3;
  hashes.pop_back();
  uint64_t new_hash = hashes.back();
  Move move = move_history.back();
  move_history.pop_back();
  int8_t piece = get_piece(move.get_to());
  if (abs(piece) == KING && abs(move.get_from() - move.get_to()) == 16) {
    // it's a castling
    Square rook_square =
        (Square)((move.get_to() / 8 > 4) * 56 + move.get_from() % 8);
    Square new_rook_square =
        (Square)(rook_square + ((move.get_to() / 8 > 4) ? -16 : 24));
    int8_t rook = get_piece(new_rook_square);
    set_piece(move.get_from(), piece);
    set_piece(move.get_to(), NONE);
    set_piece(new_rook_square, NONE);
    set_piece(rook_square, rook);
    side_to_move = -side_to_move;
    hash ^= (1ULL << 63);
    move_number -= (side_to_move == BLACK);
    halfmove_counter--;
  } else {
    bool en_passant_capture = move.get_en_passant();
    int8_t promoted_piece = move.get_promoted_piece();
    set_piece(move.get_from(), promoted_piece ? -side_to_move * PAWN : piece);
    if (en_passant_capture) {
      set_piece(from_file_rank(en_passant, side_to_move == WHITE ? 4 : 5),
                side_to_move * PAWN);
      set_piece(move.get_to(), NONE);
    } else {
      set_piece(move.get_to(), move.get_captured_piece());
    }
    side_to_move = -side_to_move;
    hash ^= (1ULL << 63);
    move_number -= (side_to_move == BLACK);
    if (halfmove_counter == 0) {
      if (!half_moves.empty()) {
        halfmove_counter = half_moves.back();
        half_moves.pop_back();
      }
    } else
      halfmove_counter--;
  }
  if (hash != new_hash) {
    cerr << "Hashes not matching" << endl;
    cerr << *this;
    cerr << "Move history: " << endl;
    while (!move_history.empty()) {
      cerr << move_history.back() << " ";
      move_history.pop_back();
    }
    cerr << endl << "Attempting to undo move " << move << endl;
    exit(1);
  }
  return move;
}

string Board::get_fen() const {
  string res;
  int empty_squares = 0;
  for (int rank = 8; rank >= 1; rank--) {
    for (int file = 1; file <= 8; file++) {
      switch (mailbox_board[from_file_rank(file, rank)]) {
      case (WHITE * PAWN):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'P';
        empty_squares = 0;
        break;
      case (WHITE * KNIGHT):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'N';
        empty_squares = 0;
        break;
      case (WHITE * BISHOP):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'B';
        empty_squares = 0;
        break;
      case (WHITE * ROOK):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'R';
        empty_squares = 0;
        break;
      case (WHITE * QUEEN):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'Q';
        empty_squares = 0;
        break;
      case (WHITE * KING):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'K';
        empty_squares = 0;
        break;
      case (BLACK * PAWN):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'p';
        empty_squares = 0;
        break;
      case (BLACK * KNIGHT):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'n';
        empty_squares = 0;
        break;
      case (BLACK * BISHOP):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'b';
        empty_squares = 0;
        break;
      case (BLACK * ROOK):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'r';
        empty_squares = 0;
        break;
      case (BLACK * QUEEN):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'q';
        empty_squares = 0;
        break;
      case (BLACK * KING):
        if (empty_squares) {
          res += to_string(empty_squares);
        }
        res += 'k';
        empty_squares = 0;
        break;
      case NONE:
        empty_squares++;
        break;
      }
    }
    if (empty_squares) {
      res += to_string(empty_squares);
    }
    if (rank > 1) {
      res += '/';
      empty_squares = 0;
    }
  }
  res += side_to_move == WHITE ? " w " : " b ";
  if (white_castling & KINGSIDE) {
    res += "K";
  }
  if (white_castling & QUEENSIDE) {
    res += "Q";
  }
  if (black_castling & KINGSIDE) {
    res += "k";
  }
  if (black_castling & QUEENSIDE) {
    res += "q";
  }
  if (white_castling | black_castling) {
    res += ' ';
  }
  res += (en_passant > 0 ? string(1, 'a' + en_passant - 1) +
                               (side_to_move == WHITE ? '6' : '3')
                         : "-");
  res += ' ';
  res += to_string(halfmove_counter) + ' ';
  res += to_string(move_number);
  return res;
}
