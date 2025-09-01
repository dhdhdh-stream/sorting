/**
 * target fixed signal fixed target
 */

#ifndef CLEAN_H
#define CLEAN_H

#include <vector>

#include "problem.h"

class Clean : public Problem {
public:
	std::vector<double> world;
	int curr_index;

	std::vector<int> targets;
	int curr_target_index;

	double random_factor;

	Clean();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
};

class TypeClean : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* CLEAN_H */