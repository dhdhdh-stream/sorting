/**
 * - O -
 * O X O
 * - O -
 */

#ifndef EVAL_TEST_H
#define EVAL_TEST_H

#include <vector>

#include "problem.h"

const int EVAL_TEST_ACTION_UP = 0;
const int EVAL_TEST_ACTION_RIGHT = 1;
const int EVAL_TEST_ACTION_DOWN = 2;
const int EVAL_TEST_ACTION_LEFT = 3;

class EvalTest : public Problem {
public:
	std::vector<std::vector<int>> world;
	int current_x;
	int current_y;

	EvalTest();

	std::vector<double> get_observations();
	void perform_action(Action action);
	double score_result();

	void print();
};

class TypeEvalTest : public ProblemType {
public:
	Problem* get_problem();

	int num_obs();
	int num_possible_actions();
	Action random_action();
};

#endif /* EVAL_TEST_H */