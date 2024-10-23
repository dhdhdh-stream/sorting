#ifndef CONDITION_NODE_H
#define CONDITION_NODE_H

// - not strong enough to break out of bad merges?

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <utility>
#include <vector>

class Problem;
class ScopeHistory;

class ConditionNodeHistory;
class ConditionNode : public AbstractNode {
public:
	/**
	 * TODO:
	 * - consider conditioning on obs as well
	 *   - e.g., compare if obs greater than a particular val
	 *     - e.g., initially a standard deviation above average
	 */
	std::vector<std::pair<std::pair<std::vector<int>,std::vector<int>>, bool>> conditions;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	ConditionNode();
	ConditionNode(ConditionNode* original);
	~ConditionNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void new_scope_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* scope_history);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ConditionNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	ConditionNodeHistory(ConditionNode* node);
	ConditionNodeHistory(ConditionNodeHistory* original);
};

#endif /* CONDITION_NODE_H */