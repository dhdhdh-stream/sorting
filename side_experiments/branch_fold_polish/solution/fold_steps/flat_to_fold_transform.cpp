#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::flat_to_fold() {
	this->starting_average_misguess = 0.0;
	this->starting_average_local_impact = 0.0;

	// this->starting_compress_original_size > 0
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
			this->curr_scope_sizes[0],		// i.e., starting_state_size
			20);
	}

	this->test_fold = new FoldNetwork(this->curr_fold);
	this->test_fold->pop_scope();
	// this->test_starting_compress_new_size can be 0
	this->test_fold->add_scope(this->test_starting_compress_new_size);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->test_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
			this->test_input_folds[f_index]->pop_scope();
			this->test_input_folds[f_index]->add_scope(this->test_starting_compress_new_size);
		}
	}
	this->test_end_fold = new FoldNetwork(this->curr_end_fold);
	this->test_end_fold->pop_scope();
	this->test_end_fold->add_scope(this->test_starting_compress_new_size);

	this->last_state = STATE_STEP_ADDED;
	this->state = STATE_STARTING_COMPRESS;
	this->state_iter = 0;
	this->sum_error = 0.0;
}
