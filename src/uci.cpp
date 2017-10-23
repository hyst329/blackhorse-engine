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
					depth = 30;
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
				stop_thinking.store(false);
				thread t([this, depth, max_time]() { return output_thinking(depth, max_time); });
				t.detach();
			}
			if (command == "stop")
			{
				// TODO: Stop thinking
				stop_thinking.store(true);
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
	clock_t scheduled = start + (clock_t)(max_time / 1000.0 * CLOCKS_PER_SEC);
	Move bestmove(INVALID, INVALID);
	int16_t best_er = EvaluationEngine::MIN_SCORE;
	double elapsed_time = 0;
	uint64_t all_nodes = 0;
	for (int i = 1; i <= max_depth; i++)
	{
		vector<Move> variation;
		map<uint64_t, pair<int16_t, int>> hash_table;
		map<uint64_t, Move> hash_var;
		int16_t er = EvaluationEngine::evaluate(board, i, true, output, hash_table, hash_var, scheduled);
		if (i > 1 && clock() > scheduled)
		{
			break;
		}
		Board temp = board;
		while (hash_var.count(temp.get_hash()))
		{
			Move m = hash_var[temp.get_hash()];
			temp.make_move(m);
			variation.push_back(m);
		}
		output << "info depth " << i;
		int sel_depth = variation.size();
		if (sel_depth > i)
		{
			output << " seldepth " << sel_depth;
		}
		if (er == EvaluationEngine::MIN_SCORE)
		{
			output << " score mate -" << (i / 2);
		}
		else if (er == EvaluationEngine::MAX_SCORE)
		{
			output << " score mate " << ((i + 1) / 2);
		}
		else
		{
			output << " score cp " << er;
		}
		elapsed_time = (double)(clock() - start) / CLOCKS_PER_SEC * 1000;
		int nodes = hash_table.size();
		int nps = round((all_nodes + nodes) * 1000.0 / elapsed_time);
		output << " time " << (int)elapsed_time;
		output << " nodes " << all_nodes + nodes;
		output << " nps " << nps;
		output << " pv";
		for (Move &m : variation)
		{
			output << " " << m;
		}
		output << endl;
		//if (er > best_er)
		//{
			best_er = er;
			bestmove = variation[0];
		//}
		if (elapsed_time > max_time || stop_thinking.load())
		{
			output << "bestmove " << bestmove << endl;
			return;
		}
		all_nodes += hash_table.size();
	}
	output << "bestmove " << bestmove << endl;
}