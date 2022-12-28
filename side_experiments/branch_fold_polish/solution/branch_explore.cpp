#include "branch.h"

#include <iostream>
#include <limits>

#include "fold_to_path.h"

using namespace std;

void Branch::resolve_fold(int b_index) {
	int new_sequence_length;
	vector<bool> new_is_inner_scope;
	vector<Scope*> new_scopes;
	vector<int> new_obs_sizes;
	vector<vector<FoldNetwork*>> new_inner_input_networks;
	vector<vector<int>> new_inner_input_sizes;
	vector<double> new_scope_scale_mod;
	vector<bool> new_step_types;
	vector<Branch*> new_branches;
	vector<Fold*> new_folds;
	vector<FoldNetwork*> new_score_networks;
	vector<double> new_average_misguesses;
	vector<double> new_average_inner_scope_impacts;
	vector<double> new_average_local_impacts;
	vector<double> new_average_inner_branch_impacts;
	vector<bool> new_active_compress;
	vector<int> new_compress_new_sizes;
	vector<FoldNetwork*> new_compress_networks;
	vector<int> new_compress_original_sizes;
	fold_to_path(this->folds[b_index]->finished_steps,
				 new_sequence_length,
				 new_is_inner_scope,
				 new_scopes,
				 new_obs_sizes,
				 new_inner_input_networks,
				 new_inner_input_sizes,
				 new_scope_scale_mod,
				 new_step_types,
				 new_branches,
				 new_folds,
				 new_score_networks,
				 new_average_misguesses,
				 new_average_inner_scope_impacts,
				 new_average_local_impacts,
				 new_average_inner_branch_impacts,
				 new_active_compress,
				 new_compress_new_sizes,
				 new_compress_networks,
				 new_compress_original_sizes);

	// this->score_networks[b_index] already set correctly

	BranchPath* new_branch_path = new BranchPath(new_sequence_length,
												 new_is_inner_scope,
												 new_scopes,
												 new_obs_sizes,
												 new_inner_input_networks,
												 new_inner_input_sizes,
												 new_scope_scale_mod,
												 new_step_types,
												 new_branches,
												 new_folds,
												 new_score_networks,
												 new_average_misguesses,
												 new_average_inner_scope_impacts,
												 new_average_local_impacts,
												 new_average_inner_branch_impacts,
												 new_active_compress,
												 new_compress_new_sizes,
												 new_compress_networks,
												 new_compress_original_sizes,
												 this->is_scope_end);

	this->is_branch[b_index] = true;
	this->branches[b_index] = new_branch_path;
	// this->end_scale_mods already set correctly

	delete this->folds[b_index];
	this->folds[b_index] = NULL;
}
