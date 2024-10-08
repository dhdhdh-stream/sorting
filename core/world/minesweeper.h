/**
 * - wider field of view makes progress significantly easier
 *   - instead of needing to find:
 *     - good to-and-back information gather
 *     - good action
 *   - just need good action
 * 
 * TODO:
 * - if see entire board, can use convolutional NN to localize
 *   - include through time as well
 */

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

	bool hit_mine;

	Minesweeper();

	std::vector<double> get_observations();
	ProblemLocation* get_location();
	void perform_action(Action action);
	void return_to_location(ProblemLocation* location);
	double score_result(int num_decisions,
						int num_actions);

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
	void print_obs();

private:
	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
	void print_obs_helper(int x, int y);
};

class TypeMinesweeper : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

class MinesweeperLocation : public ProblemLocation {
public:
	int loc_x;
	int loc_y;
};

#endif /* MINESWEEPER_H */