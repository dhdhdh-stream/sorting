#include "factor.h"

#include <iostream>

#include "network.h"

using namespace std;

void Factor::activate(int index,
					  vector<ContextLayer>& context,
					  RunHelper& run_helper,
					  bool& initialized,
					  double& value) {
	bool has_match = false;
	vector<bool> matches(this->input_scope_contexts.size());
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
			has_match = true;

			matches[i_index] = true;
		} else {
			matches[i_index] = false;
		}
	}

	if (has_match) {
		// run_helper.num_analyze += (int)this->inputs.size();

		vector<double> input_vals(this->inputs.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			map<pair<pair<vector<Scope*>,vector<int>>,
				pair<int,int>>,double>::iterator it =
					context.back().obs_history.find(this->inputs[i_index]);
			if (it != context.back().obs_history.end()) {
				input_vals[i_index] = it->second;
			}
		}
		this->network->activate(input_vals);

		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (matches[i_index]) {
				context[context.size() - this->input_scope_contexts[i_index].size()]
					.obs_history[{{this->input_scope_contexts[i_index],
						this->input_node_context_ids[i_index]}, {index,-1}}] = this->network->output->acti_vals[0];
			}
		}

		initialized = true;
		value = this->network->output->acti_vals[0];
	} else {
		initialized = false;
	}
}
