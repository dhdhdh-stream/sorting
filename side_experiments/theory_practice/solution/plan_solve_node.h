#ifndef PLAN_SOLVE_NODE_H
#define PLAN_SOLVE_NODE_H

#include "plan_node.h"

class PlanSolveNode : public PlanNode {
public:
	Theory* theory;

	Pattern* pattern;


};

#endif /* PLAN_SOLVE_NODE_H */