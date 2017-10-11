#include "eval.h"
#include <algorithm>

const int16_t PIECE_VALUES[PIECES_COUNT + 1] = {
    0,   // NONE
    100, // PAWN
    300, // KNIGHT
    325, // BISHOP
    475, // ROOK
    850, // QUEEN
    400, // KING
};

const int16_t MOBILITY_FACTOR = 10;

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
    for (int i = 0; i < SQUARES_COUNT; i++)
    {
        int8_t piece = board.get_piece((Square)i);
        res += PIECE_VALUES[abs(piece)] * (piece > 0 ? 1 : -1);
    }
    int legal_moves = MoveGenerator::generate_moves_legal(board).size();
    board.switch_sides();
    int opponent_legal_moves = MoveGenerator::generate_moves_legal(board).size();
    board.switch_sides();
    res += MOBILITY_FACTOR * (legal_moves - opponent_legal_moves);
    return {res, variation};
}

EvaluationResult EvaluationEngine::evaluate_depth(Board &board, int depth, int16_t &alpha, int16_t &beta, vector<Move> &variation)
{
    if (depth == 0)
    {
        return evaluate_quiesce(board, alpha, beta, variation);
    }
    vector<Move> moves = MoveGenerator::generate_moves_legal(board);
    if (moves.empty())
    {
        if (MoveGenerator::detect_check(board))
        {
            int16_t checkmate_score = board.get_side_to_move() == WHITE ? MIN_SCORE : MAX_SCORE;
            return {checkmate_score, variation};
        }
    }
    sort(moves.begin(), moves.end(), [&board](Move &m1, Move &m2) { return helper_compare(board, m1, m2); });
    if (board.get_side_to_move() == WHITE)
    {
        int16_t v = MIN_SCORE;
        vector<Move> newvar;
        for (Move &m : moves)
        {
            newvar = vector<Move>(variation.begin(), variation.end());
            newvar.push_back(m);
            board.make_move(m);
            EvaluationResult er = evaluate_depth(board, depth - 1, alpha, beta, newvar);
            newvar = er.variation;
            v = max(v, er.score);
            alpha = max(alpha, v);
            board.unmake_move();
            if (beta < alpha)
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
        for (Move &m : moves)
        {
            newvar = vector<Move>(variation.begin(), variation.end());
            newvar.push_back(m);
            board.make_move(m);
            EvaluationResult er = evaluate_depth(board, depth - 1, alpha, beta, newvar);
            newvar = er.variation;
            v = min(v, er.score);
            beta = min(beta, v);
            board.unmake_move();
            if (beta < alpha)
            {
                break;
            }
        }
        return {v, newvar};
    }
}

EvaluationResult EvaluationEngine::evaluate_quiesce(Board &board, int16_t &alpha, int16_t &beta, vector<Move> &variation)
{
    // placeholder, no quiesce yet
    return evaluate_final(board, variation);
}