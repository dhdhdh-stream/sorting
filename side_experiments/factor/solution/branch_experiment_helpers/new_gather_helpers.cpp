#include "branch_experiment.h"

using namespace std;

const int NEW_GATHER_ITERS = 10;

void BranchExperiment::existing_gather_activate(
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		if (this->new_inputs.size() < NETWORK_NUM_INPUTS) {
			vector<Scope*> scope_context;
			vector<int> node_context;
			int node_count = 0;
			pair<pair<vector<Scope*>,vector<int>>,pair<int,int>> new_input;
			gather_possible_helper(scope_history,
								   scope_context,
								   node_context,
								   node_count,
								   new_input);

			bool is_existing = false;
			for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
				if (new_input == this->new_inputs[i_index]) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->new_inputs.push_back(new_input);
			}
		}

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void BranchExperiment::existing_gather_backprop() {
	this->state_iter++;
	if (this->state_iter >= NEW_GATHER_ITERS) {
		this->scope_histories.reserve(NUM_DATAPOINTS);
		this->target_val_histories.reserve(NUM_DATAPOINTS);

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
		this->state_iter = 0;
	}
}
