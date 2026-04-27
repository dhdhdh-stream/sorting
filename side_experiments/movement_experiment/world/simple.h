#ifndef SIMPLE_H
#define SIMPLE_H

#include <vector>

#include "problem.h"

const int SIMPLE_ACTION_LEFT = 0;
const int SIMPLE_ACTION_RIGHT = 1;

class Simple : public Problem {
public:
	std::vector<int> world;
	int current_index;

	Simple();

	std::vector<double> get_observations();
	void perform_action(int action);

	void print();
};

class TypeSimple : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLE_H */