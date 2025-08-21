/**
 * target fixed signal fixed target
 */

#ifndef SIMPLEST_H
#define SIMPLEST_H

#include <vector>

#include "problem.h"

class Simplest : public Problem {
public:
	std::vector<double> world;
	int curr_index;

	double random_factor;

	Simplest();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	void print();
};

class TypeSimplest : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLEST_H */