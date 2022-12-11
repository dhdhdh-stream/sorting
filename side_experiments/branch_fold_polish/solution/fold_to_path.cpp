#include "fold_to_path.h"

#include <iostream>

#include "definitions.h"

using namespace std;

Scope* construct_scope_helper(vector<FinishedStep*> finished_steps,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<SmallNetwork*>& outer_input_networks,
							  FoldNetwork*& outer_score_network,
							  double& outer_average_misguess,
							  double& outer_average_local_impact,
							  bool& outer_active_compress,
							  int& outer_compress_new_size,
							  FoldNetwork*& outer_compress_network,
							  int& outer_compress_original_size) {
	vector<int> scope_starts;
	vector<int> scope_ends;

	int curr_start = -1;
	int num_scopes = 0;
	for (int n_index = 0; n_index < (int)finished_steps.size(); n_index++) {
		num_scopes++;
		num_scopes -= finished_steps[n_index]->compress_num_layers;
		if (finished_steps[n_index]->compress_new_size > 0) {
			num_scopes++;
		}

		if (num_scopes > 0) {
			if (curr_start == -1) {
				curr_start = n_index;
			}
		} else {
			// num_scopes can be < 0 if end of scope
			if (curr_start != -1) {
				scope_starts.push_back(curr_start);
				scope_ends.push_back(n_index);

				curr_start = -1;
			}
		}
	}

	int scope_num_inputs = 0;

	vector<bool> scope_is_inner_scope;
	vector<Scope*> scope_scopes;
	vector<int> scope_obs_sizes;

	vector<vector<FoldNetwork*>> scope_inner_input_networks;
	vector<vector<int>> scope_inner_input_sizes;
	vector<double> scope_scope_scale_mod;

	vector<int> scope_step_types;	// initially STEP_TYPE_STEP
	vector<Branch*> scope_branches;	// initially NULL
	vector<Fold*> scope_folds;		// initially NULL

	vector<FoldNetwork*> scope_score_networks;

	vector<double> scope_average_misguesses;
	vector<double> scope_average_inner_scope_impacts;
	vector<double> scope_average_local_impacts;
	vector<double> scope_average_inner_branch_impacts;	// initially doesn't matter

	vector<bool> scope_active_compress;
	vector<int> scope_compress_new_sizes;
	vector<FoldNetwork*> scope_compress_networks;
	vector<int> scope_compress_original_sizes;

	int n_index = 0;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		while (true) {
			if (n_index < scope_starts[s_index]) {
				scope_is_inner_scope.push_back(finished_steps[n_index]->is_inner_scope);
				scope_scopes.push_back(finished_steps[n_index]->scope);
				scope_obs_sizes.push_back(finished_steps[n_index]->obs_size);

				scope_inner_input_networks.push_back(vector<FoldNetwork*>());
				scope_inner_input_sizes.push_back(vector<int>());
				scope_scale_mod.push_back(finished_steps[n_index]->scope_scale_mod);
				if (finished_steps[n_index]->is_inner_scope) {
					scope_inner_input_networks.back().push_back(finished_steps[n_index]->inner_input_network);
					scope_inner_input_sizes.back().push_back(finished_steps[n_index]->scope->num_inputs);

					for (int i_index = 0; i_index < finished_steps[n_index]->inner_input_input_networks.size(); i_index++) {
						outer_input_layer.push_back(finished_steps[n_index]->inner_input_input_layer[i_index]);

						
					}
				}

				for (int i_index = 0; i_index < )

				n_index++;
			} else {

			}
		}
	}
}

void fold_to_path(vector<FinishedStep*> finished_steps,
				  vector<bool>& is_inner_scope,
				  vector<Scope*>& scopes,
				  vector<int>& obs_sizes,
				  vector<vector<FoldNetwork*>>& inner_input_networks,
				  vector<vector<int>>& inner_input_sizes,
				  vector<double>& scope_scale_mod,
				  vector<bool>& step_types,
				  vector<Branch*>& branches,
				  vector<Fold*>& folds,
				  vector<FoldNetwork*>& score_networks,
				  vector<double>& average_misguesses,
				  vector<double>& average_inner_scope_impacts,
				  vector<double>& average_local_impacts,
				  vector<double>& average_inner_branch_impacts,
				  vector<bool>& active_compress,
				  vector<int>& compress_new_sizes,
				  vector<FoldNetwork*>& compress_networks,
				  vector<int>& compress_original_sizes) {

}
