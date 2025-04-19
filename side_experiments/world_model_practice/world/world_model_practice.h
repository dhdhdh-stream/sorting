#ifndef WORLD_MODEL_PRACTICE_H
#define WORLD_MODEL_PRACTICE_H

#include <vector>

#include "problem.h"

const int WORLD_MODEL_ACTION_LEFT = 0;
const int WORLD_MODEL_ACTION_RIGHT = 1;
const int WORLD_MODEL_ACTION_ACTION = 2;

class WorldModelPractice : public Problem {
public:
	std::vector<int> world;
	int current_index;

	bool performed_action;

	WorldModelPractice();

	std::vector<double> get_observations();
	void perform_action(Action action);

	void print();
};

class TypeWorldModelPractice : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* WORLD_MODEL_PRACTICE_H */