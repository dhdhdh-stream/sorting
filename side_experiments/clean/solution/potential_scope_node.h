#ifndef POTENTIAL_SCOPE_NODE_H
#define POTENTIAL_SCOPE_NODE_H

#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class Scope;
class ScopeNode;

class PotentialScopeNodeHistory;
class PotentialScopeNode {
public:
	std::vector<int> input_types;
	/**
	 * - inner always local
	 */
	std::vector<int> input_inner_indexes;
	/**
	 * - negative indexing
	 */
	std::vector<int> input_scope_depths;
	std::vector<bool> input_outer_is_local;
	std::vector<int> input_outer_indexes;
	std::vector<double> input_init_vals;
	std::vector<bool> input_used;

	/**
	 * - inner always locale
	 */
	std::vector<int> output_inner_indexes;
	std::vector<int> output_scope_depths;
	std::vector<bool> output_outer_is_local;
	std::vector<int> output_outer_indexes;
	std::vector<bool> output_used;

	int experiment_scope_depth;
	#if defined(MDEBUG) && MDEBUG
	std::set<State*> used_experiment_states;
	#endif /* MDEBUG */

	Scope* scope;
	std::vector<bool> used_states;

	ScopeNode* scope_node_placeholder;

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