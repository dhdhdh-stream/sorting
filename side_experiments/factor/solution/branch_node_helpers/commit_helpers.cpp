#include "branch_node.h"

using namespace std;

void BranchNode::commit_gather_activate(AbstractNode*& curr_node,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										int& node_count,
										AbstractNode*& potential_node_context,
										bool& potential_is_branch) {
	double sum_vals = 0.0;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		map<pair<pair<vector<Scope*>,vector<int>>, pair<int,int>>>::iterator it
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

	if (solution->timestamp >= MAINTAIN_ITERS
			|| (this->parent->id == 0 || this->parent->id > solution->num_existing_scopes)) {
		uniform_int_distribution<int> select_distribution(0, node_count);
		node_count++;
		if (select_distribution(generator) == 0) {
			potential_node_context = this;
			potential_is_branch = false;
		}
	}
}

void BranchNode::commit_activate(AbstractNode*& curr_node,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 PotentialCommit* potential_commit) {
	double sum_vals = 0.0;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		map<pair<pair<vector<Scope*>,vector<int>>, pair<int,int>>>::iterator it
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

	if (potential_commit->node_context == this) {
		potential_commit->activate(curr_node,
								   problem,
								   context,
								   run_helper);
	}
}
