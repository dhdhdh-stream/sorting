#ifndef ENDING_SCOPE_NODE_HELPER_H
#define ENDING_SCOPE_NODE_HELPER_H

#include "context_layer.h"
#include "run_helper.h"

class ScopeNode;

class EndingScopeNodeHelper {
public:
	ScopeNode* scope_node;

	bool is_halfway;
	int furthest_matching_layer;

	std::vector<std::vector<double>> inner_state_vals;
	std::vector<std::vector<double>*> starting_state_vals_save;

	EndingScopeNodeHelper(ScopeNode* scope_node);

	void forward(std::vector<int>& next_starting_node_ids,
				 std::vector<std::vector<double>*>& next_starting_state_vals,
				 std::vector<std::vector<double>>& next_starting_state_weights,
				 std::vector<ContextLayer>& context,
				 RunHelper& run_helper);
	void backward(std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  SequenceHistory* history);
};

#endif /* ENDING_SCOPE_NODE_HELPER_H */