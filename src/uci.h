#ifndef UCI_H
#define UCI_H

#include <iostream>

#include "board.h"

class UCI
{
  public:
    UCI(istream &input, ostream &output) : input(input), output(output) {}
    const istream& get_input() const { return input; }
    const ostream& get_output() const { return output; }
    Move output_thinking(int max_depth, int max_time);
    void main_loop();

  private:
    istream &input;
    ostream &output;
    Board board;
};

#endif