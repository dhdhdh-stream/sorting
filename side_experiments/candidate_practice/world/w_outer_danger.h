/**
 * - obs:
 *   - location
 *   - target
 *   - chase
 *   - [10 noise]
 */

#ifndef W_OUTER_DANGER_H
#define W_OUTER_DANGER_H

#include <vector>

#include "problem.h"

const int W_OUTER_DANGER_ACTION_LEFT = 0;
const int W_OUTER_DANGER_ACTION_RIGHT = 1;
const int W_OUTER_DANGER_ACTION_CLICK = 2;

class WOuterDanger : public Problem {
public:
	int curr_index;

	std::vector<int> targets;
	int curr_target_index;

	double score;

	double danger;

	std::vector<double> noise;

	WOuterDanger();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
};

class TypeWOuterDanger : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* W_OUTER_DANGER_H */