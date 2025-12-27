/**
 * - obs:
 *   - location
 *   - target
 *   - chase
 *   - [10 noise]
 */

#ifndef TUNNEL_PRACTICE_H
#define TUNNEL_PRACTICE_H

#include <vector>

#include "problem.h"

const int TUNNEL_PRACTICE_ACTION_LEFT = 0;
const int TUNNEL_PRACTICE_ACTION_RIGHT = 1;
const int TUNNEL_PRACTICE_ACTION_CLICK = 2;

class TunnelPractice : public Problem {
public:
	int curr_index;

	std::vector<int> targets;
	int curr_target_index;

	double score;

	std::vector<double> noise;

	TunnelPractice();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
};

class TypeTunnelPractice : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* TUNNEL_PRACTICE_H */