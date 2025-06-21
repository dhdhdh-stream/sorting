/**
 * target fixed  target
 * fixed  signal fixed
 * target fixed  target
 */

#ifndef SIMPLE_H
#define SIMPLE_H

#include <vector>

#include "problem.h"

class Simple : public Problem {
public:
	std::vector<std::vector<int>> world;
	int current_x;
	int current_y;

	std::vector<int> targets;
	int curr_index;

	double random_factor;

	Simple();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	#if defined(MDEBUG) && MDEBUG
	Problem* copy_and_reset();
	Problem* copy_snapshot();
	#endif /* MDEBUG */

	void print();
};

class TypeSimple : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLE_H */