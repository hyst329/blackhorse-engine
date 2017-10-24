#ifndef EVAL_H
#define EVAL_H

#include "board.h"

#include <map>
#include <atomic>
#include <ctime>

class EvaluationEngine
{
public:
  static int16_t evaluate(Board &board, int depth, bool show_moves, ostream &output,
                          map<uint64_t, pair<int16_t, int>> &hash_table, map<uint64_t, Move> &hash_var,
                          clock_t scheduled, Move bestmove)
  {
      int16_t alpha = MIN_SCORE, beta = MAX_SCORE;
      return evaluate_depth(board, depth, depth, alpha, beta, show_moves, output, hash_table, hash_var, scheduled, bestmove);
    }
    static const int16_t MAX_SCORE = 20000, MIN_SCORE = -20000;
private:
    static int16_t evaluate_final(Board &board);
    static int16_t evaluate_depth(Board &board, int depth, int odepth, int16_t alpha, int16_t beta, bool show_moves, ostream &output,
                                  map<uint64_t, pair<int16_t, int>> &hash_table, map<uint64_t, Move> &hash_var,
                                  clock_t scheduled, Move bestmove);
    static int16_t evaluate_quiesce(Board &board, int16_t alpha, int16_t beta);
};

#endif