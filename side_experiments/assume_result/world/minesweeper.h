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
	void perform_action(Action action);
	double score_result(int num_analyze,
						int num_actions);

	ProblemLocation* get_absolute_location();
	ProblemLocation* get_relative_location(ProblemLocation* comparison);
	void return_to_location(ProblemLocation* comparison,
							ProblemLocation* relative);

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();

	double get_observation_helper(int x, int y);
	void reveal_helper(int x, int y);
};

class TypeMinesweeper : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();

	void save_location(ProblemLocation* location,
					   std::ofstream& output_file);
	ProblemLocation* load_location(std::ifstream& input_file);
	ProblemLocation* deep_copy_location(ProblemLocation* original);
};

class MinesweeperLocation : public ProblemLocation {
public:
	int loc_x;
	int loc_y;
};

#endif /* MINESWEEPER_H */