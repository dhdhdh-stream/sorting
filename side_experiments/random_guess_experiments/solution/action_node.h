#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <vector>

#include "abstract_node.h"
#include "constants.h"

class ActionNode : public AbstractNode {
public:
	int action;

	std::vector<int> state_indexes;
	std::vector<bool> state_xor_switch;

	ActionNode(int action,
			   std::vector<int> state_indexes,
			   std::vector<bool> state_xor_switch);
	ActionNode(ActionNode* original);
	~ActionNode();

	void activate(int& curr_spot,
				  int& curr_0_index,
				  std::vector<int>& spots,
				  std::vector<bool>& switches,
				  int& num_actions);
	void fetch_context(std::vector<Scope*>& scope_context,
					   std::vector<int>& node_context,
					   int& curr_num_action,
					   int target_num_action);
};

#endif /* ACTION_NODE_H */