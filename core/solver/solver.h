#ifndef SOLVER_H
#define SOLVER_H

#include <random>

#include "action.h"
#include "problem.h"
#include "solution_tree_node.h"

class Solver {
public:
	SolutionTreeNode* root;

	std::default_random_engine generator;

	Solver();
	~Solver();

	void run();

private:
	void simple_pass();
};

#endif /* SOLVER_H */