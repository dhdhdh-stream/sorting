#ifndef SIMPLE_MINESWEEPER_H
#define SIMPLE_MINESWEEPER_H

#include <vector>

#include "problem.h"

const int SIMPLE_MINESWEEPER_ACTION_UP = 0;
const int SIMPLE_MINESWEEPER_ACTION_RIGHT = 1;
const int SIMPLE_MINESWEEPER_ACTION_DOWN = 2;
const int SIMPLE_MINESWEEPER_ACTION_LEFT = 3;
const int SIMPLE_MINESWEEPER_ACTION_EVAL = 4;

class SimpleMinesweeper : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> revealed;
	std::vector<std::vector<bool>> flagged;
	int current_x;
	int current_y;

	int starting_revealed;

	bool hit_mine;

	int num_actions;

	SimpleMinesweeper();

	int num_obs();
	int num_possible_actions();
	Action random_action();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	Problem* copy_and_reset();

	void print();

private:
	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
};

#endif /* SIMPLE_MINESWEEPER_H */