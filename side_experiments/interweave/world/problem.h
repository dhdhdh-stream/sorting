#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	virtual std::vector<double> get_observations() = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result() = 0;
	/**
	 * - simply give entire score signal at end
	 * 
	 * - don't discount future signals
	 *   - can lead to pursuing short term gains at the cost of future ones
	 */

	virtual void print() = 0;
};

class ProblemType {
public:
	virtual ~ProblemType() {};

	virtual Problem* get_problem() = 0;

	virtual int num_obs() = 0;
	virtual int num_possible_actions() = 0;
	virtual Action random_action() = 0;
};

#endif /* PROBLEM_H */