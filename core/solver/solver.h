#ifndef SOLVER_H
#define SOLVER_H

#include <random>

#include "action.h"
#include "problem.h"
#include "solution_node.h"

class Solver {
public:
	std::vector<SolutionNode*> nodes;
	int current_node_index;

	std::default_random_engine generator;

	Solver();
	~Solver();

	void run();

private:
	void simple_pass();
};

#endif /* SOLVER_H */