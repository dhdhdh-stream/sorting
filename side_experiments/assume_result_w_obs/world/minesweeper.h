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

	void get_observations(std::vector<double>& obs,
						  std::vector<std::vector<double>>& locations);
	void perform_action(Action action);
	double score_result(int num_analyze,
						int num_actions);

	std::vector<double> get_location();
	void return_to_location(std::vector<double>& location);

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

	std::vector<double> relative_to_world(std::vector<double>& comparison,
										  std::vector<double>& relative_location);
	std::vector<double> world_to_relative(std::vector<double>& comparison,
										  std::vector<double>& world_location);
};

#endif /* MINESWEEPER_H */