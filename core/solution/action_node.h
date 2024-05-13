#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
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

	ActionNode();
	ActionNode(ActionNode* original);
	~ActionNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void new_action_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void step_through_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   RunHelper& run_helper,
							   ActionNodeHistory* history);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_snapshot;

	ActionNodeHistory();
	ActionNodeHistory(ActionNodeHistory* original);
};

#endif /* ACTION_NODE_H */