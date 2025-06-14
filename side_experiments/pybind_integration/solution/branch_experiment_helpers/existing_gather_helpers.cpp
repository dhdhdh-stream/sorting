#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void BranchExperiment::existing_gather_check_activate(SolutionWrapper* wrapper) {
	vector<Scope*> scope_context;
	vector<int> node_context;
	int node_count = 0;
	Input new_input;
	gather_possible_helper(wrapper->scope_histories.back(),
						   scope_context,
						   node_context,
						   node_count,
						   new_input,
						   wrapper);

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
		gather_possible_factor_helper(wrapper->scope_histories.back(),
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
	if (this->state_iter >= GATHER_ITERS) {
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

		this->sum_num_instances = 0;

		this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
		this->state_iter = 0;
	}
}
