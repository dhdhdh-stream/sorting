#include "obs_experiment.h"

using namespace std;

void ObsExperiment::flat_helper() {

	if (this->test_hook_index != -1) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts.size()-1; c_index++) {
				if (this->test_hook_scope_contexts[c_index] != context[context.size()-this->test_hook_scope_contexts.size()+c_index].scope_id
						|| this->test_hook_node_contexts[c_index] != context[context.size()-this->test_hook_scope_contexts.size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			run_helper.experiment_history->test_indexes.push_back(this->test_hook_index);
			if (this->test_hook_obs_id == -1) {
				run_helper.experiment_history->test_vals.push_back(history->obs_snapshot);
			} else {
				run_helper.experiment_history->test_vals.push_back(history->state_snapshots[this->test_hook_obs_id]);
			}
		}
	}

}

void ObsExperiment::flat() {




}
