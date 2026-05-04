#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>

class Problem {
public:
	virtual ~Problem() {};

	virtual void perform_action(int action) = 0;
	virtual double score_result() = 0;
};

class ProblemType {
public:
	virtual ~ProblemType() {};

	virtual Problem* get_problem() = 0;

	virtual int num_possible_actions() = 0;
};

#endif /* PROBLEM_H */