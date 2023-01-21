#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::flat_to_fold() {
	this->starting_average_score = 0.0;
	this->starting_average_misguess = 0.0;
	this->starting_average_local_impact = 0.0;

	if (this->starting_compress_original_size == 0) {
		// outer start edge case

		// if this->existing_actions[0] != NULL, this->curr_scope_sizes.size() == 1, so skip STATE_INNER_SCOPE_INPUT

		if (this->is_existing[0]) {
			this->curr_input_network = this->curr_input_folds[0];
			this->curr_input_folds[0] = NULL;
		}

		if (!this->is_existing[0]) {
			this->curr_s_input_sizes.push_back(0);
			// obs_size always 1 for sorting
			this->curr_scope_sizes.push_back(1);
		} else {
			this->curr_s_input_sizes.push_back(this->existing_actions[0]->num_inputs);
			this->curr_scope_sizes.push_back(this->existing_actions[0]->num_outputs);
		}

		this->curr_fold->add_scope(this->curr_scope_sizes.back());
		this->curr_fold->fold_index++;
		this->curr_fold->migrate_weights();	// TODO: migrate from fold_index to last_state for all hidden layer
		for (int f_index = 1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				this->curr_input_folds[f_index]->add_scope(this->curr_scope_sizes.back());
				this->curr_input_folds[f_index]->fold_index++;
				this->curr_input_folds[f_index]->migrate_weights();
			}
		}
		this->curr_end_fold->add_scope(this->curr_scope_sizes.back());
		this->curr_end_fold->fold_index++;
		this->curr_end_fold->migrate_weights();

		this->curr_score_network = new FoldNetwork(1,
												   this->curr_s_input_sizes[0],
												   this->curr_scope_sizes,
												   20);

		cout << "ending FLAT" << endl;
		cout << "beginning STATE_SCORE" << endl;

		this->last_state = STATE_SCORE;	// for STATE_SCORE, also set last_state to STATE_SCORE
		this->state = STATE_SCORE;
		this->state_iter = 0;
		this->sum_error = 0.0;
	} else {
		// this->starting_compress_original_size == this->curr_starting_compress_new_size
		this->test_starting_compress_new_size = this->curr_starting_compress_new_size-1;

		// no change to this->curr_s_input_sizes
		this->test_scope_sizes = this->curr_scope_sizes;
		this->test_scope_sizes.pop_back();
		this->test_scope_sizes.push_back(this->test_starting_compress_new_size);

		if (this->test_starting_compress_new_size > 0) {
			this->test_starting_compress_network = new FoldNetwork(
				this->test_starting_compress_new_size,
				this->curr_s_input_sizes[0],	// i.e., starting_s_input_size
				vector<int>{this->curr_scope_sizes[0]},		// i.e., starting_state_size
				20);
		}

		this->test_fold = new FoldNetwork(this->curr_fold);
		this->test_fold->pop_scope();
		// this->test_starting_compress_new_size can be 0
		this->test_fold->add_scope(this->test_starting_compress_new_size);
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				this->test_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
				this->test_input_folds[f_index]->pop_scope();
				this->test_input_folds[f_index]->add_scope(this->test_starting_compress_new_size);
			}
		}
		this->test_end_fold = new FoldNetwork(this->curr_end_fold);
		this->test_end_fold->pop_scope();
		this->test_end_fold->add_scope(this->test_starting_compress_new_size);

		cout << "ending FLAT" << endl;
		cout << "beginning STATE_STARTING_COMPRESS" << endl;

		this->last_state = STATE_STEP_ADDED;
		this->state = STATE_STARTING_COMPRESS;
		this->state_iter = 0;
		this->sum_error = 0.0;
	}
}
