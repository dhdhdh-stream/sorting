#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	virtual std::vector<double> get_observations() = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result() = 0;

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