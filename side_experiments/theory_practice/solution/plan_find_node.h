#ifndef PLAN_FIND_NODE_H
#define PLAN_FIND_NODE_H

#include "plan_node.h"

class PlanFindNode : public PlanNode {
public:
	Scope* scope;

	Pattern* pattern;


};

#endif /* PLAN_FIND_NODE_H */