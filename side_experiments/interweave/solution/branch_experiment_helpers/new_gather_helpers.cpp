#include "branch_experiment.h"

#include "constants.h"
#include "globals.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::new_gather_activate(
		ScopeHistory* scope_history,
		BranchExperimentOverallHistory* overall_history) {
	uniform_int_distribution<int> active_distribution(
		0, overall_history->active_concurrents.size()-1);
	int concurrent_index = overall_history->active_concurrents[
		active_distribution(generator)];

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
	for (int i_index = 0; i_index < (int)this->new_inputs[concurrent_index].size(); i_index++) {
		if (new_input == this->new_inputs[concurrent_index][i_index]) {
			is_existing = true;
			break;
		}
	}
	if (!is_existing) {
		this->new_inputs[concurrent_index].push_back(new_input);
	}

	for (int f_index = 0; f_index < GATHER_FACTORS_PER_ITER; f_index++) {
		int factor_count = 0;
		pair<int,int> new_factor = {-1, -1};
		gather_possible_factor_helper(scope_history,
									  factor_count,
									  new_factor);

		if (new_factor.first != -1) {
			bool is_existing = false;
			for (int i_index = 0; i_index < (int)this->new_factor_ids[concurrent_index].size(); i_index++) {
				if (new_factor == this->new_factor_ids[concurrent_index][i_index]) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->new_factor_ids[concurrent_index].push_back(new_factor);
			}
		}
	}

	this->instance_iter++;
}

void BranchExperiment::new_gather_update() {
	this->run_iter++;
	if (this->run_iter >= GATHER_ITERS
			&& this->instance_iter >= GATHER_ITERS * BRANCH_EXPERIMENT_NUM_CONCURRENT) {
		for (int c_index = 0; c_index < BRANCH_EXPERIMENT_NUM_CONCURRENT; c_index++) {
			while (this->new_inputs[c_index].size() > GATHER_ITERS) {
				uniform_int_distribution<int> remove_distribution(0, this->new_inputs[c_index].size()-1);
				int remove_index = remove_distribution(generator);
				this->new_inputs[c_index].erase(this->new_inputs[c_index].begin() + remove_index);
			}

			while (this->new_factor_ids[c_index].size() > GATHER_ITERS * GATHER_FACTORS_PER_ITER) {
				uniform_int_distribution<int> remove_distribution(0, this->new_factor_ids[c_index].size()-1);
				int remove_index = remove_distribution(generator);
				this->new_factor_ids[c_index].erase(this->new_factor_ids[c_index].begin() + remove_index);
			}
		}

		this->new_input_histories = vector<vector<vector<double>>>(BRANCH_EXPERIMENT_NUM_CONCURRENT);
		this->new_factor_histories = vector<vector<vector<double>>>(BRANCH_EXPERIMENT_NUM_CONCURRENT);
		this->new_target_val_histories = vector<vector<double>>(BRANCH_EXPERIMENT_NUM_CONCURRENT);

		this->state = BRANCH_EXPERIMENT_STATE_TRAIN_NEW;
		this->instance_iter = 0;
		this->run_iter = 0;
	}
}
