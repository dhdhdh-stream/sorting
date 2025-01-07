#include "obs_node.h"

using namespace std;

void ObsNode::experiment_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

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
			context[context.size() - this->input_scope_contexts[i_index].size()]
				.obs_history[{{this->input_scope_contexts[i_index],
					this->input_node_context_ids[i_index]}, {-1,this->input_obs_indexes[i_index]}}] = obs[this->input_obs_indexes[i_index]];
		}
	}

	history->factor_initialized = vector<bool>(this->factors.size());
	history->factor_values = vector<double>(this->factors.size());
	for (int f_index = 0; f_index < (int)this->factors.size(); f_index++) {
		this->factors[f_index]->activate(context,
										 run_helper,
										 history->factor_initialized[f_index],
										 history->factor_values[f_index]);
	}

	curr_node = this->next_node;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
		if (is_selected) {
			return;
		}
	}
}
