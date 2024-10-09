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
class ScopeHistory;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	Action action;

	std::vector<std::vector<int>> input_scope_context_ids;
	std::vector<std::vector<int>> input_node_context_ids;
	/**
	 * - -1 if on/off
	 */
	std::vector<int> input_obs_indexes;

	int next_node_id;
	AbstractNode* next_node;

	ActionNode();
	ActionNode(ActionNode* original);
	~ActionNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void explore_activate(Problem* problem,
						  RunHelper& run_helper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	ActionNodeHistory();
	ActionNodeHistory(ActionNodeHistory* original);
};

#endif /* ACTION_NODE_H */