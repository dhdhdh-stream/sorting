#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::score_end() {
	this->curr_compress_num_layers = 0;
	this->curr_compress_new_size = -1;
	this->curr_compress_original_size = -1;

	int sum_scope_sizes = 0;
	for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
		sum_scope_sizes += this->curr_scope_sizes[sc_index];
	}
	// sum_scope_sizes > 0

	this->test_s_input_sizes = this->curr_s_input_sizes;
	this->test_scope_sizes = this->curr_scope_sizes;

	this->test_compress_num_layers = (int)this->curr_scope_sizes.size();
	this->test_compress_new_size = sum_scope_sizes-1;
	this->test_compress_original_size = sum_scope_sizes;

	this->test_compress_network = new FoldNetwork(this->test_compress_new_size,
												  this->curr_s_input_sizes[0],
												  this->curr_scope_sizes,
												  20);

	this->test_fold = new FoldNetwork(this->curr_fold);
	for (int f_index = this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->test_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
		}
	}
	this->test_end_fold = new FoldNetwork(this->curr_end_fold);

	this->test_compressed_s_input_sizes = vector<int>(this->test_compress_num_layers);
	this->test_compressed_scope_sizes = vector<int>(this->test_compress_num_layers);
	for (int sc_index = this->test_compress_num_layers-1; sc_index >= 1; sc_index--) {
		this->test_compressed_s_input_sizes[sc_index] = this->test_s_input_sizes.back();
		this->test_s_input_sizes.pop_back();

		this->test_compressed_scope_sizes[sc_index] = this->test_scope_sizes.back();
		this->test_scope_sizes.pop_back();

		this->test_fold->pop_scope();
		for (int f_index = this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->existing_actions[f_index] != NULL) {
				this->test_input_folds[f_index]->pop_scope();
			}
		}
		this->test_end_fold->pop_scope();
	}
	this->test_compressed_s_input_sizes[0] = this->test_s_input_sizes.back();
	// don't pop last test_s_input_sizes layer
	this->test_compressed_scope_sizes[0] = this->test_scope_sizes.back();
	this->test_scope_sizes.back() = this->test_compress_new_size;	// can be 0
	this->test_fold->pop_scope();
	this->test_fold->add_scope(this->test_compress_new_size);
	for (int f_index = this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->test_input_folds[f_index]->pop_scope();
			this->test_input_folds[f_index]->add_scope(this->test_compress_new_size);
		}
	}
	this->test_end_fold->pop_scope();
	this->test_end_fold->add_scope(this->test_compress_new_size);
	// s_input stays the same

	this->last_state = STATE_SCORE;
	this->state = STATE_COMPRESS_STATE;
	this->state_iter = 0;
	this->sum_error = 0.0;
	this->new_state_factor = 25;
}
