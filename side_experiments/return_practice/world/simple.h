#ifndef SIMPLE_H
#define SIMPLE_H

#include <vector>

#include "problem.h"

class Simple : public Problem {
public:
	int current_index;

	Simple();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();
};

class TypeSimple : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLE_H */