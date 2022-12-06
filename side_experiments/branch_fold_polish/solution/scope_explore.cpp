#include "scope.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"

using namespace std;

void Scope::explore_replace() {
	if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_STEP) {
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

	for (int a_index = this->explore_index_inclusive+1; a_index < this->explore_end_non_inclusive; a_index++) {
		if (this->is_inner_scope[a_index]) {
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				delete this->inner_input_networks[a_index][i_index];
			}

			delete this->scopes[a_index];
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			delete this->score_networks[a_index];

			if (this->active_compress[a_index]) {
				delete this->compress_networks[a_index];
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

	this->average_misguesses.erase(this->average_misguesses.begin()+this->explore_index_inclusive+1,
		this->average_misguesses.begin()+this->explore_end_non_inclusive);

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

void Scope::explore_branch() {
	if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_STEP) {
		vector<FoldNetwork*> new_score_networks;
		new_score_networks.push_back(this->score_networks[this->explore_index_inclusive]);
		this->score_networks[this->explore_index_inclusive] = NULL;
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
		

	} else if (this->step_types[this->explore_index_inclusive] == STEP_TYPE_BRANCH) {

	} else {

	}
}
