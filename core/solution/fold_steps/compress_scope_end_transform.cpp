#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::compress_scope_end() {
	// if (this->sum_error/10000 < 0.01) {
	if (rand()%2 == 0) {
		delete this->curr_compress_network;	// can't be NULL
		this->curr_compress_network = this->test_compress_network;	// can be NULL
		this->test_compress_network = NULL;

		this->curr_compress_num_layers = this->test_compress_num_layers;
		this->curr_compress_new_size = this->test_compress_new_size;
		this->curr_compress_original_size = this->test_compress_original_size;
		this->curr_compressed_s_input_sizes = this->test_compressed_s_input_sizes;
		this->curr_compressed_scope_sizes = this->test_compressed_scope_sizes;

		this->curr_s_input_sizes = this->test_s_input_sizes;
		this->curr_scope_sizes = this->test_scope_sizes;

		delete this->curr_fold;
		this->curr_fold = this->test_fold;
		this->test_fold = NULL;
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				delete this->curr_input_folds[f_index];
				this->curr_input_folds[f_index] = this->test_input_folds[f_index];
				this->test_input_folds[f_index] = NULL;
			}
		}
		delete this->curr_end_fold;
		this->curr_end_fold = this->test_end_fold;
		this->test_end_fold = NULL;

		int sum_scope_sizes = 0;
		for (int sc_index = 0; sc_index < this->curr_compress_num_layers-1; sc_index++) {
			sum_scope_sizes += this->curr_compressed_scope_sizes[this->curr_compressed_scope_sizes.size()-1-sc_index];
		}
		int compress_size = this->curr_compress_original_size - this->curr_compress_new_size;
		if (compress_size > sum_scope_sizes) {
			// this->curr_scope_sizes > 1

			if (this->curr_s_input_sizes[0] + this->curr_scope_sizes[0] == 0) {
				this->curr_score_network->subfold_index++;
			}
			
			if (this->curr_score_network->subfold_index == (int)this->curr_scope_sizes.size()-2) {
				cout << "COMPRESS_SCOPE success" << endl;
				cout << "ending COMPRESS_SCOPE" << endl;
				cout << "beginning ADD_STEP" << endl;

				add_finished_step();
			} else {
				this->test_score_network = new FoldNetwork(this->curr_score_network);
				this->test_score_network->subfold_index++;

				if (this->curr_compress_num_layers > 0 && this->curr_compress_new_size > 0) {
					if (this->curr_s_input_sizes[0] + this->curr_scope_sizes[0] == 0) {
						this->curr_compress_network->subfold_index++;
					}
					this->test_compress_network = new FoldNetwork(this->curr_compress_network);
					this->test_compress_network->subfold_index++;
				}

				cout << "COMPRESS_SCOPE success" << endl;
				cout << "ending COMPRESS_SCOPE" << endl;
				cout << "beginning STATE_INPUT" << endl;

				this->last_state = STATE_COMPRESS_SCOPE;
				this->state = STATE_INPUT;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->new_state_factor = 25;
			}
		} else if (compress_size == sum_scope_sizes) {
			this->test_compress_num_layers = this->curr_compress_num_layers-1;
			this->test_compress_new_size = 0;
			this->test_compress_original_size = sum_scope_sizes;

			this->test_compressed_s_input_sizes = this->curr_compressed_s_input_sizes;
			this->test_compressed_scope_sizes = this->curr_compressed_scope_sizes;

			this->test_compressed_s_input_sizes.erase(this->test_compressed_s_input_sizes.begin());
			// no change to test_s_input_sizes

			this->test_scope_sizes.pop_back();
			this->test_scope_sizes.push_back(this->test_compressed_scope_sizes[0]);

			this->test_fold = new FoldNetwork(this->curr_fold);
			this->test_fold->pop_scope();
			this->test_fold->add_scope(this->test_compressed_scope_sizes[0]);
			for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
				if (this->is_existing[f_index]) {
					this->test_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
					this->test_input_folds[f_index]->pop_scope();
					this->test_input_folds[f_index]->add_scope(this->test_compressed_scope_sizes[0]);
				}
			}
			this->test_end_fold = new FoldNetwork(this->curr_end_fold);
			this->test_end_fold->pop_scope();
			this->test_end_fold->add_scope(this->test_compressed_scope_sizes[0]);

			this->test_compressed_scope_sizes.erase(this->test_compressed_scope_sizes.begin());

			cout << "COMPRESS_SCOPE success" << endl;
			cout << "ending COMPRESS_SCOPE" << endl;
			cout << "beginning COMPRESS_SCOPE" << endl;

			this->last_state = STATE_COMPRESS_SCOPE;
			this->state = STATE_COMPRESS_SCOPE;
			this->state_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;	// though doesn't matter
		} else {
			this->test_compress_num_layers = this->curr_compress_num_layers-1;
			this->test_compress_new_size = sum_scope_sizes - compress_size;
			this->test_compress_original_size = sum_scope_sizes;

			this->test_compressed_s_input_sizes = this->curr_compressed_s_input_sizes;
			this->test_compressed_scope_sizes = this->curr_compressed_scope_sizes;

			this->test_compressed_s_input_sizes.erase(this->test_compressed_s_input_sizes.begin());
			this->test_s_input_sizes.push_back(this->test_compressed_s_input_sizes[0]);

			this->test_scope_sizes.pop_back();
			this->test_scope_sizes.push_back(this->test_compressed_scope_sizes[0]);
			this->test_scope_sizes.push_back(this->test_compress_new_size);

			// copy s_input_sizes and scope_sizes from curr_score_network
			this->test_compress_network = new FoldNetwork(this->test_compress_new_size,
														  this->curr_score_network->s_input_size,
														  this->curr_score_network->scope_sizes,
														  20);

			this->test_fold = new FoldNetwork(this->curr_fold);
			this->test_fold->pop_scope();
			this->test_fold->add_scope(this->test_compressed_scope_sizes[0]);
			this->test_fold->add_scope(this->test_compress_new_size);
			for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
				if (this->is_existing[f_index]) {
					this->test_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
					this->test_input_folds[f_index]->pop_scope();
					this->test_input_folds[f_index]->add_scope(this->test_compressed_scope_sizes[0]);
					this->test_input_folds[f_index]->add_scope(this->test_compress_new_size);
				}
			}
			this->test_end_fold = new FoldNetwork(this->curr_end_fold);
			this->test_end_fold->pop_scope();
			this->test_end_fold->add_scope(this->test_compressed_scope_sizes[0]);
			this->test_end_fold->add_scope(this->test_compress_new_size);

			this->test_compressed_scope_sizes.erase(this->test_compressed_scope_sizes.begin());

			cout << "COMPRESS_SCOPE success" << endl;
			cout << "ending COMPRESS_SCOPE" << endl;
			cout << "beginning COMPRESS_SCOPE" << endl;

			this->last_state = STATE_COMPRESS_SCOPE;
			this->state = STATE_COMPRESS_SCOPE;
			this->state_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;
		}
	} else {
		if (this->test_compress_network != NULL) {
			delete this->test_compress_network;
			this->test_compress_network = NULL;
		}

		delete this->test_fold;
		this->test_fold = NULL;
		for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
			if (this->is_existing[f_index]) {
				delete this->test_input_folds[f_index];
				this->test_input_folds[f_index] = NULL;
			}
		}
		delete this->test_end_fold;
		this->test_end_fold = NULL;

		// exiting compress_scope_end, so no need to rewind test_s_input_sizes and test_scope_sizes

		if (this->curr_scope_sizes.size() > 1) {
			if (this->curr_s_input_sizes[0] + this->curr_scope_sizes[0] == 0) {
				this->curr_score_network->subfold_index++;
			}
			
			if (this->curr_score_network->subfold_index == (int)this->curr_scope_sizes.size()-2) {
				cout << "COMPRESS_SCOPE fail" << endl;
				cout << "ending COMPRESS_SCOPE" << endl;
				cout << "beginning ADD_STEP" << endl;

				add_finished_step();
			} else {
				this->test_score_network = new FoldNetwork(this->curr_score_network);
				this->test_score_network->subfold_index++;

				// this->curr_compress_num_layers > 0 && this->curr_compress_new_size > 0
				if (this->curr_s_input_sizes[0] + this->curr_scope_sizes[0] == 0) {
					this->curr_compress_network->subfold_index++;
				}
				this->test_compress_network = new FoldNetwork(this->curr_compress_network);
				this->test_compress_network->subfold_index++;

				cout << "COMPRESS_SCOPE fail" << endl;
				cout << "ending COMPRESS_SCOPE" << endl;
				cout << "beginning STATE_INPUT" << endl;

				this->last_state = STATE_COMPRESS_SCOPE;
				this->state = STATE_INPUT;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->new_state_factor = 25;
			}
		} else {
			cout << "COMPRESS_SCOPE fail" << endl;
			cout << "ending COMPRESS_SCOPE" << endl;
			cout << "beginning ADD_STEP" << endl;

			add_finished_step();
		}
	}
}
