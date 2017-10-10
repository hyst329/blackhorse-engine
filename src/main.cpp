#include <iostream>

#include "board.h"
#include "move.h"

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
    // cout << board;
    // cout << board.get_hash() << endl;
    // cout << board.get_fen() << endl;
    // vector<Move> moves = MoveGenerator::generate_moves_legal(board);
    // for (Move move : moves)
    //     cout << move << " ";
    // cout << endl;
    // board.make_move(Move(E2, E4));
    // board.make_move(Move(E7, E5));
    // board.make_move(Move(G1, F3));
    // board.make_move(Move(B8, C6));
    // board.make_move(Move(F1, B5));
    // board.make_move(Move(G8, F6));
    // board.make_move(Move(E1, G1));
    // board.make_move(Move(A7, A6));
    // board.make_move(Move(B1, C3));
    // cout << board;
    // cout << board.get_hash() << endl;
    // cout << board.get_fen() << endl;
    // moves = MoveGenerator::generate_moves_legal(board);
    // for (Move move : moves)
    //     cout << move << " ";
    // cout << endl;
    // board.unmake_move();
    // board.unmake_move();
    // board.unmake_move();
    // board.unmake_move();
    // board.unmakMese_move();
    // board.unmake_move();
    // board.unmake_move();
    // board.unmake_move();
    // board.unmake_move();
    // cout << board;
    // cout << board.get_hash() << endl;
    // cout << board.get_fen() << endl;
    // moves = MoveGenerator::generate_moves_legal(board);
    // for (Move move : moves)
    //     cout << move << " ";
    // cout << endl;
    // board.make_move(Move(E2, E4));
    // board.make_move(Move(C7, C5));
    // board.make_move(Move(H2, H4));
    // board.make_move(Move(F7, F5)); 
    // board.make_move(Move(B1, C3)); 
    // board.make_move(Move(B8, C6)); 
    // board.unmake_move(); 
    // board.unmake_move(); 
    //vector<Move> moves = MoveGenerator::generate_moves_legal(board);
    //for (Move move : moves)
    //    cout << move << " ";
    //for (int i = 1; i <= 5; i++)
    cout << perft_test(5) << endl;
    // cout << endl << endl;
    // cout << board.get_hash() << endl;
    // cout << board.get_fen() << endl;
        // return 0;
}