/**
 * - - -
 * - - -
 * O - -
 * X - X
 * - S -
 * X - X
 * - - -
 * - - O
 * - - -
 */

#ifndef MULTI_LEVEL_H
#define MULTI_LEVEL_H

#include "problem.h"

const int MULTI_LEVEL_ACTION_UP = 0;
const int MULTI_LEVEL_ACTION_RIGHT = 1;
const int MULTI_LEVEL_ACTION_DOWN = 2;
const int MULTI_LEVEL_ACTION_LEFT = 3;
const int MULTI_LEVEL_ACTION_CLICK = 4;

class MultiLevel : public Problem {
public:
	int current_x;
	int current_y;

	bool hit_top;
	bool hit_bottom;

	bool is_fail;

	MultiLevel();

	void perform_action(int action);
	double score_result();
};

class TypeMultiLevel : public ProblemType {
public:
	Problem* get_problem();

	int num_possible_actions();
};

#endif /* MULTI_LEVEL_H */