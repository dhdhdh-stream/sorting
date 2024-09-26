#ifndef SIMPLE_RL_PRACTICE_H
#define SIMPLE_RL_PRACTICE_H

#include <vector>

#include "problem.h"

const int SIMPLE_RL_PRACTICE_LEFT = 0;
const int SIMPLE_RL_PRACTICE_MIDDLE = 1;
const int SIMPLE_RL_PRACTICE_RIGHT = 2;

class SimpleRLPractice : public Problem {
public:
	int curr_target;

	int score;

	SimpleRLPractice();

	void get_observations(std::vector<double>& obs,
						  std::vector<std::vector<int>>& locations);
	void perform_action(Action action);
	double score_result(double time_spent);

	std::vector<int> get_location();
	void return_to_location(std::vector<int>& location);

	Problem* copy_and_reset();
	Problem* copy_snapshot();

	void print();
	void print_obs();
};

class TypeSimpleRLPractice : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();

	int num_dimensions();
	std::vector<int> relative_to_world(std::vector<int>& comparison,
									   std::vector<int>& relative_location);
	std::vector<int> world_to_relative(std::vector<int>& comparison,
									   std::vector<int>& world_location);
};

#endif /* SIMPLE_RL_PRACTICE_H */