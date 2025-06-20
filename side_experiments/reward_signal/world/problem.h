#ifndef PROBLEM_H
#define PROBLEM_H

class Problem {
public:
	virtual ~Problem() {};

	virtual std::vector<double> get_observations() = 0;
	virtual void perform_action(int action) = 0;
	virtual double score_result() = 0;

	#if defined(MDEBUG) && MDEBUG
	virtual Problem* copy_and_reset() = 0;
	virtual Problem* copy_snapshot() = 0;
	#endif /* MDEBUG */

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