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
};

class TypeSorting : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

class SortingLocation : public ProblemLocation {
public:
	int loc;
};

#endif /* SORTING_H */