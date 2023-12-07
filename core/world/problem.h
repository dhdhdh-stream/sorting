#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

const int PROBLEM_TYPE_SORTING = 0;
const int PROBLEM_TYPE_MINESWEEPER = 1;

class Problem {
public:
	int type;

	virtual ~Problem() {};

	virtual Action random_action() = 0;

	virtual double get_observation() = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result(int num_actions) = 0;

	virtual Problem* copy_and_reset() = 0;

	virtual void print() = 0;
};

#endif /* PROBLEM_H */