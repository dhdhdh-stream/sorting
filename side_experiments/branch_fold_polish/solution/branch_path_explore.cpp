#include "branch_path.h"

#include <iostream>

#include "definitions.h"

using namespace std;

// identical to for Scope

void BranchPath::explore_replace() {
	if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_STEP) {
		// can't be scope end

		delete this->score_networks[this->explore_index_inclusive];
		this->score_networks[this->explore_index_inclusive] = NULL;

		if (this->active_compress[this->explore_index_inclusive]) {
			delete this->compress_networks[this->explore_index_inclusive];
			this->compress_networks[this->explore_index_inclusive] = NULL;
		}
	} else if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_BRANCH) {
		delete this->branches[this->explore_index_inclusive];
		this->branches[this->explore_index_inclusive] = NULL;
	} else {
		delete this->score_networks[this->explore_index_inclusive];
		this->score_networks[this->explore_index_inclusive] = NULL;

		delete this->folds[this->explore_index_inclusive];
		this->folds[this->explore_index_inclusive] = NULL;
	}

	this->step_types[this->explore_index_inclusive] = STEP_TYPE_FOLD;

	this->score_networks[this->explore_index_inclusive] = this->explore_fold->starting_score_network;
	this->explore_fold->starting_score_network = NULL;
	delete this->explore_fold->combined_score_network;
	this->explore_fold->combined_score_network = NULL;

	this->folds[this->explore_index_inclusive] = this->explore_fold;

	this->average_misguesses[this->explore_index_inclusive] = this->explore_fold->average_misguess;
	// this->average_inner_scope_impacts[this->explore_index_inclusive] unchanged
	this->average_local_impacts[this->explore_index_inclusive] = 0.0;	// no longer matters
	this->average_inner_branch_impacts[this->explore_index_inclusive] = 0.0;	// initialize to 0.0

	for (int a_index = this->explore_index_inclusive+1; a_index < this->explore_end_non_inclusive; a_index++) {
		if (this->is_inner_scope[a_index]) {
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				delete this->inner_input_networks[a_index][i_index];
			}

			delete this->scopes[a_index];
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == this->scopes.size()-1) {
				// scope end -- do nothing
			} else {
				delete this->score_networks[a_index];

				if (this->active_compress[a_index]) {
					delete this->compress_networks[a_index];
				}
			}
		} else if (this->step_types[a_index] == STEP_TYPE_BRANCH) {
			delete this->branches[a_index];
		} else {
			delete this->score_networks[a_index];

			delete this->folds[a_index];
		}

	}

	this->is_inner_scope.erase(this->is_inner_scope.begin()+this->explore_index_inclusive+1,
		this->is_inner_scope.begin()+this->explore_end_non_inclusive);
	this->scopes.erase(this->scopes.begin()+this->explore_index_inclusive+1,
		this->scopes.begin()+this->explore_end_non_inclusive);
	this->obs_sizes.erase(this->obs_sizes.begin()+this->explore_index_inclusive+1,
		this->obs_sizes.begin()+this->explore_end_non_inclusive);

	this->inner_input_networks.erase(this->inner_input_networks.begin()+this->explore_index_inclusive+1,
		this->inner_input_networks.begin()+this->explore_end_non_inclusive);
	this->inner_input_sizes.erase(this->inner_input_sizes.begin()+this->explore_index_inclusive+1,
		this->inner_input_sizes.begin()+this->explore_end_non_inclusive);
	this->scope_scale_mod.erase(this->scope_scale_mod.begin()+this->explore_index_inclusive+1,
		this->scope_scale_mod.begin()+this->explore_end_non_inclusive);

	this->step_types.erase(this->step_types.begin()+this->explore_index_inclusive+1,
		this->step_types.begin()+this->explore_end_non_inclusive);
	this->branches.erase(this->branches.begin()+this->explore_index_inclusive+1,
		this->branches.begin()+this->explore_end_non_inclusive);
	this->folds.erase(this->folds.begin()+this->explore_index_inclusive+1,
		this->folds.begin()+this->explore_end_non_inclusive);

	this->score_networks.erase(this->score_networks.begin()+this->explore_index_inclusive+1,
		this->score_networks.begin()+this->explore_end_non_inclusive);

	this->average_misguesses.erase(this->average_misguesses.begin()+this->explore_index_inclusive+1,
		this->average_misguesses.begin()+this->explore_end_non_inclusive);
	this->average_inner_scope_impacts.erase(this->average_inner_scope_impacts.begin()+this->explore_index_inclusive+1,
		this->average_inner_scope_impacts.begin()+this->explore_end_non_inclusive);
	this->average_local_impacts.erase(this->average_local_impacts.begin()+this->explore_index_inclusive+1,
		this->average_local_impacts.begin()+this->explore_end_non_inclusive);
	this->average_inner_branch_impacts.erase(this->average_inner_branch_impacts.begin()+this->explore_index_inclusive+1,
		this->average_inner_branch_impacts.begin()+this->explore_end_non_inclusive);

	this->active_compress.erase(this->active_compress.begin()+this->explore_index_inclusive+1,
		this->active_compress.begin()+this->explore_end_non_inclusive);
	this->compress_new_sizes.erase(this->compress_new_sizes.begin()+this->explore_index_inclusive+1,
		this->compress_new_sizes.begin()+this->explore_end_non_inclusive);
	this->compress_networks.erase(this->compress_networks.begin()+this->explore_index_inclusive+1,
		this->compress_networks.begin()+this->explore_end_non_inclusive);
	this->compress_original_sizes.erase(this->compress_original_sizes.begin()+this->explore_index_inclusive+1,
		this->compress_original_sizes.begin()+this->explore_end_non_inclusive);

	this->explore_index_inclusive = -1;
	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

void BranchPath::explore_branch() {
	if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_BRANCH
			&& this->explore_index_inclusive+1 == this->explore_end_non_inclusive) {
		// this->branches[this->explore_index_inclusive]->passed_branch_score == false
		delete this->branches[this->explore_index_inclusive]->branch_score_network;
		this->branches[this->explore_index_inclusive]->branch_score_network = this->explore_fold->combined_score_network;
		this->explore_fold->combined_score_network = NULL;

		this->branches[this->explore_index_inclusive]->score_networks.push_back(
			this->explore_fold->starting_score_network);
		this->explore_fold->starting_score_network = NULL;
		this->branches[this->explore_index_inclusive]->is_branch.push_back(false);
		this->branches[this->explore_index_inclusive]->branches.push_back(NULL);
		this->branches[this->explore_index_inclusive]->folds.push_back(this->explore_fold);
		this->branches[this->explore_index_inclusive]->end_scale_mods.push_back(
			this->explore_fold->end_scale_mod_calc->output->constants[0]);
		delete this->explore_fold->end_scale_mod_calc;
		this->explore_fold->end_scale_mod_calc = NULL;
	} else {
		vector<FoldNetwork*> new_score_networks;
		if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_BRANCH) {
			new_score_networks.push_back(this->branches[this->explore_index_inclusive]->branch_score_network);
			this->branches[this->explore_index_inclusive]->branch_score_network = NULL;
			this->branches[this->explore_index_inclusive]->passed_branch_score = true;
		} else {
			new_score_networks.push_back(this->score_networks[this->explore_index_inclusive]);
			this->score_networks[this->explore_index_inclusive] = NULL;
		}
		new_score_networks.push_back(this->explore_fold->starting_score_network);
		this->explore_fold->starting_score_network = NULL;

		vector<bool> branch_is_inner_scope;
		branch_is_inner_scope.push_back(false);	// start doesn't matter
		branch_is_inner_scope.insert(branch_is_inner_scope.end(),
			this->is_inner_scope.begin()+this->explore_index_inclusive+1,
			this->is_inner_scope.begin()+this->explore_end_non_inclusive);
		vector<Scope*> branch_scopes;
		branch_scopes.push_back(NULL);	// start doesn't matter
		branch_scopes.insert(branch_scopes.end(),
			this->scopes.begin()+this->explore_index_inclusive+1,
			this->scopes.begin()+this->explore_end_non_inclusive);
		vector<int> branch_obs_sizes;
		branch_obs_sizes.push_back(-1);	// start doesn't matter
		branch_obs_sizes.insert(branch_obs_sizes.end(),
			this->obs_sizes.begin()+this->explore_index_inclusive+1,
			this->obs_sizes.begin()+this->explore_end_non_inclusive);

		vector<vector<FoldNetwork*>> branch_inner_input_networks;
		branch_inner_input_networks.push_back(vector<FoldNetwork*>());	// start doesn't matter
		branch_inner_input_networks.insert(branch_inner_input_networks.end(),
			this->inner_input_networks.begin()+this->explore_index_inclusive+1,
			this->inner_input_networks.begin()+this->explore_end_non_inclusive);
		vector<vector<int>> branch_inner_input_sizes;
		branch_inner_input_sizes.push_back(vector<int>());	// start doesn't matter
		branch_inner_input_sizes.insert(branch_inner_input_sizes.end(),
			this->inner_input_sizes.begin()+this->explore_index_inclusive+1,
			this->inner_input_sizes.begin()+this->explore_end_non_inclusive);
		vector<double> branch_scope_scale_mod;
		branch_scope_scale_mod.push_back(0.0);	// start doesn't matter
		branch_scope_scale_mod.insert(branch_scope_scale_mod.end(),
			this->scope_scale_mod.begin()+this->explore_index_inclusive+1,
			this->scope_scale_mod.begin()+this->explore_end_non_inclusive);

		vector<bool> branch_step_types(this->step_types.begin()+this->explore_index_inclusive,
			this->step_types.begin()+this->explore_end_non_inclusive);
		// set this->step_types[this->explore_index_inclusive] below
		vector<Branch*> branch_branches(this->branches.begin()+this->explore_index_inclusive,
			this->branches.begin()+this->explore_end_non_inclusive);
		// set this->branches[this->explore_index_inclusive] below
		vector<Fold*> branch_folds(this->folds.begin()+this->explore_index_inclusive,
			this->folds.begin()+this->explore_end_non_inclusive);
		this->branch_folds[this->explore_index_inclusive] = NULL;

		vector<FoldNetwork*> branch_score_networks;
		branch_score_networks.push_back(NULL);	// start doesn't matter
		branch_score_networks.insert(branch_score_networks.end(),
			this->score_networks.begin()+this->explore_index_inclusive+1,
			this->score_networks.begin()+this->explore_end_non_inclusive);

		vector<double> branch_average_misguesses(this->average_misguesses.begin()+this->explore_index_inclusive,
			this->average_misguesses.begin()+this->explore_end_non_inclusive);
		this->average_misguesses[this->explore_index_inclusive] = this->explore_fold->average_misguess;
		vector<double> branch_average_inner_scope_impacts;
		branch_average_inner_scope_impacts.push_back(0.0);	// start doesn't matter
		branch_average_inner_scope_impacts.insert(branch_average_inner_scope_impacts.end(),
			this->average_inner_scope_impacts.begin()+this->explore_index_inclusive+1,
			this->average_inner_scope_impacts.begin()+this->explore_end_non_inclusive);
		// this->average_inner_scope_impacts[this->explore_index_inclusive] unchanged
		vector<double> branch_average_local_impacts(this->average_local_impacts.begin()+this->explore_index_inclusive,
			this->average_local_impacts.begin()+this->explore_end_non_inclusive);
		this->average_local_impacts[this->explore_index_inclusive] = 0.0;	// no longer matters
		vector<double> branch_average_inner_branch_impacts(this->average_inner_branch_impacts.begin()+this->explore_index_inclusive,
			this->average_inner_branch_impacts.begin()+this->explore_end_non_inclusive);
		this->average_inner_branch_impacts[this->explore_index_inclusive] = 0.0;	// initialize to 0.0

		vector<bool> branch_active_compress(this->active_compress.begin()+this->explore_index_inclusive,
			this->active_compress.begin()+this->explore_end_non_inclusive);
		this->active_compress[this->explore_index_inclusive] = false;
		vector<int> branch_compress_new_sizes(this->compress_new_sizes.begin()+this->explore_index_inclusive,
			this->compress_new_sizes.begin()+this->explore_end_non_inclusive);
		this->compress_new_sizes[this->explore_index_inclusive] = -1;
		vector<FoldNetwork*> branch_compress_networks(this->compress_networks.begin()+this->explore_index_inclusive,
			this->compress_networks.begin()+this->explore_end_non_inclusive);
		this->compress_networks[this->explore_index_inclusive] = NULL;
		vector<int> branch_compress_original_sizes(this->compress_original_sizes.begin()+this->explore_index_inclusive,
			this->compress_original_sizes.begin()+this->explore_end_non_inclusive);
		this->compress_original_sizes[this->explore_index_inclusive] = -1;

		BranchPath* new_branch_path = new BranchPath(branch_is_inner_scope,
													 branch_scopes,
													 branch_obs_sizes,
													 branch_inner_input_networks,
													 branch_inner_input_sizes,
													 branch_scope_scale_mod,
													 branch_step_types,
													 branch_branches,
													 branch_folds,
													 branch_score_networks,
													 branch_average_misguesses,
													 branch_average_inner_scope_impacts,
													 branch_average_local_impacts,
													 branch_average_inner_branch_impacts,
													 branch_active_compress,
													 branch_compress_new_sizes,
													 branch_compress_networks,
													 branch_compress_original_sizes);

		vector<bool> new_is_branch;
		new_is_branch.push_back(true);
		new_is_branch.push_back(false);

		vector<BranchPath*> new_branches;
		new_branches.push_back(new_branch_path);
		new_branches.push_back(NULL);

		vector<Fold*> new_folds;
		new_folds.push_back(NULL);
		new_folds.push_back(this->explore_fold);

		vector<double> new_end_scale_mods;
		new_end_scale_mods.push_back(1.0);
		new_end_scale_mods.push_back(this->explore_fold->end_scale_mod_calc->output->constants[0]);
		delete this->explore_fold->end_scale_mod_calc;
		this->explore_fold->end_scale_mod_calc = NULL;

		Branch* new_branch = new Branch(this->explore_fold->combined_score_network,
										new_score_networks,
										new_is_branch,
										new_branches,
										new_folds,
										new_end_scale_mods);
		this->explore_fold->combined_score_network = NULL;

		this->step_types[this->explore_index_inclusive] = STEP_TYPE_BRANCH;
		this->branches[this->explore_index_inclusive] = new_branch;
	}

	this->is_inner_scope.erase(this->is_inner_scope.begin()+this->explore_index_inclusive+1,
		this->is_inner_scope.begin()+this->explore_end_non_inclusive);
	this->scopes.erase(this->scopes.begin()+this->explore_index_inclusive+1,
		this->scopes.begin()+this->explore_end_non_inclusive);
	this->obs_sizes.erase(this->obs_sizes.begin()+this->explore_index_inclusive+1,
		this->obs_sizes.begin()+this->explore_end_non_inclusive);

	this->inner_input_networks.erase(this->inner_input_networks.begin()+this->explore_index_inclusive+1,
		this->inner_input_networks.begin()+this->explore_end_non_inclusive);
	this->inner_input_sizes.erase(this->inner_input_sizes.begin()+this->explore_index_inclusive+1,
		this->inner_input_sizes.begin()+this->explore_end_non_inclusive);
	this->scope_scale_mod.erase(this->scope_scale_mod.begin()+this->explore_index_inclusive+1,
		this->scope_scale_mod.begin()+this->explore_end_non_inclusive);

	this->step_types.erase(this->step_types.begin()+this->explore_index_inclusive+1,
		this->step_types.begin()+this->explore_end_non_inclusive);
	this->branches.erase(this->branches.begin()+this->explore_index_inclusive+1,
		this->branches.begin()+this->explore_end_non_inclusive);
	this->folds.erase(this->folds.begin()+this->explore_index_inclusive+1,
		this->folds.begin()+this->explore_end_non_inclusive);

	this->score_networks.erase(this->score_networks.begin()+this->explore_index_inclusive+1,
		this->score_networks.begin()+this->explore_end_non_inclusive);

	this->average_misguesses.erase(this->average_misguesses.begin()+this->explore_index_inclusive+1,
		this->average_misguesses.begin()+this->explore_end_non_inclusive);
	this->average_inner_scope_impacts.erase(this->average_inner_scope_impacts.begin()+this->explore_index_inclusive+1,
		this->average_inner_scope_impacts.begin()+this->explore_end_non_inclusive);
	this->average_local_impacts.erase(this->average_local_impacts.begin()+this->explore_index_inclusive+1,
		this->average_local_impacts.begin()+this->explore_end_non_inclusive);
	this->average_inner_branch_impacts.erase(this->average_inner_branch_impacts.begin()+this->explore_index_inclusive+1,
		this->average_inner_branch_impacts.begin()+this->explore_end_non_inclusive);

	this->active_compress.erase(this->active_compress.begin()+this->explore_index_inclusive+1,
		this->active_compress.begin()+this->explore_end_non_inclusive);
	this->compress_new_sizes.erase(this->compress_new_sizes.begin()+this->explore_index_inclusive+1,
		this->compress_new_sizes.begin()+this->explore_end_non_inclusive);
	this->compress_networks.erase(this->compress_networks.begin()+this->explore_index_inclusive+1,
		this->compress_networks.begin()+this->explore_end_non_inclusive);
	this->compress_original_sizes.erase(this->compress_original_sizes.begin()+this->explore_index_inclusive+1,
		this->compress_original_sizes.begin()+this->explore_end_non_inclusive);

	this->explore_index_inclusive = -1;
	this->explore_type = EXPLORE_TYPE_NONE;
	this->explore_end_non_inclusive = -1;
	this->explore_fold = NULL;
}

void BranchPath::resolve_fold(int a_index) {

}
