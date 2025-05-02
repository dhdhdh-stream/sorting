#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	virtual double get_observation() = 0;
	virtual void perform_action(Action action) = 0;

	virtual void print() = 0;
};

class ProblemType {
public:
	virtual ~ProblemType() {};

	virtual Problem* get_problem() = 0;

	virtual int num_obs() = 0;
	virtual int num_possible_actions() = 0;
};

#endif /* PROBLEM_H */