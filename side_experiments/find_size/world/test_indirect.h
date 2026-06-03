#ifndef TEST_INDIRECT_H
#define TEST_INDIRECT_H

#include <vector>

#include "problem.h"

class TestIndirect : public Problem {
public:
	int curr_context;
	int current_index;

	TestIndirect();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();
};

class TypeTestIndirect : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* TEST_INDIRECT_H */