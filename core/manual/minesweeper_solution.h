#ifndef MINESWEEPER_SOLUTION_H
#define MINESWEEPER_SOLUTION_H

#include "manual_solution.h"

const int MINESWEEPER_SOLUTION_STATE_TO_BOTTOM = 0;
const int MINESWEEPER_SOLUTION_STATE_TO_RIGHT = 1;
const int MINESWEEPER_SOLUTION_STATE_TRAVERSE_LEFT = 2;
const int MINESWEEPER_SOLUTION_STATE_TRAVERSE_RIGHT = 3;

class MinesweeperSolution : public ManualSolution {
public:
	int state;

	bool iter_made_progress;

	MinesweeperSolution();
	~MinesweeperSolution();

	void step(std::vector<double>& observations,
			  bool& done,
			  std::vector<Action>& actions);
};

#endif /* MINESWEEPER_SOLUTION_H */