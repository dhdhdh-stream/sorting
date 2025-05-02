#ifndef TWO_DIMENSIONAL_H
#define TWO_DIMENSIONAL_H

#include <vector>

#include "problem.h"

const int TWO_DIMENSIONAL_ACTION_UP = 0;
const int TWO_DIMENSIONAL_ACTION_RIGHT = 1;
const int TWO_DIMENSIONAL_ACTION_DOWN = 2;
const int TWO_DIMENSIONAL_ACTION_LEFT = 3;

class TwoDimensional : public Problem {
public:
	std::vector<std::vector<double>> world;
	int x_index;
	int y_index;

	TwoDimensional();

	double get_observation();
	void perform_action(Action action);

	void print();
};

class TypeTwoDimensional : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
};

#endif /* TWO_DIMENSIONAL_H */