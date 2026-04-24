#ifndef SIMPLE_H
#define SIMPLE_H

#include <vector>

#include "problem.h"

const int SIMPLE_ACTION_UP = 0;
const int SIMPLE_ACTION_RIGHT = 1;
const int SIMPLE_ACTION_DOWN = 2;
const int SIMPLE_ACTION_LEFT = 3;

class Simple : public Problem {
public:
	std::vector<std::vector<int>> world;
	std::vector<std::vector<bool>> hit;
	int current_x;
	int current_y;

	bool hit_mine;

	Simple();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();

	double get_observation_helper(int x, int y);
};

class TypeSimple : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLE_H */