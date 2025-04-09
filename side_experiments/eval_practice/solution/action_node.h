#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include "abstract_node.h"
#include "action.h"

class Problem;

class ActionNode : public AbstractNode {
public:
	Action action;

	ActionNode();

	void new_activate(Problem* problem);
};

#endif /* ACTION_NODE_H */