#include "eval.h"
#include <algorithm>
#include <random>

const int16_t PIECE_VALUES[PIECES_COUNT + 1] = {
	0,   // NONE
	100, // PAWN
	300, // KNIGHT
	325, // BISHOP
	475, // ROOK
	850, // QUEEN
	400, // KING
};

const int16_t MOBILITY_FACTOR = 5;

const int16_t NONE_EVAL_TABLE[SQUARES_COUNT] = {};

const int16_t PAWN_EVAL_TABLE[SQUARES_COUNT] = {
	0, 5, 5, 0, 5, 10, 50, 0,
	0, 10, -5, 0, 5, 10, 50, 0,
	0, 10, -10, 0, 10, 20, 50, 0,
	0, -20, 0, 20, 25, 30, 50, 0,
	0, -20, 0, 20, 25, 30, 50, 0,
	0, 10, -10, 0, 10, 20, 50, 0,
	0, 10, -5, 0, 5, 10, 50, 0,
	0, 5, 5, 0, 5, 10, 50, 0,
};

const int16_t KNIGHT_EVAL_TABLE[SQUARES_COUNT] = {
	-50, -40, -30, -30, -30, -30, -40, -50,
	-40, -20, 5, 0, 5, 0, -20, -40,
	-30, 0, 10, 15, 15, 10, 0, -30,
	-30, 5, 15, 20, 20, 15, 0, -30,
	-30, 5, 15, 20, 20, 15, 0, -30,
	-30, 0, 10, 15, 15, 10, 0, -30,
	-40, -20, 5, 0, 5, 0, -20, -40,
	-50, -40, -30, -30, -30, -30, -40, -50,
};

const int16_t BISHOP_EVAL_TABLE[SQUARES_COUNT] = {
	-20, -10, -10, -10, -10, -10, -10, -20,
	-10, 5, 10, 0, 5, 0, 0, -10,
	-10, 0, 10, 10, 5, 5, 0, -10,
	-10, 0, 10, 10, 10, 10, 0, -10,
	-10, 0, 10, 10, 10, 10, 0, -10,
	-10, 0, 10, 10, 5, 5, 0, -10,
	-10, 5, 10, 0, 5, 0, 0, -10,
	-20, -10, -10, -10, -10, -10, -10, -20,
};

const int16_t ROOK_EVAL_TABLE[SQUARES_COUNT] = {
	0, -5, -5, -5, -5, -5, 5, 0,
	0, 0, 0, 0, 0, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 10, 0,
	5, 0, 0, 0, 0, 0, 10, 0,
	5, 0, 0, 0, 0, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 10, 0,
	0, -5, -5, -5, -5, -5, 5, 0,
};

const int16_t QUEEN_EVAL_TABLE[SQUARES_COUNT] = {
	-20, -10, -10, 0, -5, -10, -10, -20,
	-10, 0, 5, 0, 0, 0, 0, -10,
	-10, 5, 5, 5, 5, 5, 0, -10,
	-5, 0, 5, 5, 5, 5, 0, -5,
	-5, 0, 5, 5, 5, 5, 0, -5,
	-10, 0, 5, 5, 5, 5, 0, -10,
	-10, 0, 0, 0, 0, 0, 0, -10,
	-20, -10, -10, -5, -5, -10, -10, -20,
};

const int16_t KING1_EVAL_TABLE[SQUARES_COUNT] = {
	20, 20, -10, -20, -30, -30, -30, -30,
	30, 20, -20, -30, -40, -40, -40, -40,
	10, 0, -20, -30, -40, -40, -40, -40,
	0, 0, -20, -40, -50, -50, -50, -50,
	0, 0, -20, -40, -50, -50, -50, -50,
	10, 0, -20, -30, -40, -40, -40, -40,
	30, 20, -20, -30, -40, -40, -40, -40,
	20, 20, -10, -20, -30, -30, -30, -30,
};

const int16_t KING2_EVAL_TABLE[SQUARES_COUNT] = {
	-50, -30, -30, -30, -30, -30, -30, -50,
	-30, -30, -10, -10, -10, -10, -20, -40,
	-30, 0, 20, 30, 30, 20, -10, -30,
	-30, 0, 30, 40, 40, 30, 0, -20,
	-30, 0, 30, 40, 40, 30, 0, -20,
	-30, 0, 20, 30, 30, 20, -10, -30,
	-30, -30, -10, -10, -10, -10, -20, -40,
	-50, -30, -30, -30, -30, -30, -30, -50,
};

// TODO : Make use of KING2
const int16_t *EVAL_TABLES[PIECES_COUNT + 1] = {
	NONE_EVAL_TABLE,
	PAWN_EVAL_TABLE,
	KNIGHT_EVAL_TABLE,
	BISHOP_EVAL_TABLE,
	ROOK_EVAL_TABLE,
	QUEEN_EVAL_TABLE,
	KING1_EVAL_TABLE,
};

bool helper_compare(Board &board, Move &m1, Move &m2)
{
	int16_t captured1_piece_value = PIECE_VALUES[abs(m1.get_captured_piece())];
	int16_t capturing1_piece_value = PIECE_VALUES[board.get_piecename(m1.get_from())];
	int16_t captured2_piece_value = PIECE_VALUES[abs(m2.get_captured_piece())];
	int16_t capturing2_piece_value = PIECE_VALUES[board.get_piecename(m2.get_from())];
	return (2 * captured1_piece_value - capturing1_piece_value) > (2 * captured2_piece_value - capturing2_piece_value);
}

int16_t EvaluationEngine::evaluate_final(Board &board)
{
	int16_t res = 0;
	int16_t pieces_count = 0;
	//normal_distribution<double> normal(1, 0.01);
	static uniform_int_distribution<int16_t> dist(-15, 15);
	mt19937_64& rng = Board::get_rng();
	for (int i = 0; i < SQUARES_COUNT; i++)
	{
		int8_t piece = board.get_piece((Square)i);
		pieces_count += (piece != 0);
	}
	bool endgame = (pieces_count <= 18) ||
		((board.get_bitboard(WHITE * QUEEN) | board.get_bitboard(BLACK * QUEEN)) == 0 && pieces_count <= 24);

	for (int i = 0; i < SQUARES_COUNT; i++)
	{
		int8_t piece = board.get_piece((Square)i);
		const int16_t *eval_table = (endgame && abs(piece) == KING) ? KING2_EVAL_TABLE : EVAL_TABLES[abs(piece)];
		res += (PIECE_VALUES[abs(piece)] + eval_table[piece > 0 ? i : (i & 56 | (7 - i & 7))]) * (piece > 0 ? 1 : -1);
	}
	int16_t delta = dist(rng);
	res += delta;
	// int legal_moves = MoveGenerator::generate_moves_legal(board).size();
	// board.switch_sides();
	// int opponent_legal_moves = MoveGenerator::generate_moves_legal(board).size();
	// board.switch_sides();
	// res += MOBILITY_FACTOR * (legal_moves - opponent_legal_moves);
	res *= board.get_side_to_move();
	return res;
}

int16_t EvaluationEngine::evaluate_depth(Board &board, int depth, int16_t alpha, int16_t beta,
	bool show_moves, ostream& output, 
	map<uint64_t, pair<int16_t, int>> &hash_table, map<uint64_t, Move> &hash_var,
	clock_t scheduled)
{
	int16_t best_score = MIN_SCORE;
	if (depth == 0 || clock() > scheduled)
	{
		int16_t res = evaluate_quiesce(board, alpha, beta);
		//EvaluationResult res = evaluate_final(board);
		hash_table[board.get_hash()] = { res, depth };
		return res;
	}
	vector<Move> moves = MoveGenerator::generate_moves_legal(board);
	if (moves.empty())
	{
		if (MoveGenerator::detect_check(board))
		{
			int16_t checkmate_score = board.get_side_to_move() == WHITE ? MIN_SCORE : MAX_SCORE;
			hash_table[board.get_hash()] = { checkmate_score, depth };
			return hash_table[board.get_hash()].first;
		}
		else
		{
			hash_table[board.get_hash()] = { 0, depth };
			return hash_table[board.get_hash()].first;
		}
	}
	sort(moves.begin(), moves.end(), [&board](Move m1, Move m2) { return helper_compare(board, m1, m2); });
	int index = 0;
	for (Move m : moves)
	{
		if (show_moves)
		{
			output << "info currmove " << m << " currmovenumber " << (++index) << endl;
		}
		//cout << depth << m << endl;
		//cout << alpha << " " << beta << endl;
		board.make_move(m);
		int16_t er;
		if (hash_table.count(board.get_hash()) /*&& depth > hash_table[board.get_hash()].second*/)
		{
			er = hash_table[board.get_hash()].first;
		}
		else
		{
			er = -evaluate_depth(board, depth - 1, -beta, -alpha, false, output, hash_table, hash_var, scheduled);
			hash_table[board.get_hash()] = { er, depth };
		}
		board.unmake_move();
		if (er >= beta)
		{
			return er;
		}
		if (er > best_score)
		{
			best_score = er;
			if (er > alpha)
			{
				alpha = er;
			}
			hash_var[board.get_hash()] = m;
		}
	}
	//cout << "depth=" << depth << " pv=" << variation.size() << endl;
	return alpha;
}

int16_t EvaluationEngine::evaluate_quiesce(Board &board, int16_t alpha, int16_t beta)
{
	vector<Move> legal_moves = MoveGenerator::generate_moves_legal(board);
	if (legal_moves.empty())
	{
		if (MoveGenerator::detect_check(board))
		{
			int16_t checkmate_score = board.get_side_to_move() == WHITE ? MIN_SCORE : MAX_SCORE;
			return checkmate_score;
		}
		else
		{
			return 0;
		}
	}
	int16_t er = evaluate_final(board);
	if (er >= beta)
	{
		return er;
	}
	if (alpha < er)
	{
		alpha = er;
	}
	for (Move &m : legal_moves)
	{
		if (m.get_captured_piece())
		{
			board.make_move(m);
			int16_t ner = -evaluate_quiesce(board, -beta, -alpha);
			board.unmake_move();
			if (ner >= beta)
			{
				return beta;
			}
			if (ner > alpha)
			{
				alpha = ner;
			}
		}
	}
	return alpha;
}