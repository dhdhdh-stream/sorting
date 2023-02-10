#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::add_finished_step() {
	FinishedStep* new_finished_step = new FinishedStep(this->is_inner_scope[this->finished_steps.size()],
													   this->scopes[this->finished_steps.size()],
													   this->actions[this->finished_steps.size()],
													   this->inner_input_input_layer,
													   this->inner_input_input_sizes,
													   this->inner_input_input_networks,
													   this->curr_inner_input_network,
													   this->scope_scale_mod[this->finished_steps.size()],
													   this->curr_score_network,
													   this->curr_confidence_network,
													   this->curr_compress_num_layers,
													   this->curr_compress_new_size,
													   this->curr_compress_network,
													   this->curr_compress_original_size,
													   this->curr_compressed_s_input_sizes,
													   this->curr_compressed_scope_sizes,
													   this->input_layer,
													   this->input_sizes,
													   this->input_networks);

	this->scope_scale_mod[this->finished_steps.size()] = NULL;
	this->inner_input_input_layer.clear();
	this->inner_input_input_sizes.clear();
	this->inner_input_input_networks.clear();
	this->curr_inner_input_network = NULL;
	this->curr_score_network = NULL;
	this->curr_confidence_network = NULL;
	this->curr_compress_network = NULL;
	this->curr_compressed_s_input_sizes.clear();
	this->curr_compressed_scope_sizes.clear();
	this->input_layer.clear();
	this->input_sizes.clear();
	this->input_networks.clear();

	this->finished_steps.push_back(new_finished_step);

	if ((int)this->finished_steps.size() == this->sequence_length) {
		cout << "ending STEP_ADDED" << endl;
		cout << "STATE_DONE" << endl;

		this->state = STATE_DONE;
	} else {
		this->checkpoint_s_input_sizes = this->curr_s_input_sizes;
		this->checkpoint_scope_sizes = this->curr_scope_sizes;
		delete this->checkpoint_fold;
		this->checkpoint_fold = new FoldNetwork(this->curr_fold);
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index] && this->curr_input_folds[f_index] != NULL) {
				delete this->checkpoint_input_folds[f_index];
				this->checkpoint_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
			}
		}
		delete this->checkpoint_end_fold;
		this->checkpoint_end_fold = new FoldNetwork(this->curr_end_fold);

		if (!this->is_inner_scope[this->finished_steps.size()]) {
			this->curr_s_input_sizes.push_back(0);
			// obs_size always 1 for sorting
			this->curr_scope_sizes.push_back(1);

			this->curr_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_fold->fold_index++;
			this->curr_fold->migrate_weights();
			for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
													   20);
			this->curr_confidence_network = new FoldNetwork(1,
															this->curr_s_input_sizes[0],
															this->curr_scope_sizes,
															20);

			cout << "ending STEP_ADDED" << endl;
			cout << "beginning STATE_SCORE" << endl;

			this->state = STATE_SCORE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			// don't move/delete this->curr_input_folds[this->finished_steps.size()] for saving/loading
			this->curr_inner_input_network = new FoldNetwork(this->curr_input_folds[this->finished_steps.size()]);

			if (this->curr_scope_sizes.size() > 1) {
				this->test_inner_input_network = new FoldNetwork(this->curr_inner_input_network);
				this->test_inner_input_network->subfold_index++;
				this->test_inner_input_network->set_s_input_size(this->curr_s_input_sizes[
					this->test_inner_input_network->subfold_index+1]);

				cout << "ending STEP_ADDED" << endl;
				cout << "beginning STATE_INNER_SCOPE_INPUT" << endl;

				this->state = STATE_INNER_SCOPE_INPUT;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->new_state_factor = 25;
			} else {
				this->curr_s_input_sizes.push_back(this->scopes[this->finished_steps.size()]->num_inputs);
				this->curr_scope_sizes.push_back(this->scopes[this->finished_steps.size()]->num_outputs);

				this->curr_fold->add_scope(this->curr_scope_sizes.back());
				this->curr_fold->fold_index++;
				this->curr_fold->migrate_weights();
				for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
														   20);
				this->curr_confidence_network = new FoldNetwork(1,
																this->curr_s_input_sizes[0],
																this->curr_scope_sizes,
																20);

				cout << "ending STEP_ADDED" << endl;
				cout << "beginning STATE_SCORE" << endl;

				this->state = STATE_SCORE;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	}
}

void Fold::restart_from_finished_step() {
	if (this->finished_steps.size() == 0) {
		// if no steps have been added yet, try from STARTING_COMPRESS
		
		// flat_to_fold initializes checkpoints

		// reset if there was any partial progress
		this->curr_starting_compress_new_size = this->starting_compress_original_size;
		if (this->curr_starting_compress_network != NULL) {
			delete this->curr_starting_compress_network;
			this->curr_starting_compress_network = NULL;
		}

		flat_to_fold();
	} else {
		this->checkpoint_s_input_sizes = this->curr_s_input_sizes;
		this->checkpoint_scope_sizes = this->curr_scope_sizes;
		this->checkpoint_fold = new FoldNetwork(this->curr_fold);
		this->checkpoint_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
		for (int f_index = 0; f_index < this->sequence_length; f_index++) {
			if (this->is_inner_scope[f_index] && this->curr_input_folds[f_index] != NULL) {
				this->checkpoint_input_folds[f_index] = new FoldNetwork(this->curr_input_folds[f_index]);
			}
		}
		this->checkpoint_end_fold = new FoldNetwork(this->curr_end_fold);

		if (!this->is_inner_scope[this->finished_steps.size()]) {
			this->curr_s_input_sizes.push_back(0);
			// obs_size always 1 for sorting
			this->curr_scope_sizes.push_back(1);

			this->curr_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_fold->fold_index++;
			this->curr_fold->migrate_weights();
			for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
													   20);
			this->curr_confidence_network = new FoldNetwork(1,
															this->curr_s_input_sizes[0],
															this->curr_scope_sizes,
															20);

			this->state = STATE_SCORE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			// don't move/delete this->curr_input_folds[this->finished_steps.size()] for saving/loading
			this->curr_inner_input_network = new FoldNetwork(this->curr_input_folds[this->finished_steps.size()]);

			if (this->curr_scope_sizes.size() > 1) {
				this->test_inner_input_network = new FoldNetwork(this->curr_inner_input_network);
				this->test_inner_input_network->subfold_index++;
				this->test_inner_input_network->set_s_input_size(this->curr_s_input_sizes[
					this->test_inner_input_network->subfold_index+1]);

				this->state = STATE_INNER_SCOPE_INPUT;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->new_state_factor = 25;
			} else {
				this->curr_s_input_sizes.push_back(this->scopes[this->finished_steps.size()]->num_inputs);
				this->curr_scope_sizes.push_back(this->scopes[this->finished_steps.size()]->num_outputs);

				this->curr_fold->add_scope(this->curr_scope_sizes.back());
				this->curr_fold->fold_index++;
				this->curr_fold->migrate_weights();
				for (int f_index = (int)this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
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
														   20);
				this->curr_confidence_network = new FoldNetwork(1,
																this->curr_s_input_sizes[0],
																this->curr_scope_sizes,
																20);

				this->state = STATE_SCORE;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	}
}
