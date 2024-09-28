#ifndef PROBLEM_H
#define PROBLEM_H

#include "action.h"

class Problem {
public:
	virtual ~Problem() {};

	virtual void get_observations(std::vector<double>& obs,
								  std::vector<std::vector<int>>& locations) = 0;
	virtual void perform_action(Action action) = 0;
	virtual double score_result(double time_spent) = 0;

	virtual std::vector<int> get_location() = 0;
	virtual void return_to_location(std::vector<int>& location) = 0;

	virtual Problem* copy_and_reset() = 0;
	virtual Problem* copy_snapshot() = 0;

	virtual void print() = 0;
	virtual void print_obs() = 0;
};

class ProblemType {
public:
	virtual ~ProblemType() {};

	virtual Problem* get_problem() = 0;

	virtual int num_obs() = 0;
	virtual int num_possible_actions() = 0;
	virtual Action random_action() = 0;

	virtual int num_dimensions() = 0;
	virtual std::vector<int> relative_to_world(std::vector<int>& comparison,
											   std::vector<int>& relative_location) = 0;
	virtual std::vector<int> world_to_relative(std::vector<int>& comparison,
											   std::vector<int>& world_location) = 0;
};

#endif /* PROBLEM_H */