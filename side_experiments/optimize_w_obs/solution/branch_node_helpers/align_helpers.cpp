#include "branch_node.h"

#include "network.h"

using namespace std;

void BranchNode::align_activate(AbstractNode*& curr_node,
								vector<ContextLayer>& context) {
	// TODO: is_local
	vector<double> input_vals(this->inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		map<pair<pair<vector<int>,vector<int>>,int>, double>::iterator it =
			context.back().obs_history.find(this->inputs[i_index]);
		if (it != context.back().obs_history.end()) {
			input_vals[i_index] = it->second;
		}
	}
	this->network->activate(input_vals);

	bool is_branch;
	if (this->network->output->acti_vals[0] >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	if (this->input_scope_context_ids.size() > 0) {
		for (int i_index = 0; i_index < (int)this->input_scope_context_ids.size(); i_index++) {
			bool match_context = false;
			if (context.size() >= this->input_scope_context_ids[i_index].size()) {
				match_context = true;
				for (int l_index = 0; l_index < (int)this->input_scope_context_ids[i_index].size()-1; l_index++) {
					int context_index = context.size() - this->input_scope_context_ids[i_index].size() + l_index;
					if (context[context_index].scope_id != this->input_scope_context_ids[i_index][l_index]
							|| context[context_index].node_id != this->input_node_context_ids[i_index][l_index]) {
						match_context = false;
						break;
					}
				}
			}

			if (match_context) {
				if (is_branch) {
					context[context.size() - this->input_scope_context_ids[i_index].size()]
						.obs_history[{{this->input_scope_context_ids[i_index],
							this->input_node_context_ids[i_index]}, -1}] = 1.0;
				} else {
					context[context.size() - this->input_scope_context_ids[i_index].size()]
						.obs_history[{{this->input_scope_context_ids[i_index],
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
}
