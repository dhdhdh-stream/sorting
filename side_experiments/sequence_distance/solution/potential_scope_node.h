#ifndef POTENTIAL_SCOPE_NODE_H
#define POTENTIAL_SCOPE_NODE_H

#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class AbstractNode;
class Scope;
class ScopeNode;

const int OUTER_TYPE_INPUT = 0;
const int OUTER_TYPE_LOCAL = 1;
const int OUTER_TYPE_TEMP = 2;

class PotentialScopeNodeHistory;
class PotentialScopeNode {
public:
	std::vector<int> input_types;
	std::vector<int> input_inner_indexes;
	/**
	 * - negative indexing
	 */
	std::vector<int> input_scope_depths;
	std::vector<int> input_outer_types;
	std::vector<void*> input_outer_indexes;
	std::vector<double> input_init_vals;
	std::vector<double> input_init_index_vals;

	std::vector<int> output_inner_indexes;
	std::vector<int> output_scope_depths;
	std::vector<int> output_outer_types;
	std::vector<void*> output_outer_indexes;

	int experiment_scope_depth;
	std::set<State*> used_experiment_states;

	Scope* scope;

	ScopeNode* scope_node_placeholder;

	std::vector<AbstractNode*> original_nodes;
	std::vector<int> starting_original_scope_context;
	std::vector<int> starting_original_node_context;
	std::vector<int> ending_original_scope_context;
	std::vector<int> ending_original_node_context;
	/**
	 * - ((is_starting, layer), (is_local, index))
	 */
	std::vector<std::pair<std::pair<bool,int>, std::pair<bool,int>>> original_states;

	PotentialScopeNode();
	~PotentialScopeNode();

	void activate(Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  PotentialScopeNodeHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper);
	#endif /* MDEBUG */
};

class PotentialScopeNodeHistory {
public:
	PotentialScopeNode* potential_scope_node;

	ScopeHistory* scope_history;

	PotentialScopeNodeHistory(PotentialScopeNode* potential_scope_node);
	PotentialScopeNodeHistory(PotentialScopeNodeHistory* original);
	~PotentialScopeNodeHistory();
};

#endif /* POTENTIAL_SCOPE_NODE_H */