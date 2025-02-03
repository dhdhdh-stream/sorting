#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"
#include "action.h"
#include "run_helper.h"

class Problem;
class ScopeHistory;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	Action action;

	int next_node_id;
	AbstractNode* next_node;

	ActionNode();
	~ActionNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	ActionNodeHistory(ActionNode* node);
};

#endif /* ACTION_NODE_H */