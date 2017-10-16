#include "uci.h"
#include "eval.h"

#include <sstream>
#include <algorithm>
#include <ctime>
#include <thread>
#include <future>

void UCI::main_loop()
{
    bool uci_initialised = false;
    for (;;)
    {
        string command, parameters;
        input >> command;
        getline(input, parameters);
        if (command == "quit")
        {
            break;
        }
        if (command == "uci")
        {
            output << "id name Blackhorse 0.0" << endl;
            output << "id author trolley.813 (RUS)" << endl;
            uci_initialised = true;
            output << "uciok" << endl;
        }
        if (uci_initialised)
        {
            if (command == "ucinewgame")
            {
                // TODO: ucinewgame handling
            }
            if (command == "isready")
            {
                // Ready always!
                output << "readyok" << endl;
            }
            if (command == "setoption")
            {
                // TODO: handle real options, currently unsupported
            }
            if (command == "position")
            {
                size_t pos = parameters.find("moves");
                string position_description = parameters.substr(0, pos), moves = "";
                if (pos != string::npos)
                {
                    moves = parameters.substr(pos + 5); // length of "moves"
                }
                size_t pdstart = position_description.find_first_not_of(' ');
                size_t pdend = position_description.find_last_not_of(' ');
                position_description = position_description.substr(pdstart, pdend - pdstart + 1);
                if (position_description.substr(0, 3) == "fen")
                {
                    board = Board(position_description.substr(3));
                }
                if (position_description.substr(0, 8) == "startpos")
                {
                    board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                }
                if (!moves.empty())
                {
                    istringstream iss(moves);
                    string move_str;
                    while (iss >> move_str)
                    {
                        Move m(move_str);
                        vector<Move> legal_moves = MoveGenerator::generate_moves_legal(board);
                        auto it = find_if(legal_moves.begin(), legal_moves.end(),
                                          [m](Move n) { return n.get_from() == m.get_from() && n.get_to() == m.get_to(); });
                        if (it != legal_moves.end())
                        {
                            board.make_move(*it);
                        }
                    }
                }
            }
            if (command == "go")
            {
                int depth = 0, max_time = 0;
                int wtime = 0, btime = 0, winc = 0, binc = 0;
                string param_name;
                istringstream iss(parameters);
                while (iss >> param_name)
                {
                    if (param_name == "wtime")
                    {
                        iss >> wtime;
                    }
                    if (param_name == "btime")
                    {
                        iss >> btime;
                    }
                    if (param_name == "winc")
                    {
                        iss >> winc;
                    }
                    if (param_name == "binc")
                    {
                        iss >> binc;
                    }
                    if (param_name == "depth")
                    {
                        iss >> depth;
                    }
                    if (param_name == "movetime")
                    {
                        iss >> max_time;
                    }
                    if (param_name == "infinite")
                    {
                        depth = 99;
                        max_time = 10800000; // 3 hours
                    }
                }
                if (depth == 0)
                {
                    depth = 20;
                }
                if (max_time == 0)
                {
                    max_time = (board.get_side_to_move() == WHITE ? wtime : btime) / 20;
                }
                if (max_time == 0)
                {
                    max_time = 180000;
                }
                output << "depth " << depth << " max_time " << max_time << endl;
                thinking.store(true);
                thread t([this, depth, max_time]() { return output_thinking(depth, max_time); });
                t.detach();
            }
            if (command == "stop")
            {
                // TODO: Stop thinking
                thinking.store(false);
            }
            if (command == "print")
            {
                output << board << endl;
            }
        }
    }
}

void UCI::output_thinking(int max_depth, int max_time)
{
    clock_t start = clock();
    Move bestmove(INVALID, INVALID);
	EvaluationResult best_er;
	best_er.score = EvaluationEngine::MIN_SCORE;
    double elapsed_time = 0;
    for (int i = 1; i <= max_depth; i++)
    {
        double old_elapsed = elapsed_time;
        vector<Move> variation;
        map<uint64_t, EvaluationResult> hash_table;
        Board b = board;
        vector<Move> legal_moves = MoveGenerator::generate_moves_legal(b);
        int index = 0;
        for (Move m : legal_moves)
        {
            output << "info currmove " << m << " currmovenumber " << ++index << endl;
            EvaluationResult er = EvaluationEngine::evaluate(b, i, variation, {m}, hash_table);
            output << "info depth " << i;
            output << " score cp " << er.score;
            elapsed_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;
            int nodes = hash_table.size();
            int nps = round(nodes * 1000.0 / (elapsed_time - old_elapsed));
            output << " time " << (int)elapsed_time;
            output << " nodes " << nodes;
            output << " nps " << nps;
            output << " pv";
            for (Move &m : er.variation)
            {
                output << " " << m;
            }
            output << endl;
            if (er.score > best_er.score)
            {
                best_er = er;
                bestmove = er.variation[0];
            }
            if (elapsed_time > max_time || !thinking.load())
            {
                output << "bestmove " << bestmove << endl;
                return;
            }
        }
    }
    output << "bestmove " << bestmove << endl;
}