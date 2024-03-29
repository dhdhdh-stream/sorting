#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <vector>

#include "problem.h"

const int MINESWEEPER_ACTION_UP = 0;
const int MINESWEEPER_ACTION_RIGHT = 1;
const int MINESWEEPER_ACTION_DOWN = 2;
const int MINESWEEPER_ACTION_LEFT = 3;
const int MINESWEEPER_ACTION_CLICK = 4;
const int MINESWEEPER_ACTION_FLAG = 5;

class Minesweeper : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	int starting_revealed;

	bool hit_mine;

	int num_actions;

	Minesweeper();

	Action random_action();

	double get_observation();
	void perform_action(Action action);
	double score_result();

	Problem* copy_and_reset();

	void print();

private:
	void reveal_helper(int x, int y);
};

#endif /* MINESWEEPER_H */