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

EvaluationResult EvaluationEngine::evaluate_final(Board &board, vector<Move> &variation)
{
    int16_t res = 0;
    normal_distribution<double> normal(1, 0.01);
    mt19937_64 rng = Board::get_rng();
    for (int i = 0; i < SQUARES_COUNT; i++)
    {
        int8_t piece = board.get_piece((Square)i);
        res += round((PIECE_VALUES[abs(piece)] + EVAL_TABLES[abs(piece)][piece > 0 ? i : (i & 56 | (7 - i & 7))]) * (piece > 0 ? 1 : -1) * normal(rng));
    }
    // int legal_moves = MoveGenerator::generate_moves_legal(board).size();
    // board.switch_sides();
    // int opponent_legal_moves = MoveGenerator::generate_moves_legal(board).size();
    // board.switch_sides();
    // res += MOBILITY_FACTOR * (legal_moves - opponent_legal_moves);
    return {res, variation};
}

EvaluationResult EvaluationEngine::evaluate_depth(Board &board, int depth, int16_t alpha, int16_t beta, vector<Move> &variation, vector<Move> movelist, map<uint64_t, EvaluationResult> &hash_table)
{
    if (depth == 0)
    {
        EvaluationResult res = evaluate_quiesce(board, alpha, beta, variation);
        //EvaluationResult res = evaluate_final(board, variation);
        hash_table[board.get_hash()] = res;
        return res;
    }
    vector<Move> moves = movelist;
    if (moves.empty())
    {
        moves = MoveGenerator::generate_moves_legal(board);
    }
    if (moves.empty())
    {
        if (MoveGenerator::detect_check(board))
        {
            int16_t checkmate_score = board.get_side_to_move() == WHITE ? MIN_SCORE : MAX_SCORE;
            hash_table[board.get_hash()] = {checkmate_score, variation};
            return hash_table[board.get_hash()];
        }
        else
        {
            hash_table[board.get_hash()] = {0, variation};
            return hash_table[board.get_hash()];
        }
    }
    sort(moves.begin(), moves.end(), [&board](Move m1, Move m2) { return helper_compare(board, m1, m2); });
    if (board.get_side_to_move() == WHITE)
    {
        int16_t v = MIN_SCORE;
        vector<Move> newvar;
        for (Move m : moves)
        {
            //cout << depth << m << endl;
            //cout << alpha << " " << beta << endl;
            newvar = vector<Move>(variation.begin(), variation.end());
            newvar.push_back(m);
            board.make_move(m);
            EvaluationResult er;
            if (hash_table.count(board.get_hash()))
            {
                er = hash_table[board.get_hash()];
            }
            else
            {
                er = evaluate_depth(board, depth - 1, alpha, beta, newvar, {}, hash_table);
                hash_table[board.get_hash()] = er;
            }
            newvar = er.variation;
            v = max(v, er.score);
            alpha = max(alpha, v);
            board.unmake_move();
            if (beta <= v)
            {
                break;
            }
        }
        return {v, newvar};
    }
    else
    {
        int16_t v = MAX_SCORE;
        vector<Move> newvar;
        for (Move m : moves)
        {
            //cout << depth << m << endl;
            //cout << alpha << " " << beta << endl;
            newvar = vector<Move>(variation.begin(), variation.end());
            newvar.push_back(m);
            board.make_move(m);
            EvaluationResult er;
            if (hash_table.count(board.get_hash()))
            {
                er = hash_table[board.get_hash()];
            }
            else
            {
                er = evaluate_depth(board, depth - 1, alpha, beta, newvar, {}, hash_table);
                hash_table[board.get_hash()] = er;
            }
            newvar = er.variation;
            v = min(v, er.score);
            beta = min(beta, v);
            board.unmake_move();
            if (v <= alpha)
            {
                break;
            }
        }
        return {v, newvar};
    }
}

EvaluationResult EvaluationEngine::evaluate_quiesce(Board &board, int16_t alpha, int16_t beta, vector<Move> &variation)
{
    EvaluationResult er = evaluate_final(board, variation);
    if (er.score >= beta)
    {
        return er;
    }
    if (alpha < er.score)
    {
        alpha = er.score;
    }
    vector<Move> legal_moves = MoveGenerator::generate_moves_legal(board);
    for (Move m : legal_moves)
    {
        if (m.get_captured_piece())
        {
            vector<Move> newvar = vector<Move>(variation.begin(), variation.end());
            newvar.push_back(m);
            board.make_move(m);
            EvaluationResult ner = evaluate_quiesce(board, alpha, beta, newvar);
            board.unmake_move();
            if (ner.score >= beta)
            {
                er.score = beta;
                return er;
            }
            if (ner.score > alpha)
            {
                alpha = ner.score;
            }
        }
    }
    er.score = alpha;
    return er;
}