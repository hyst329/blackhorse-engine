#include "board.h"

struct EvaluationResult
{
    int16_t score;
    vector<Move> variation;
};

class EvaluationEngine
{
public:
    static EvaluationResult evaluate(Board& board, int depth, vector<Move> & variation) {
        int16_t alpha = MIN_SCORE, beta = MAX_SCORE;
        return evaluate_depth(board, depth, alpha, beta, variation);
    }
    static const int16_t MAX_SCORE = 20000, MIN_SCORE = -20000;
private:
    static EvaluationResult evaluate_final(Board &board, vector<Move> &variation);
    static EvaluationResult evaluate_depth(Board &board, int depth, int16_t& alpha, int16_t& beta, vector<Move> &variation);
    static EvaluationResult evaluate_quiesce(Board &board, int16_t& alpha, int16_t& beta, vector<Move> &variation);
};