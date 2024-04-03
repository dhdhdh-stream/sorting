#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "action.h"
#include "context_layer.h"
#include "run_helper.h"

class Problem;
class Scope;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	Action action;

	int next_node_id;
	AbstractNode* next_node;

	std::vector<int> hook_indexes;
	std::vector<std::vector<Scope*>> hook_scope_contexts;
	std::vector<std::vector<AbstractNode*>> hook_node_contexts;

	ActionNode();
	~ActionNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);

	void back_activate(std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   std::vector<double>& input_vals,
					   ActionNodeHistory* history);

	void step_through_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   ActionNodeHistory* history);

	void reset();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);
};

#endif /* ACTION_NODE_H */