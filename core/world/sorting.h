#ifndef SORTING_H
#define SORTING_H

#include <vector>

#include "problem.h"

const int SORTING_ACTION_LEFT = 0;
const int SORTING_ACTION_RIGHT = 1;
const int SORTING_ACTION_SWAP = 2;

class Sorting : public Problem {
public:
	std::vector<double> initial_world;
	
	std::vector<double> current_world;
	int current_pointer;

	Sorting();

	int num_obs();
	int num_possible_actions();
	Action random_action();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	Problem* copy_and_reset();

	void print();
};

#endif /* SORTING_H */