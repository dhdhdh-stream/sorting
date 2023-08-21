#include "loop_experiment.h"

using namespace std;

void LoopExperiment::seed_new_scope_activate_helper(
		bool on_path,
		) {



}

void LoopExperiment::seed_new_sequence_activate_helper(
		double& new_val,
		SequenceHistory* seed_history,
		SequenceHistory* history) {
	for (int l_index = 0; l_index < (int)seed_history->node_histories.size(); l_index++) {
		int scope_id = this->sequence->scopes[l_index]->id;
		vector<StateNetwork*> scope_state_networks = &(this->test_new_state_networks.find(scope_id)->second);

		history->node_histories.push_back(vector<StateNetwork*>());
		for (int n_index = 0; n_index < (int)seed_history->node_histories[l_index].size(); n_index++) {
			if (seed_history->node_histories[l_index][n_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* seed_action_node_history = (ActionNodeHistory*)history->node_histories[l_index][n_index];
				ActionNodeHistory* new_action_node_history = new ActionNodeHistory(seed_action_node_history);
				history->node_histories[l_index].push_back(new_action_node_history);

				if (scope_state_networks[n_index] == NULL) {

				}

				new_action_node_history->test_snapshot = new_val;

				StateNetwork* network = scope_state_networks[n_index];
				StateNetworkHistory* network_history = new StateNetworkHistory();
				network->new_activate(new_action_node_history->obs_snapshot,
									  new_action_node_history->input_snapshot,
									  new_action_node_history->new_snapshot,
									  new_action_node_history->test_snapshot,
									  network_history);
				new_action_node_history->test_network_history = network_history;
				new_val += network->output->acti_vals[0];
			} else if (seed_history->node_histories[l_index][n_index]->node->type == NODE_TYPE_SCOPE) {

			}
		}
	}
}

void LoopExperiment::seed_new_experiment_activate_helper(
		double& new_val,
		LoopExperimentHistory* seed_history,
		LoopExperimentHistory* history) {
	for (int i_index = 0; i_index < (int)seed_history->sequence_histories.size(); i_index++) {
		run_helper.scale_factor *= this->scale_mod->weight;

		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		history->sequence_histories.push_back(sequence_history);
		seed_new_sequence_activate_helper(new_val,
										  seed_history->sequence_histories[i_index],
										  sequence_history);

		run_helper.scale_factor /= this->scale_mod->weight;
	}
}

void LoopExperiment::seed_new_activate() {
	bool can_zero;
	if (rand()%5 == 0) {
		can_zero = true;
	} else {
		can_zero = false;
	}
	double test_val = 0.0;

	int starting_context_index;
	if (this->test_state_layer == 0) {
		starting_context_index = 0;
	} else {
		starting_context_index = (int)this->seed_pre_context_histories.size()
			- (int)this->scope_context.size() + (this->test_state_layer-1);
	}
	vector<ScopeHistory*> new_pre_scope_histories;
	for (int c_index = starting_context_index; c_index < (int)this->seed_pre_context_histories.size(); c_index++) {
		ScopeHistory* new_scope_history = new ScopeHistory(this->seed_pre_context_histories[c_index]->scope);
		new_pre_scope_histories.push_back(new_scope_history);
		seed_new_scope_activate_helper(true,
									   test_val,
									   can_zero,
									   this->seed_pre_context_histories[c_index]->scope,
									   new_scope_history);
	}

	LoopExperimentHistory* new_experiment_history = new LoopExperimentHistory(this);
	seed_new_experiment_activate_helper(test_val,
										can_zero,
										this->seed_experiment_history,
										new_experiment_history);

	vector<ScopeHistory*> new_post_scope_histories;
	for (int c_index = (int)this->seed_pre_context_histories.size()-1; c_index >= starting_context_index; c_index--) {
		ScopeHistory* new_scope_history = new ScopeHistory(this->seed_post_context_histories[c_index]->scope);
		new_post_scope_histories.push_back(new_scope_history);
		seed_new_scope_activate_helper(false,
									   test_val,
									   can_zero,
									   this->seed_post_context_histories[c_index]->scope,
									   new_scope_history);
	}

	

}
