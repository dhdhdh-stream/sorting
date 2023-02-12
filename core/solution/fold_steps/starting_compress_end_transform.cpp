#include "fold.h"

#include <cmath>
#include <iostream>

#include "globals.h"

using namespace std;

void Fold::starting_compress_end() {
	if (this->sum_error/10000 < 0.01) {
		this->curr_starting_compress_new_size = this->test_starting_compress_new_size;

		// no change to this->curr_s_input_sizes
		this->curr_scope_sizes = this->test_scope_sizes;

		if (this->curr_starting_compress_network != NULL) {
			delete this->curr_starting_compress_network;
		}
		this->curr_starting_compress_network = this->test_starting_compress_network;
		this->test_starting_compress_network = NULL;

		delete this->curr_fold;
		this->curr_fold = this->test_fold;
		this->test_fold = NULL;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				delete this->curr_input_folds[f_index];
				this->curr_input_folds[f_index] = this->test_input_folds[f_index];
				this->test_input_folds[f_index] = NULL;
			}
		}
		delete this->curr_end_fold;
		this->curr_end_fold = this->test_end_fold;
		this->test_end_fold = NULL;

		if (this->curr_starting_compress_new_size == 0) {
			// if this->is_inner_scope[0] == true, this->curr_scope_sizes.size() == 1, so skip STATE_INNER_SCOPE_INPUT

			if (this->is_inner_scope[0]) {
				// don't move/delete this->curr_input_folds[0] for saving/loading
				this->curr_inner_input_network = new FoldNetwork(this->curr_input_folds[0]);
			}

			if (!this->is_inner_scope[0]) {
				this->curr_s_input_sizes.push_back(0);
				// obs_size always 1 for sorting
				this->curr_scope_sizes.push_back(1);
			} else {
				this->curr_s_input_sizes.push_back(solution->scope_dictionary[this->existing_scope_ids[0]]->num_inputs);
				this->curr_scope_sizes.push_back(solution->scope_dictionary[this->existing_scope_ids[0]]->num_outputs);
			}

			this->curr_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_fold->fold_index++;
			this->curr_fold->migrate_weights();
			for (int f_index = 1; f_index < this->sequence_length; f_index++) {
				if (this->is_inner_scope[f_index]) {
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
													   50);

			cout << "STARTING_COMPRESS success" << endl;
			cout << "ending STARTING_COMPRESS" << endl;
			cout << "beginning STATE_SCORE" << endl;

			this->state = STATE_SCORE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			this->test_starting_compress_new_size = this->curr_starting_compress_new_size-1;

			// no change to this->curr_s_input_sizes
			this->test_scope_sizes = this->curr_scope_sizes;
			this->test_scope_sizes.pop_back();
			this->test_scope_sizes.push_back(this->test_starting_compress_new_size);

			if (this->test_starting_compress_new_size > 0) {
				this->test_starting_compress_network = new FoldNetwork(
					this->test_starting_compress_new_size,
					this->curr_s_input_sizes[0],	// i.e., starting_s_input_size
					vector<int>{this->starting_compress_original_size},
					50);
			}

			this->test_fold = new FoldNetwork(this->curr_fold);
			this->test_fold->pop_scope();
			// this->test_starting_compress_new_size can be 0
			this->test_fold->add_scope(this->test_starting_compress_new_size);
			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
				if (this->is_inner_scope[f_index]) {
					this->test_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
					this->test_input_folds[f_index]->pop_scope();
					this->test_input_folds[f_index]->add_scope(this->test_starting_compress_new_size);
				}
			}
			this->test_end_fold = new FoldNetwork(this->curr_end_fold);
			this->test_end_fold->pop_scope();
			this->test_end_fold->add_scope(this->test_starting_compress_new_size);

			cout << "STARTING_COMPRESS success" << endl;
			cout << "ending STARTING_COMPRESS" << endl;
			cout << "beginning STARTING_COMPRESS" << endl;

			this->state = STATE_STARTING_COMPRESS;
			this->state_iter = 0;
			this->sum_error = 0.0;
		}
	} else {
		if (this->test_starting_compress_network != NULL) {
			delete this->test_starting_compress_network;
			this->test_starting_compress_network = NULL;
		}

		delete this->test_fold;
		this->test_fold = NULL;
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
				delete this->test_input_folds[f_index];
				this->test_input_folds[f_index] = NULL;
			}
		}
		delete this->test_end_fold;
		this->test_end_fold = NULL;

		// if this->is_inner_scope[0] == true, this->curr_scope_sizes.size() == 1, so skip STATE_INNER_SCOPE_INPUT

		if (this->is_inner_scope[0]) {
			// don't move/delete this->curr_input_folds[0] for saving/loading
			this->curr_inner_input_network = new FoldNetwork(this->curr_input_folds[0]);
		}

		if (!this->is_inner_scope[0]) {
			this->curr_s_input_sizes.push_back(0);
			// obs_size always 1 for sorting
			this->curr_scope_sizes.push_back(1);
		} else {
			this->curr_s_input_sizes.push_back(solution->scope_dictionary[this->existing_scope_ids[0]]->num_inputs);
			this->curr_scope_sizes.push_back(solution->scope_dictionary[this->existing_scope_ids[0]]->num_outputs);
		}

		this->curr_fold->add_scope(this->curr_scope_sizes.back());
		this->curr_fold->fold_index++;
		this->curr_fold->migrate_weights();
		for (int f_index = 1; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index]) {
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
												   50);

		cout << "STARTING_COMPRESS fail" << endl;
		cout << "ending STARTING_COMPRESS" << endl;
		cout << "beginning STATE_SCORE" << endl;

		this->state = STATE_SCORE;
		this->state_iter = 0;
		this->sum_error = 0.0;
	}
}
