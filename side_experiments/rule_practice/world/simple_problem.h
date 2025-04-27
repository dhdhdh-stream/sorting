#ifndef SIMPLE_PROBLEM_H
#define SIMPLE_PROBLEM_H

#include <vector>

#include "problem.h"

const int SIMPLE_ACTION_LEFT = 0;
const int SIMPLE_ACTION_RIGHT = 1;
const int SIMPLE_ACTION_CLICK = 2;

class SimpleProblem : public Problem {
public:
	std::vector<bool> clicked;
	int target_index;
	int current_index;

	SimpleProblem();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	#if defined(MDEBUG) && MDEBUG
	Problem* copy_and_reset();
	Problem* copy_snapshot();
	#endif /* MDEBUG */

	void print();
};

class TypeSimpleProblem : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLE_PROBLEM_H */