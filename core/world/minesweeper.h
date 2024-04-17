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
const int MINESWEEPER_ACTION_DOUBLECLICK = 6;

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

	int num_obs();
	int num_possible_actions();
	Action random_action();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	Problem* copy_and_reset();

	void print();

	int current_direction;
	bool travel();

private:
	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
};

#endif /* MINESWEEPER_H */