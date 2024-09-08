#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

/**
 * - for both absolute and relative location
 */
class ProblemLocation {
public:
	virtual ~ProblemLocation() {};
};

class Problem {
public:
	virtual ~Problem() {};

	virtual std::vector<double> get_observations() = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result(int num_analyze,
								int num_actions) = 0;

	virtual ProblemLocation* get_absolute_location() = 0;
	virtual ProblemLocation* get_relative_location(ProblemLocation* comparison) = 0;
	virtual void return_to_location(ProblemLocation* comparison,
									ProblemLocation* relative) = 0;

	virtual Problem* copy_and_reset() = 0;
	virtual Problem* copy_snapshot() = 0;

	virtual void print() = 0;
};

class ProblemType {
public:
	virtual ~ProblemType() {};

	virtual Problem* get_problem() = 0;

	virtual int num_obs() = 0;
	virtual int num_possible_actions() = 0;
	virtual Action random_action() = 0;

	virtual void save_location(ProblemLocation* location,
							   std::ofstream& output_file) = 0;
	virtual ProblemLocation* load_location(std::ifstream& input_file) = 0;
	virtual ProblemLocation* deep_copy_location(ProblemLocation* original) = 0;
};

#endif /* PROBLEM_H */