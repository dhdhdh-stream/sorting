/**
 * target fixed signal fixed target
 */

#ifndef SIMPLER_H
#define SIMPLER_H

#include <vector>

#include "problem.h"

class Simpler : public Problem {
public:
	std::vector<double> world;
	int curr_index;

	std::vector<int> targets;
	int curr_target_index;

	double random_factor;

	Simpler();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	#if defined(MDEBUG) && MDEBUG
	Problem* copy_and_reset();
	Problem* copy_snapshot();
	#endif /* MDEBUG */

	void print();
};

class TypeSimpler : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* SIMPLER_H */