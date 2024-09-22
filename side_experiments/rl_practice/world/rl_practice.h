/**
 * - to score, need to perform right action and be at end when terminate
 */

#ifndef RL_PRACTICE_H
#define RL_PRACTICE_H

#include <vector>

#include "problem.h"

const int RL_PRACTICE_MOVE_LEFT = 0;
const int RL_PRACTICE_MOVE_MIDDLE = 1;
const int RL_PRACTICE_MOVE_RIGHT = 2;
const int RL_PRACTICE_PERFORM_LEFT = 3;
const int RL_PRACTICE_PERFORM_RIGHT = 4;

class RLPractice : public Problem {
public:
	int open_door;
	bool at_end;
	bool left_needed;

	bool action_performed;
	bool past_end;

	RLPractice();

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

class TypeRLPractice : public ProblemType {
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

#endif /* RL_PRACTICE_H */