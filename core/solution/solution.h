#ifndef SOLVER_H
#define SOLVER_H

#include <mutex>

#include "solution_node.h"

class SolutionNode;
class Solver {
public:
	Explore* explore;

	std::vector<SolutionNode*> nodes;
	std::mutex nodes_mtx;
	
	int current_state_counter;
	std::mutex state_mtx;

	int current_potential_state_counter;

	std::mutex display_mtx;

	Solver(Explore* explore);
	~Solver();

	void iteration();

	void save();
};

#endif /* SOLVER_H */