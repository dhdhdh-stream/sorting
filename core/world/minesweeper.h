// TODO: try transfer learning

// TODO: actually, if world modeling, don't have to execute full actions, but can abstract
// - another way in which world modeling can make exploring much faster

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <vector>

#include "problem.h"

const int MINESWEEPER_ACTION_UP = 0;
const int MINESWEEPER_ACTION_RIGHT = 1;
const int MINESWEEPER_ACTION_DOWN = 2;
const int MINESWEEPER_ACTION_LEFT = 3;
const int MINESWEEPER_ACTION_CLICK = 4;

class Minesweeper : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	int current_x;
	int current_y;

	int num_revealed;

	bool ended;

	Minesweeper();

	Action random_action();

	double get_observation();
	void perform_action(Action action);
	double score_result(int num_actions);

	Problem* copy_and_reset();

	void print();
};

#endif /* MINESWEEPER_H */