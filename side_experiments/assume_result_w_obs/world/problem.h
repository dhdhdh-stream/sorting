#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	/**
	 * - keep track of starting location in Problem if needed
	 */
	virtual void get_observations(std::vector<double>& obs,
								  std::vector<vector<double>>& locations) = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result(int num_analyze,
								int num_actions) = 0;

	virtual vector<double> get_location() = 0;
	virtual void return_to_location(vector<double>& location) = 0;

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

	virtual vector<double> relative_to_world(vector<double>& comparison,
											 vector<double>& relative_location) = 0;
	virtual vector<double> world_to_relative(vector<double>& comparison,
											 vector<double>& world_location) = 0;
};

#endif /* PROBLEM_H */