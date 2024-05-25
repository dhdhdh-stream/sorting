#ifndef DISTANCE_H
#define DISTANCE_H

#include <vector>

#include "action.h"
#include "problem.h"

const int DISTANCE_ACTION_UP = 0;
const int DISTANCE_ACTION_RIGHT = 1;
const int DISTANCE_ACTION_DOWN = 2;
const int DISTANCE_ACTION_LEFT = 3;

const int DISTANCE_ACTION_SEQUENCE = 4;

class Distance : public Problem {
public:
	int object_start_x;
	int object_start_y;

	int object_end_x;
	int object_end_y;

	int current_x;
	int current_y;

	bool is_start;

	Distance();

	int num_obs();
	int num_possible_actions();
	Action random_action();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result(int num_decisions);

	Problem* copy_snapshot();

	void print();

private:
	double get_observation_helper(int x, int y);
};

#endif /* DISTANCE_H */