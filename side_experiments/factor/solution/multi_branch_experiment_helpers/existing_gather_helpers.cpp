#include "multi_branch_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "solution_helpers.h"

using namespace std;

void MultiBranchExperiment::existing_gather_activate(
		ScopeHistory* scope_history) {
	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		if (this->existing_inputs.size() < NETWORK_NUM_INPUTS) {
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
			for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
				if (new_input == this->existing_inputs[i_index]) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->existing_inputs.push_back(new_input);
			}
		}

		for (int f_index = 0; f_index < GATHER_FACTORS_PER_ITER; f_index++) {
			if (this->existing_factor_ids.size() < GATHER_ITERS * GATHER_FACTORS_PER_ITER) {
				pair<int,int> new_factor = {-1, -1};
				gather_possible_factor_helper(scope_history,
											  new_factor);

				if (new_factor.first != -1) {
					bool is_existing = false;
					for (int i_index = 0; i_index < (int)this->existing_factor_ids.size(); i_index++) {
						if (new_factor == this->existing_factor_ids[i_index]) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						this->existing_factor_ids.push_back(new_factor);
					}
				}
			}
		}

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void MultiBranchExperiment::existing_gather_backprop() {
	this->state_iter++;
	if (this->state_iter >= GATHER_ITERS) {
		this->state = MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
		this->state_iter = 0;
	}
}
