#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	double sum_vals = 0.0;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		map<pair<pair<vector<Scope*>,vector<int>>, pair<int,int>>, double>::iterator it
			= context.back().obs_history.find(
				{{vector<Scope*>{this->parent},vector<int>{this->factor_ids[f_index].first}},
					{this->factor_ids[f_index].second,-1}});
		if (it != context.back().obs_history.end()) {
			sum_vals += it->second * this->factor_weights[f_index];
		}
	}

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (sum_vals >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	if (this->input_scope_contexts.size() > 0) {
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			bool match_context = false;
			if (context.size() >= this->input_scope_contexts[i_index].size()) {
				match_context = true;
				for (int l_index = 0; l_index < (int)this->input_scope_contexts[i_index].size()-1; l_index++) {
					int context_index = context.size() - this->input_scope_contexts[i_index].size() + l_index;
					if (context[context_index].scope != this->input_scope_contexts[i_index][l_index]
							|| context[context_index].node_id != this->input_node_context_ids[i_index][l_index]) {
						match_context = false;
						break;
					}
				}
			}

			if (match_context) {
				if (is_branch) {
					context[context.size() - this->input_scope_contexts[i_index].size()]
						.obs_history[{{this->input_scope_contexts[i_index],
							this->input_node_context_ids[i_index]}, {-1,-1}}] = 1.0;
				} else {
					context[context.size() - this->input_scope_contexts[i_index].size()]
						.obs_history[{{this->input_scope_contexts[i_index],
							this->input_node_context_ids[i_index]}, {-1,-1}}] = -1.0;
				}
			}
		}
	}

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}
