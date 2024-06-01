#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	virtual int num_obs() = 0;
	virtual int num_possible_actions() = 0;
	virtual Action random_action() = 0;

	virtual std::vector<double> get_observations() = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result(int num_decisions) = 0;

	virtual Problem* copy_and_reset() = 0;
	virtual Problem* copy_snapshot() = 0;

	virtual void print() = 0;
};

#endif /* PROBLEM_H */