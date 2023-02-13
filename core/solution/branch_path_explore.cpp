#include "branch_path.h"

#include <iostream>

#include "fold_to_path.h"
#include "globals.h"

using namespace std;

void BranchPath::explore_replace() {
	cout << "BranchPath explore_replace" << endl;
	cout << "new_fold:";
	for (int s_index = 0; s_index < this->explore_fold->sequence_length; s_index++) {
		if (this->explore_fold->is_inner_scope[s_index]) {
			cout << " S_" << this->explore_fold->existing_scope_ids[s_index];
		} else {
			cout << " " << this->explore_fold->actions[s_index].to_string();
		}
	}
	cout << endl;

	if (this->step_types[this->best_explore_index_inclusive] == STEP_TYPE_STEP) {
		// can't be scope end

		if (this->best_explore_index_inclusive != 0) {
			delete this->score_networks[this->best_explore_index_inclusive];
			this->score_networks[this->best_explore_index_inclusive] = NULL;
		}

		if (this->active_compress[this->best_explore_index_inclusive]) {
			delete this->compress_networks[this->best_explore_index_inclusive];
			this->compress_networks[this->best_explore_index_inclusive] = NULL;
		}
	} else if (this->step_types[this->best_explore_index_inclusive] == STEP_TYPE_BRANCH) {
		delete this->branches[this->best_explore_index_inclusive];
		this->branches[this->best_explore_index_inclusive] = NULL;
	}
	// this->step_types[this->best_explore_index_inclusive] != STEP_TYPE_FOLD

	this->step_types[this->best_explore_index_inclusive] = STEP_TYPE_FOLD;

	if (this->best_explore_index_inclusive == 0) {
		delete this->parent->score_networks[this->parent_index];
		this->parent->score_networks[this->parent_index] = this->explore_fold->starting_score_network;
	} else {
		this->score_networks[this->best_explore_index_inclusive] = this->explore_fold->starting_score_network;
	}
	this->explore_fold->starting_score_network = NULL;
	delete this->explore_fold->combined_score_network;
	this->explore_fold->combined_score_network = NULL;

	this->folds[this->best_explore_index_inclusive] = this->explore_fold;

	// this->average_inner_scope_impacts[this->best_explore_index_inclusive] unchanged
	this->average_local_impacts[this->best_explore_index_inclusive] = 0.0;	// no longer matters
	this->average_inner_branch_impacts[this->best_explore_index_inclusive] = 0.0;	// initialize to 0.0

	for (int a_index = this->best_explore_index_inclusive+1; a_index < this->best_explore_end_non_inclusive; a_index++) {
		if (this->is_inner_scope[a_index]) {
			for (int i_index = 0; i_index < (int)this->inner_input_networks[a_index].size(); i_index++) {
				delete this->inner_input_networks[a_index][i_index];
			}
			delete this->scope_scale_mod[a_index];

			// scopes owned and deleted by solution
		}

		if (this->step_types[a_index] == STEP_TYPE_STEP) {
			if (a_index == (int)this->scopes.size()-1 && !this->full_last) {
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

	this->sequence_length = this->sequence_length - (this->best_explore_end_non_inclusive-this->best_explore_index_inclusive) + 1;
	this->is_inner_scope.erase(this->is_inner_scope.begin()+this->best_explore_index_inclusive+1,
		this->is_inner_scope.begin()+this->best_explore_end_non_inclusive);
	this->scopes.erase(this->scopes.begin()+this->best_explore_index_inclusive+1,
		this->scopes.begin()+this->best_explore_end_non_inclusive);
	this->actions.erase(this->actions.begin()+this->best_explore_index_inclusive+1,
		this->actions.begin()+this->best_explore_end_non_inclusive);

	this->inner_input_networks.erase(this->inner_input_networks.begin()+this->best_explore_index_inclusive+1,
		this->inner_input_networks.begin()+this->best_explore_end_non_inclusive);
	this->inner_input_sizes.erase(this->inner_input_sizes.begin()+this->best_explore_index_inclusive+1,
		this->inner_input_sizes.begin()+this->best_explore_end_non_inclusive);
	this->scope_scale_mod.erase(this->scope_scale_mod.begin()+this->best_explore_index_inclusive+1,
		this->scope_scale_mod.begin()+this->best_explore_end_non_inclusive);

	this->step_types.erase(this->step_types.begin()+this->best_explore_index_inclusive+1,
		this->step_types.begin()+this->best_explore_end_non_inclusive);
	this->branches.erase(this->branches.begin()+this->best_explore_index_inclusive+1,
		this->branches.begin()+this->best_explore_end_non_inclusive);
	this->folds.erase(this->folds.begin()+this->best_explore_index_inclusive+1,
		this->folds.begin()+this->best_explore_end_non_inclusive);

	this->score_networks.erase(this->score_networks.begin()+this->best_explore_index_inclusive+1,
		this->score_networks.begin()+this->best_explore_end_non_inclusive);

	this->average_inner_scope_impacts.erase(this->average_inner_scope_impacts.begin()+this->best_explore_index_inclusive+1,
		this->average_inner_scope_impacts.begin()+this->best_explore_end_non_inclusive);
	this->average_local_impacts.erase(this->average_local_impacts.begin()+this->best_explore_index_inclusive+1,
		this->average_local_impacts.begin()+this->best_explore_end_non_inclusive);
	this->average_inner_branch_impacts.erase(this->average_inner_branch_impacts.begin()+this->best_explore_index_inclusive+1,
		this->average_inner_branch_impacts.begin()+this->best_explore_end_non_inclusive);

	this->active_compress.erase(this->active_compress.begin()+this->best_explore_index_inclusive+1,
		this->active_compress.begin()+this->best_explore_end_non_inclusive);
	this->compress_new_sizes.erase(this->compress_new_sizes.begin()+this->best_explore_index_inclusive+1,
		this->compress_new_sizes.begin()+this->best_explore_end_non_inclusive);
	this->compress_networks.erase(this->compress_networks.begin()+this->best_explore_index_inclusive+1,
		this->compress_networks.begin()+this->best_explore_end_non_inclusive);
	this->compress_original_sizes.erase(this->compress_original_sizes.begin()+this->best_explore_index_inclusive+1,
		this->compress_original_sizes.begin()+this->best_explore_end_non_inclusive);

	this->starting_state_sizes.erase(this->starting_state_sizes.begin()+this->best_explore_index_inclusive+1,
		this->starting_state_sizes.begin()+this->best_explore_end_non_inclusive);

	if (this->explore_fold->state == STATE_DONE) {
		// sequence length 0 edge case
		vector<Fold*> folds_to_delete;
		resolve_fold(this->best_explore_index_inclusive,
					 folds_to_delete);
		delete folds_to_delete[0];
	}

	this->explore_curr_try = 0;
	this->explore_target_tries = 1;
	int rand_scale = rand()%4;
	for (int i = 0; i < rand_scale; i++) {
		this->explore_target_tries *= 10;
	}
	this->best_explore_surprise = 0.0;
	this->explore_fold = NULL;
}

void BranchPath::explore_branch() {
	cout << "BranchPath explore_branch" << endl;
	cout << "new_fold:";
	for (int s_index = 0; s_index < this->explore_fold->sequence_length; s_index++) {
		if (this->explore_fold->is_inner_scope[s_index]) {
			cout << " S_" << this->explore_fold->existing_scope_ids[s_index];
		} else {
			cout << " " << this->explore_fold->actions[s_index].to_string();
		}
	}
	cout << endl;

	if (this->step_types[this->best_explore_index_inclusive] == STEP_TYPE_BRANCH
			&& this->best_explore_index_inclusive+1 == this->best_explore_end_non_inclusive) {
		// this->branches[this->best_explore_index_inclusive]->passed_branch_score == false
		delete this->branches[this->best_explore_index_inclusive]->combined_score_network;
		this->branches[this->best_explore_index_inclusive]->combined_score_network = this->explore_fold->combined_score_network;
		this->explore_fold->combined_score_network = NULL;

		this->branches[this->best_explore_index_inclusive]->score_networks.push_back(
			this->explore_fold->starting_score_network);
		this->explore_fold->starting_score_network = NULL;
		this->branches[this->best_explore_index_inclusive]->is_branch.push_back(false);
		this->branches[this->best_explore_index_inclusive]->branches.push_back(NULL);
		this->branches[this->best_explore_index_inclusive]->folds.push_back(this->explore_fold);
		this->branches[this->best_explore_index_inclusive]->num_travelled.push_back(0);
	} else {
		int new_num_inputs = this->starting_state_sizes[this->best_explore_index_inclusive];
		if (this->best_explore_index_inclusive != 0) {
			if (!this->is_inner_scope[this->best_explore_index_inclusive]) {
				// obs_size always 1 for sorting
				new_num_inputs++;
			} else {
				new_num_inputs += this->scopes[this->best_explore_index_inclusive]->num_outputs;
			}
		}
		int new_num_outputs;
		if (this->best_explore_end_non_inclusive == this->sequence_length) {
			new_num_outputs = this->num_outputs;
		} else {
			new_num_outputs = this->starting_state_sizes[this->best_explore_end_non_inclusive];
		}

		vector<bool> branch_is_inner_scope;
		branch_is_inner_scope.push_back(false);	// start doesn't matter
		branch_is_inner_scope.insert(branch_is_inner_scope.end(),
			this->is_inner_scope.begin()+this->best_explore_index_inclusive+1,
			this->is_inner_scope.begin()+this->best_explore_end_non_inclusive);
		vector<Scope*> branch_scopes;
		branch_scopes.push_back(NULL);	// start doesn't matter
		branch_scopes.insert(branch_scopes.end(),
			this->scopes.begin()+this->best_explore_index_inclusive+1,
			this->scopes.begin()+this->best_explore_end_non_inclusive);
		vector<Action> branch_actions;
		branch_actions.push_back(Action());	// start doesn't matter
		branch_actions.insert(branch_actions.end(),
			this->actions.begin()+this->best_explore_index_inclusive+1,
			this->actions.begin()+this->best_explore_end_non_inclusive);

		vector<vector<FoldNetwork*>> branch_inner_input_networks;
		branch_inner_input_networks.push_back(vector<FoldNetwork*>());	// start doesn't matter
		branch_inner_input_networks.insert(branch_inner_input_networks.end(),
			this->inner_input_networks.begin()+this->best_explore_index_inclusive+1,
			this->inner_input_networks.begin()+this->best_explore_end_non_inclusive);
		vector<vector<int>> branch_inner_input_sizes;
		branch_inner_input_sizes.push_back(vector<int>());	// start doesn't matter
		branch_inner_input_sizes.insert(branch_inner_input_sizes.end(),
			this->inner_input_sizes.begin()+this->best_explore_index_inclusive+1,
			this->inner_input_sizes.begin()+this->best_explore_end_non_inclusive);
		vector<Network*> branch_scope_scale_mod;
		branch_scope_scale_mod.push_back(NULL);	// start doesn't matter
		branch_scope_scale_mod.insert(branch_scope_scale_mod.end(),
			this->scope_scale_mod.begin()+this->best_explore_index_inclusive+1,
			this->scope_scale_mod.begin()+this->best_explore_end_non_inclusive);

		vector<int> branch_step_types(this->step_types.begin()+this->best_explore_index_inclusive,
			this->step_types.begin()+this->best_explore_end_non_inclusive);
		// set this->step_types[this->best_explore_index_inclusive] below
		vector<Branch*> branch_branches(this->branches.begin()+this->best_explore_index_inclusive,
			this->branches.begin()+this->best_explore_end_non_inclusive);
		// set this->branches[this->best_explore_index_inclusive] below
		vector<Fold*> branch_folds(this->folds.begin()+this->best_explore_index_inclusive,
			this->folds.begin()+this->best_explore_end_non_inclusive);
		this->folds[this->best_explore_index_inclusive] = NULL;

		vector<FoldNetwork*> branch_score_networks;
		branch_score_networks.push_back(NULL);	// start doesn't matter
		branch_score_networks.insert(branch_score_networks.end(),
			this->score_networks.begin()+this->best_explore_index_inclusive+1,
			this->score_networks.begin()+this->best_explore_end_non_inclusive);

		vector<double> branch_average_inner_scope_impacts;
		branch_average_inner_scope_impacts.push_back(0.0);	// start doesn't matter
		branch_average_inner_scope_impacts.insert(branch_average_inner_scope_impacts.end(),
			this->average_inner_scope_impacts.begin()+this->best_explore_index_inclusive+1,
			this->average_inner_scope_impacts.begin()+this->best_explore_end_non_inclusive);
		// this->average_inner_scope_impacts[this->best_explore_index_inclusive] unchanged
		vector<double> branch_average_local_impacts(this->average_local_impacts.begin()+this->best_explore_index_inclusive,
			this->average_local_impacts.begin()+this->best_explore_end_non_inclusive);
		this->average_local_impacts[this->best_explore_index_inclusive] = 0.0;	// no longer matters
		vector<double> branch_average_inner_branch_impacts(this->average_inner_branch_impacts.begin()+this->best_explore_index_inclusive,
			this->average_inner_branch_impacts.begin()+this->best_explore_end_non_inclusive);
		this->average_inner_branch_impacts[this->best_explore_index_inclusive] = 0.0;	// initialize to 0.0

		vector<bool> branch_active_compress(this->active_compress.begin()+this->best_explore_index_inclusive,
			this->active_compress.begin()+this->best_explore_end_non_inclusive);
		this->active_compress[this->best_explore_index_inclusive] = false;
		vector<int> branch_compress_new_sizes(this->compress_new_sizes.begin()+this->best_explore_index_inclusive,
			this->compress_new_sizes.begin()+this->best_explore_end_non_inclusive);
		this->compress_new_sizes[this->best_explore_index_inclusive] = -1;
		vector<FoldNetwork*> branch_compress_networks(this->compress_networks.begin()+this->best_explore_index_inclusive,
			this->compress_networks.begin()+this->best_explore_end_non_inclusive);
		this->compress_networks[this->best_explore_index_inclusive] = NULL;
		vector<int> branch_compress_original_sizes(this->compress_original_sizes.begin()+this->best_explore_index_inclusive,
			this->compress_original_sizes.begin()+this->best_explore_end_non_inclusive);
		this->compress_original_sizes[this->best_explore_index_inclusive] = -1;

		bool branch_full_last;
		if (this->best_explore_end_non_inclusive == (int)this->scopes.size() && !this->full_last) {
			branch_full_last = false;
		} else {
			branch_full_last = true;
		}

		BranchPath* new_branch_path = new BranchPath(new_num_inputs,
													 new_num_outputs,
													 this->outer_s_input_size,	// differs from scope
													 this->best_explore_end_non_inclusive-this->best_explore_index_inclusive,
													 branch_is_inner_scope,
													 branch_scopes,
													 branch_actions,
													 branch_inner_input_networks,
													 branch_inner_input_sizes,
													 branch_scope_scale_mod,
													 branch_step_types,
													 branch_branches,
													 branch_folds,
													 branch_score_networks,
													 branch_average_inner_scope_impacts,
													 branch_average_local_impacts,
													 branch_average_inner_branch_impacts,
													 this->average_score,
													 this->score_variance,
													 this->average_misguess,
													 this->misguess_variance,
													 branch_active_compress,
													 branch_compress_new_sizes,
													 branch_compress_networks,
													 branch_compress_original_sizes,
													 branch_full_last);

		FoldNetwork* new_combined_score_network;
		bool new_passed_combined;
		vector<FoldNetwork*> new_score_networks;
		if (this->best_explore_index_inclusive == 0) {
			new_score_networks.push_back(this->parent->score_networks[this->parent_index]);

			this->parent->score_networks[this->parent_index] = this->explore_fold->combined_score_network;
			this->explore_fold->combined_score_network = NULL;

			new_combined_score_network = NULL;
			new_passed_combined = true;
		} else if (this->step_types[this->best_explore_index_inclusive] == STEP_TYPE_BRANCH) {
			new_combined_score_network = this->explore_fold->combined_score_network;
			this->explore_fold->combined_score_network = NULL;
			new_passed_combined = false;

			new_score_networks.push_back(this->branches[this->best_explore_index_inclusive]->combined_score_network);
			this->branches[this->best_explore_index_inclusive]->combined_score_network = NULL;
			this->branches[this->best_explore_index_inclusive]->passed_combined = true;
		} else {
			new_combined_score_network = this->explore_fold->combined_score_network;
			this->explore_fold->combined_score_network = NULL;
			new_passed_combined = false;

			new_score_networks.push_back(this->score_networks[this->best_explore_index_inclusive]);
			this->score_networks[this->best_explore_index_inclusive] = NULL;
		}
		new_score_networks.push_back(this->explore_fold->starting_score_network);
		this->explore_fold->starting_score_network = NULL;

		vector<bool> new_is_branch;
		new_is_branch.push_back(true);
		new_is_branch.push_back(false);

		vector<BranchPath*> new_branches;
		new_branches.push_back(new_branch_path);
		new_branches.push_back(NULL);

		vector<Fold*> new_folds;
		new_folds.push_back(NULL);
		new_folds.push_back(this->explore_fold);

		vector<int> new_num_travelled;
		new_num_travelled.push_back(100000);
		new_num_travelled.push_back(0);

		Branch* new_branch = new Branch(new_num_inputs,
										new_num_outputs,
										this->outer_s_input_size,	// differs from scope
										new_combined_score_network,
										new_passed_combined,
										new_score_networks,
										new_is_branch,
										new_branches,
										new_folds,
										new_num_travelled);

		if (this->explore_fold->state == STATE_DONE) {
			// sequence length 0 edge case
			vector<Fold*> folds_to_delete;
			new_branch->resolve_fold(1, folds_to_delete);
			delete folds_to_delete[0];
		}

		this->step_types[this->best_explore_index_inclusive] = STEP_TYPE_BRANCH;
		this->branches[this->best_explore_index_inclusive] = new_branch;
	}

	this->sequence_length = this->sequence_length - (this->best_explore_end_non_inclusive-this->best_explore_index_inclusive) + 1;
	this->is_inner_scope.erase(this->is_inner_scope.begin()+this->best_explore_index_inclusive+1,
		this->is_inner_scope.begin()+this->best_explore_end_non_inclusive);
	this->scopes.erase(this->scopes.begin()+this->best_explore_index_inclusive+1,
		this->scopes.begin()+this->best_explore_end_non_inclusive);
	this->actions.erase(this->actions.begin()+this->best_explore_index_inclusive+1,
		this->actions.begin()+this->best_explore_end_non_inclusive);

	this->inner_input_networks.erase(this->inner_input_networks.begin()+this->best_explore_index_inclusive+1,
		this->inner_input_networks.begin()+this->best_explore_end_non_inclusive);
	this->inner_input_sizes.erase(this->inner_input_sizes.begin()+this->best_explore_index_inclusive+1,
		this->inner_input_sizes.begin()+this->best_explore_end_non_inclusive);
	this->scope_scale_mod.erase(this->scope_scale_mod.begin()+this->best_explore_index_inclusive+1,
		this->scope_scale_mod.begin()+this->best_explore_end_non_inclusive);

	this->step_types.erase(this->step_types.begin()+this->best_explore_index_inclusive+1,
		this->step_types.begin()+this->best_explore_end_non_inclusive);
	this->branches.erase(this->branches.begin()+this->best_explore_index_inclusive+1,
		this->branches.begin()+this->best_explore_end_non_inclusive);
	this->folds.erase(this->folds.begin()+this->best_explore_index_inclusive+1,
		this->folds.begin()+this->best_explore_end_non_inclusive);

	this->score_networks.erase(this->score_networks.begin()+this->best_explore_index_inclusive+1,
		this->score_networks.begin()+this->best_explore_end_non_inclusive);

	this->average_inner_scope_impacts.erase(this->average_inner_scope_impacts.begin()+this->best_explore_index_inclusive+1,
		this->average_inner_scope_impacts.begin()+this->best_explore_end_non_inclusive);
	this->average_local_impacts.erase(this->average_local_impacts.begin()+this->best_explore_index_inclusive+1,
		this->average_local_impacts.begin()+this->best_explore_end_non_inclusive);
	this->average_inner_branch_impacts.erase(this->average_inner_branch_impacts.begin()+this->best_explore_index_inclusive+1,
		this->average_inner_branch_impacts.begin()+this->best_explore_end_non_inclusive);

	this->active_compress.erase(this->active_compress.begin()+this->best_explore_index_inclusive+1,
		this->active_compress.begin()+this->best_explore_end_non_inclusive);
	this->compress_new_sizes.erase(this->compress_new_sizes.begin()+this->best_explore_index_inclusive+1,
		this->compress_new_sizes.begin()+this->best_explore_end_non_inclusive);
	this->compress_networks.erase(this->compress_networks.begin()+this->best_explore_index_inclusive+1,
		this->compress_networks.begin()+this->best_explore_end_non_inclusive);
	this->compress_original_sizes.erase(this->compress_original_sizes.begin()+this->best_explore_index_inclusive+1,
		this->compress_original_sizes.begin()+this->best_explore_end_non_inclusive);

	this->starting_state_sizes.erase(this->starting_state_sizes.begin()+this->best_explore_index_inclusive+1,
		this->starting_state_sizes.begin()+this->best_explore_end_non_inclusive);

	this->explore_curr_try = 0;
	this->explore_target_tries = 1;
	int rand_scale = rand()%4;
	for (int i = 0; i < rand_scale; i++) {
		this->explore_target_tries *= 10;
	}
	this->best_explore_surprise = 0.0;
	this->explore_fold = NULL;
}

void BranchPath::resolve_fold(int a_index,
							  vector<Fold*>& folds_to_delete) {
	cout << "BranchPath resolve_fold" << endl;

	int new_sequence_length;
	vector<bool> new_is_inner_scope;
	vector<Scope*> new_scopes;
	vector<Action> new_actions;
	vector<vector<FoldNetwork*>> new_inner_input_networks;
	vector<vector<int>> new_inner_input_sizes;
	vector<Network*> new_scope_scale_mod;
	vector<int> new_step_types;
	vector<Branch*> new_branches;
	vector<Fold*> new_folds;
	vector<FoldNetwork*> new_score_networks;
	vector<double> new_average_inner_scope_impacts;
	vector<double> new_average_local_impacts;
	vector<double> new_average_inner_branch_impacts;
	vector<bool> new_active_compress;
	vector<int> new_compress_new_sizes;
	vector<FoldNetwork*> new_compress_networks;
	vector<int> new_compress_original_sizes;
	fold_to_path(this->folds[a_index]->finished_steps,
				 this->folds[a_index]->average_score,
				 this->folds[a_index]->score_variance,
				 this->folds[a_index]->average_misguess,
				 this->folds[a_index]->misguess_variance,
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
				 new_average_inner_scope_impacts,
				 new_average_local_impacts,
				 new_average_inner_branch_impacts,
				 new_active_compress,
				 new_compress_new_sizes,
				 new_compress_networks,
				 new_compress_original_sizes);

	// this->score_networks[a_index] already set correctly

	// this->average_inner_scope_impacts[a_index] unchanged
	this->average_local_impacts[a_index] = this->folds[a_index]->starting_average_local_impact;
	// this->average_inner_branch_impacts[a_index] doesn't matter and unchanged

	if (this->folds[a_index]->curr_starting_compress_new_size != this->folds[a_index]->starting_compress_original_size
			&& this->folds[a_index]->curr_starting_compress_new_size > 0) {
		this->active_compress[a_index] = true;
	} else {
		this->active_compress[a_index] = false;
	}
	this->compress_new_sizes[a_index] = this->folds[a_index]->curr_starting_compress_new_size;
	this->compress_networks[a_index] = this->folds[a_index]->curr_starting_compress_network;
	this->folds[a_index]->curr_starting_compress_network = NULL;
	this->compress_original_sizes[a_index] = this->folds[a_index]->starting_compress_original_size;

	if (a_index == this->sequence_length-1) {
		this->full_last = true;
	}

	this->sequence_length += new_sequence_length;
	this->is_inner_scope.insert(this->is_inner_scope.begin()+a_index+1,
		new_is_inner_scope.begin(), new_is_inner_scope.end());
	this->scopes.insert(this->scopes.begin()+a_index+1,
		new_scopes.begin(), new_scopes.end());
	this->actions.insert(this->actions.begin()+a_index+1,
		new_actions.begin(), new_actions.end());

	this->inner_input_networks.insert(this->inner_input_networks.begin()+a_index+1,
		new_inner_input_networks.begin(), new_inner_input_networks.end());
	this->inner_input_sizes.insert(this->inner_input_sizes.begin()+a_index+1,
		new_inner_input_sizes.begin(), new_inner_input_sizes.end());
	this->scope_scale_mod.insert(this->scope_scale_mod.begin()+a_index+1,
		new_scope_scale_mod.begin(), new_scope_scale_mod.end());

	this->step_types.insert(this->step_types.begin()+a_index+1,
		new_step_types.begin(), new_step_types.end());
	this->branches.insert(this->branches.begin()+a_index+1,
		new_branches.begin(), new_branches.end());
	this->folds.insert(this->folds.begin()+a_index+1,
		new_folds.begin(), new_folds.end());

	this->score_networks.insert(this->score_networks.begin()+a_index+1,
		new_score_networks.begin(), new_score_networks.end());

	this->average_inner_scope_impacts.insert(this->average_inner_scope_impacts.begin()+a_index+1,
		new_average_inner_scope_impacts.begin(), new_average_inner_scope_impacts.end());
	this->average_local_impacts.insert(this->average_local_impacts.begin()+a_index+1,
		new_average_local_impacts.begin(), new_average_local_impacts.end());
	this->average_inner_branch_impacts.insert(this->average_inner_branch_impacts.begin()+a_index+1,
		new_average_inner_branch_impacts.begin(), new_average_inner_branch_impacts.end());

	this->active_compress.insert(this->active_compress.begin()+a_index+1,
		new_active_compress.begin(), new_active_compress.end());
	this->compress_new_sizes.insert(this->compress_new_sizes.begin()+a_index+1,
		new_compress_new_sizes.begin(), new_compress_new_sizes.end());
	this->compress_networks.insert(this->compress_networks.begin()+a_index+1,
		new_compress_networks.begin(), new_compress_networks.end());
	this->compress_original_sizes.insert(this->compress_original_sizes.begin()+a_index+1,
		new_compress_original_sizes.begin(), new_compress_original_sizes.end());

	int state_size = this->folds[a_index]->curr_starting_compress_new_size;
	vector<int> new_starting_state_sizes;
	for (int n_index = 0; n_index < new_sequence_length; n_index++) {
		new_starting_state_sizes.push_back(state_size);

		if (!new_is_inner_scope[n_index]) {
			// obs_size always 1 for sorting
			state_size++;
		} else {
			state_size += new_scopes[n_index]->num_outputs;
		}

		if (new_step_types[n_index] == STEP_TYPE_STEP) {
			if (new_active_compress[n_index]) {
				state_size = new_compress_new_sizes[n_index];
			} else {
				int compress_size = new_compress_original_sizes[n_index] - new_compress_new_sizes[n_index];
				state_size -= compress_size;
			}
		} else if (new_step_types[n_index] == STEP_TYPE_BRANCH) {
			state_size = new_branches[n_index]->num_outputs;
		} else {
			// new_step_types[n_index] == STEP_TYPE_FOLD
			state_size = new_folds[n_index]->num_outputs;
		}
	}
	this->starting_state_sizes.insert(this->starting_state_sizes.begin()+a_index+1,
		new_starting_state_sizes.begin(), new_starting_state_sizes.end());

	if (this->best_explore_index_inclusive > a_index) {
		this->best_explore_index_inclusive += new_sequence_length;
	}
	if (this->best_explore_end_non_inclusive > a_index) {
		this->best_explore_end_non_inclusive += new_sequence_length;
	}

	folds_to_delete.push_back(this->folds[a_index]);
	this->folds[a_index] = NULL;
	this->step_types[a_index] = STEP_TYPE_STEP;

	// ofstream solution_save_file;
	// solution_save_file.open("saves/solution.txt");
	// solution->save(solution_save_file);
	// solution_save_file.close();
}
