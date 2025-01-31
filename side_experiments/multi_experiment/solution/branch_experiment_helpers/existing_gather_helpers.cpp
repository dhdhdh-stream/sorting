#include "branch_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::existing_gather_activate(
		ScopeHistory* scope_history) {
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

	for (int f_index = 0; f_index < GATHER_FACTORS_PER_ITER; f_index++) {
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

void BranchExperiment::existing_gather_backprop() {
	this->state_iter++;
}

void BranchExperiment::existing_gather_update() {
	if (this->state_iter >= GATHER_ITERS
			|| this->existing_inputs.size() >= GATHER_ITERS) {
		while (this->existing_inputs.size() > GATHER_ITERS) {
			uniform_int_distribution<int> remove_distribution(0, this->existing_inputs.size()-1);
			int remove_index = remove_distribution(generator);
			this->existing_inputs.erase(this->existing_inputs.begin() + remove_index);
		}

		while (this->existing_factor_ids.size() > GATHER_ITERS * GATHER_FACTORS_PER_ITER) {
			uniform_int_distribution<int> remove_distribution(0, this->existing_factor_ids.size()-1);
			int remove_index = remove_distribution(generator);
			this->existing_factor_ids.erase(this->existing_factor_ids.begin() + remove_index);
		}

		this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	}
}
