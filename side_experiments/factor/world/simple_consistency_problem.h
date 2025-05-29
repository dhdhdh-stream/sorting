// TODO: or want problem where there's reason to stay on track, and reason to stay off track

#ifndef SIMPLE_CONSISTENCY_PROBLEM_H
#define SIMPLE_CONSISTENCY_PROBLEM_H

#include <vector>

#include "problem.h"

class SimpleConsistencyProblem : public Problem {
public:
	std::vector<std::vector<int>> world;
	int current_x;
	int current_y;

	std::vector<int> targets;
	int curr_target_index;

	double score;

	SimpleConsistencyProblem();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	#if defined(MDEBUG) && MDEBUG
	Problem* copy_and_reset();
	Problem* copy_snapshot();
	#endif /* MDEBUG */

	void print();
};

class TypeSimpleConsistencyProblem : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

#endif /* SIMPLE_CONSISTENCY_PROBLEM_H */