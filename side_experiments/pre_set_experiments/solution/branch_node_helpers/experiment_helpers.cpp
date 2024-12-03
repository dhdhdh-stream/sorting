#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void BranchNode::experiment_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	BranchNodeHistory* history = new BranchNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	run_helper.num_analyze += (int)this->inputs.size();

	vector<double> input_vals(this->inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		map<pair<pair<vector<Scope*>,vector<int>>,int>, double>::iterator it =
			context.back().obs_history.find(this->inputs[i_index]);
		if (it != context.back().obs_history.end()) {
			input_vals[i_index] = it->second;
		}
	}
	this->network->activate(input_vals);

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (this->network->output->acti_vals[0] >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	history->is_branch = is_branch;

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
							this->input_node_context_ids[i_index]}, -1}] = 1.0;
				} else {
					context[context.size() - this->input_scope_contexts[i_index].size()]
						.obs_history[{{this->input_scope_contexts[i_index],
							this->input_node_context_ids[i_index]}, -1}] = -1.0;
				}
			}
		}
	}

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	if (run_helper.experiment_node == this
			&& this->experiment_is_branch == is_branch) {
		this->experiment->activate(
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
	}
}
