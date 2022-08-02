#ifndef SOLVER_H
#define SOLVER_H

#include "action.h"
#include "problem.h"
#include "solution_node.h"

class SolutionNode;
class Solver {
public:
	std::vector<SolutionNode*> nodes;
	int current_node_index;

	Solver();
	~Solver();

	void add_nodes(SolutionNode* starting_point,
				   std::vector<Action> candidate);

	void single_pass(bool save_for_display);
};

#endif /* SOLVER_H */