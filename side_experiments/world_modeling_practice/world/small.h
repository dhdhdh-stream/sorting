// TODO: changing world sizes, landmarks

#ifndef SMALL_H
#define SMALL_H

#include <vector>

#include "problem.h"

class Small : public Problem {
public:
	std::vector<double> world;
	int curr_index;

	Small();

	std::vector<double> get_observations();
	void perform_action(int action);

	void print();
};

class TypeSmall : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SMALL_H */