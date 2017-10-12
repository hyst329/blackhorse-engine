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
        res += round(PIECE_VALUES[abs(piece)] * (piece > 0 ? 1 : -1) * normal(rng));
    }
    int legal_moves = MoveGenerator::generate_moves_legal(board).size();
    board.switch_sides();
    int opponent_legal_moves = MoveGenerator::generate_moves_legal(board).size();
    board.switch_sides();
    res += MOBILITY_FACTOR * (legal_moves - opponent_legal_moves);
    return {res, variation};
}

EvaluationResult EvaluationEngine::evaluate_depth(Board &board, int depth, int16_t &alpha, int16_t &beta, vector<Move> &variation, map<uint64_t, EvaluationResult> &hash_table)
{
    if (depth == 0)
    {
        //EvaluationResult res = evaluate_quiesce(board, alpha, beta, variation);
        EvaluationResult res = evaluate_final(board, variation);
        hash_table[board.get_hash()] = res;
        return res;
    }
    vector<Move> moves = MoveGenerator::generate_moves_legal(board);
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
    sort(moves.begin(), moves.end(), [&board](Move &m1, Move &m2) { return helper_compare(board, m1, m2); });
    if (board.get_side_to_move() == WHITE)
    {
        int16_t v = MIN_SCORE;
        vector<Move> newvar;
        for (Move &m : moves)
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
                er = evaluate_depth(board, depth - 1, alpha, beta, newvar, hash_table);
                hash_table[board.get_hash()] = er;
            }
            newvar = er.variation;
            v = max(v, er.score);
            alpha = max(alpha, v);
            board.unmake_move();
            // if (beta <= v)
            // {
            //     break;
            // }
        }
        return {v, newvar};
    }
    else
    {
        int16_t v = MAX_SCORE;
        vector<Move> newvar;
        for (Move &m : moves)
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
                er = evaluate_depth(board, depth - 1, alpha, beta, newvar, hash_table);
                hash_table[board.get_hash()] = er;
            }
            newvar = er.variation;
            v = min(v, er.score);
            beta = min(beta, v);
            board.unmake_move();
            // if (v <= alpha)
            // {
            //     break;
            // }
        }
        return {v, newvar};
    }
}

EvaluationResult EvaluationEngine::evaluate_quiesce(Board &board, int16_t &alpha, int16_t &beta, vector<Move> &variation)
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
    for (Move &m : legal_moves)
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