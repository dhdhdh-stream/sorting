/**
 * - 20 epochs
 *   - first 10 iters, try to hit target
 *   - next 5 iters, signal whether success
 * 
 * - after 20 epochs, always give final score
 */

#ifndef TIME_BASED_H
#define TIME_BASED_H

#include <vector>

#include "problem.h"

class TimeBased : public Problem {
public:
	int epoch;
	int iter;

	int curr_index;
	int curr_target;
	bool curr_clicked;
	int curr_success;

	double final_score;

	TimeBased();

	std::vector<double> get_observations();
	void perform_action(int action);
	double score_result();

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
};

class TypeTimeBased : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* TIME_BASED_H */