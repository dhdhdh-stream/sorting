/**
 * - obs:
 *   - location
 *   - target
 *   - last success
 *   - epoch
 */

#ifndef INSTANCE_SCORES_H
#define INSTANCE_SCORES_H

#include <vector>

#include "problem.h"

class InstanceScores : public Problem {
public:
	int epoch;

	int curr_index;
	int curr_target;
	int last_success;

	double final_score;

	InstanceScores();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	void print();
};

class TypeInstanceScores : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* INSTANCE_SCORES_H */