#include "signal_helpers.h"

#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "signal.h"

using namespace std;

void create_input_helper(Scope* scope,
						 vector<Scope*>& scope_context,
						 vector<int>& node_context,
						 int& node_count,
						 SignalInput& input) {
	scope_context.push_back(scope);

	uniform_int_distribution<int> inner_distribution(0, 1);
	uniform_int_distribution<int> pre_distribution(0, 1);
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_SCOPE:
			if (inner_distribution(generator) == 0) {
				ScopeNode* scope_node = (ScopeNode*)it->second;

				node_context.push_back(it->first);

				create_input_helper(scope_node->scope,
									scope_context,
									node_context,
									node_count,
									input);

				node_context.pop_back();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				uniform_int_distribution<int> distribution(0, node_count);
				if (distribution(generator) == 0) {
					node_context.push_back(it->first);

					input.is_pre = pre_distribution(generator) == 0;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.obs_index = -1;

					node_context.pop_back();
				}
				node_count++;
			}
			break;
		case NODE_TYPE_OBS:
			{
				uniform_int_distribution<int> distribution(0, node_count);
				if (distribution(generator) == 0) {
					node_context.push_back(it->first);

					input.is_pre = pre_distribution(generator) == 0;
					input.scope_context = scope_context;
					input.node_context = node_context;
					input.obs_index = 0;
					/**
					 * TODO: obs_size
					 */

					node_context.pop_back();
				}
				node_count++;
			}
			break;
		}
	}

	scope_context.pop_back();
}

void set_potential_inputs(Scope* scope) {
	for (int t_index = 0; t_index < POTENTIAL_NUM_TRIES; t_index++) {
		vector<SignalInput> inputs;
		for (int i_index = 0; i_index < SIGNAL_NODE_MAX_NUM_INPUTS; i_index++) {
			vector<Scope*> scope_context;
			vector<int> node_context;
			int node_count = 0;
			SignalInput input;
			create_input_helper(scope,
								scope_context,
								node_context,
								node_count,
								input);

			bool is_match = false;
			for (int ii_index = 0; ii_index < (int)inputs.size(); ii_index++) {
				if (input == inputs[ii_index]) {
					is_match = true;
					break;
				}
			}
			if (!is_match) {
				inputs.push_back(input);
			}
		}
		scope->signal->potential_inputs.push_back(inputs);
	}

	scope->signal->potential_existing_count = 0;
	scope->signal->potential_explore_count = 0;
}
