#ifndef RE_EVAL_STEP_HISTORY_H
#define RE_EVAL_STEP_HISTORY_H

#include "solution_node.h"

class SolutionNode;
class ReEvalStepHistory {
public:
	SolutionNode* node_visited;

	double guess;

	int scope_state;

	ReEvalStepHistory(SolutionNode* node_visited,
					  double guess,
					  int scope_state) {
		this->node_visited = node_visited;
		this->guess = guess;
		this->scope_state = scope_state;
	}
	// no user-declared destructor for implicit move constructor(?)
};

#endif /* RE_EVAL_STEP_HISTORY_H */