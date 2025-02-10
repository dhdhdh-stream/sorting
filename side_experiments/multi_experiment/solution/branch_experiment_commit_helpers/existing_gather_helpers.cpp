#include "branch_experiment.h"

#include "constants.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::existing_gather_commit_activate(
		ScopeHistory* scope_history,
		ScopeHistory* temp_history) {
	vector<Scope*> scope_context;
	vector<int> node_context;
	int node_count = 0;
	pair<pair<vector<Scope*>,vector<int>>,pair<int,int>> new_input;
	gather_possible_helper(scope_history,
						   scope_context,
						   node_context,
						   node_count,
						   new_input);
	gather_possible_helper(temp_history,
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
		int factor_count = 0;
		pair<int,int> new_factor = {-1, -1};
		gather_possible_factor_helper(scope_history,
									  factor_count,
									  new_factor);
		gather_possible_factor_helper(temp_history,
									  factor_count,
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
