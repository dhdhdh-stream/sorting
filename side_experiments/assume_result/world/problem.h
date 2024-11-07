#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	virtual std::vector<double> get_observations() = 0;
	virtual void perform_action(Action action) = 0;
	/**
	 * TODO: use actual time spent?
	 */
	virtual double score_result() = 0;

	virtual std::vector<double> get_location() = 0;
	virtual void return_to_location(std::vector<double>& location) = 0;

	virtual Problem* copy_and_reset() = 0;

	virtual void print() = 0;
};

class ProblemType {
public:
	virtual ~ProblemType() {};

	virtual Problem* get_problem() = 0;

	virtual int num_obs() = 0;
	virtual int num_possible_actions() = 0;
	virtual Action random_action() = 0;

	virtual int num_dimensions() = 0;
	virtual std::vector<double> relative_to_world(std::vector<double>& comparison,
												  std::vector<double>& relative_location) = 0;
	virtual std::vector<double> world_to_relative(std::vector<double>& comparison,
												  std::vector<double>& world_location) = 0;
};

#endif /* PROBLEM_H */