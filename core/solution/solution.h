#ifndef SOLVER_H
#define SOLVER_H

#include <mutex>

#include "action.h"
#include "problem.h"
#include "solution_node.h"

class SolutionNode;
class Solver {
public:
	std::vector<SolutionNode*> nodes;

	int current_node_counter;
	int current_state_counter;

	int current_potential_state_counter;

	std::mutex nodes_mtx;
	std::mutex display_mtx;

	Solver();
	~Solver();

	void add_nodes(SolutionNode* starting_point,
				   std::vector<Action> candidate);

	void tune();

	void save();
};

#endif /* SOLVER_H */