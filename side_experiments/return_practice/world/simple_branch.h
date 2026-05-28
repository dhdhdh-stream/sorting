#ifndef SIMPLE_BRANCH_H
#define SIMPLE_BRANCH_H

#include <vector>

#include "problem.h"

class SimpleBranch : public Problem {
public:
	bool is_left;

	int current_index;

	SimpleBranch();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();
};

class TypeSimpleBranch : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLE_BRANCH_H */