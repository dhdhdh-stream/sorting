/**
 * - move right once, move forward twice, and click
 * 
 *   X X X O X
 *   X X X - X
 *   X X X - X
 *   - - S - -
 *   - - - - -
 */

#ifndef SIMPLE_H
#define SIMPLE_H

#include <vector>

#include "problem.h"

const int ACTION_UP = 0;
const int ACTION_RIGHT = 1;
const int ACTION_DOWN = 2;
const int ACTION_LEFT = 3;
const int ACTION_CLICK = 4;

class Simple : public Problem {
public:
	int current_x;
	int current_y;

	bool hit_target;

	bool is_fail;

	Simple();

	void perform_action(int action);
	double score_result();
};

class TypeSimple : public ProblemType {
public:
	Problem* get_problem();

	int num_possible_actions();
};

#endif /* SIMPLE_H */
