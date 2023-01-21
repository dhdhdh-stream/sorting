#include "branch.h"

#include <iostream>
#include <limits>

#include "definitions.h"
#include "fold_to_path.h"

using namespace std;

void Branch::resolve_fold(int b_index) {
	cout << "Branch resolve_fold" << endl;

	int new_sequence_length;
	vector<bool> new_is_inner_scope;
	vector<Scope*> new_scopes;
	vector<Action> new_actions;
	vector<vector<FoldNetwork*>> new_inner_input_networks;
	vector<vector<int>> new_inner_input_sizes;
	vector<double> new_scope_scale_mod;
	vector<int> new_step_types;
	vector<Branch*> new_branches;
	vector<Fold*> new_folds;
	vector<FoldNetwork*> new_score_networks;
	vector<double> new_average_scores;
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
				 new_actions,
				 new_inner_input_networks,
				 new_inner_input_sizes,
				 new_scope_scale_mod,
				 new_step_types,
				 new_branches,
				 new_folds,
				 new_score_networks,
				 new_average_scores,
				 new_average_misguesses,
				 new_average_inner_scope_impacts,
				 new_average_local_impacts,
				 new_average_inner_branch_impacts,
				 new_active_compress,
				 new_compress_new_sizes,
				 new_compress_networks,
				 new_compress_original_sizes);

	// this->score_networks[b_index] already set correctly

	new_sequence_length++;
	new_is_inner_scope.insert(new_is_inner_scope.begin(), false);
	new_scopes.insert(new_scopes.begin(), NULL);
	new_actions.insert(new_actions.begin(), Action());

	new_inner_input_networks.insert(new_inner_input_networks.begin(), vector<FoldNetwork*>());
	new_inner_input_sizes.insert(new_inner_input_sizes.begin(), vector<int>());
	new_scope_scale_mod.insert(new_scope_scale_mod.begin(), 1.0);

	new_step_types.insert(new_step_types.begin(), STEP_TYPE_STEP);
	new_branches.insert(new_branches.begin(), NULL);
	new_folds.insert(new_folds.begin(), NULL);

	new_score_networks.insert(new_score_networks.begin(), NULL);

	new_average_scores.insert(new_average_scores.begin(), this->folds[b_index]->starting_average_score);
	new_average_misguesses.insert(new_average_misguesses.begin(), this->folds[b_index]->starting_average_misguess);
	new_average_inner_scope_impacts.insert(new_average_inner_scope_impacts.begin(), 0.0);	// doesn't matter
	new_average_local_impacts.insert(new_average_local_impacts.begin(), this->folds[b_index]->starting_average_local_impact);
	new_average_inner_branch_impacts.insert(new_average_inner_branch_impacts.begin(), 0.0);	// doesn't matter

	if (this->folds[b_index]->curr_starting_compress_new_size < this->folds[b_index]->starting_compress_original_size
			&& this->folds[b_index]->curr_starting_compress_new_size > 0) {
		new_active_compress.insert(new_active_compress.begin(), true);
	} else {
		new_active_compress.insert(new_active_compress.begin(), false);
	}
	new_compress_new_sizes.insert(new_compress_new_sizes.begin(), this->folds[b_index]->curr_starting_compress_new_size);
	new_compress_networks.insert(new_compress_networks.begin(), this->folds[b_index]->curr_starting_compress_network);
	this->folds[b_index]->curr_starting_compress_network = NULL;
	new_compress_original_sizes.insert(new_compress_original_sizes.begin(), this->folds[b_index]->starting_compress_original_size);

	BranchPath* new_branch_path = new BranchPath(this->num_inputs,
												 this->num_outputs,
												 new_sequence_length,
												 new_is_inner_scope,
												 new_scopes,
												 new_actions,
												 new_inner_input_networks,
												 new_inner_input_sizes,
												 new_scope_scale_mod,
												 new_step_types,
												 new_branches,
												 new_folds,
												 new_score_networks,
												 new_average_scores,
												 new_average_misguesses,
												 new_average_inner_scope_impacts,
												 new_average_local_impacts,
												 new_average_inner_branch_impacts,
												 new_active_compress,
												 new_compress_new_sizes,
												 new_compress_networks,
												 new_compress_original_sizes,
												 true);	// last networks matter for folds

	this->is_branch[b_index] = true;
	this->branches[b_index] = new_branch_path;
	// this->end_scale_mods already set correctly

	delete this->folds[b_index];
	this->folds[b_index] = NULL;
}
