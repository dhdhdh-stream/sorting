#ifndef SIMPLE_H
#define SIMPLE_H

#include <vector>

#include "problem.h"

const int SIMPLE_ACTION_LEFT = 0;
const int SIMPLE_ACTION_RIGHT = 1;
const int SIMPLE_ACTION_SWAP = 2;

class Simple : public Problem {
public:
	std::vector<double> initial_world;
	
	std::vector<double> current_world;
	int current_pointer;

	Simple();

	int num_actions();
	Action random_action();

	double get_observation();
	void perform_action(Action action);
	double score_result(int num_process);

	Problem* copy_and_reset();

	void print();
};

#endif /* SIMPLE_H */