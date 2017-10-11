#include <iostream>

#include "board.h"
#include "move.h"
#include "eval.h"

int perft_test(Board& board, int depth, bool divide_out = true)
{
    if (depth == 0)
    {
        return 0;
    }
    uint64_t nodes = 0;
    vector<Move> legal_moves = MoveGenerator::generate_moves_legal(board);
    if (depth == 1)
    {
        return legal_moves.size();
    }
    for (Move& m : legal_moves)
    {
        board.make_move(m);
        int pt = perft_test(board, depth - 1, false);
        if (divide_out)
        {
            //cout << board << endl;
            cout << m << " " << pt << endl;
        }
        nodes += pt;
        board.unmake_move();
    }
    return nodes;
}

int perft_test(int depth)
{
    Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    return perft_test(board, depth);
}

int main()
{
    Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    vector<Move> variation;
    EvaluationResult er = EvaluationEngine::evaluate(board, 14, variation);
    cout << "Eval score: " << er.score << endl;
    cout << er.variation.size() << endl;
    for (Move& m: er.variation)
    {
        cout << m << " ";
    }
    cout << endl;
    return 0;
}