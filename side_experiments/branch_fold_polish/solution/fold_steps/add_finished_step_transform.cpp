#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::add_finished_step() {
	FinishedStep* new_finished_step = new FinishedStep((this->existing_actions[this->finished_steps.size()] != NULL),
													   this->existing_actions[this->finished_steps.size()],
													   this->obs_sizes[this->finished_steps.size()],
													   this->inner_input_input_layer,
													   this->inner_input_input_sizes,
													   this->inner_input_input_networks,
													   this->curr_input_network,
													   this->scope_scale_mod_calcs[this->finished_steps.size()]->output->constants[0],
													   this->curr_score_network,
													   this->curr_compress_num_layers,
													   this->curr_compress_new_size,
													   this->curr_compress_network,
													   this->curr_compress_original_size,
													   this->curr_compressed_s_input_sizes,
													   this->curr_compressed_scope_sizes,
													   this->input_layer,
													   this->input_sizes,
													   this->input_networks);

	this->existing_actions[this->finished_steps.size()] = NULL;
	this->inner_input_input_layer.clear();
	this->inner_input_input_sizes.clear();
	this->inner_input_input_networks.clear();
	this->curr_input_network.clear();
	delete this->scope_scale_mod_calcs[this->finished_steps.size()];
	this->scope_scale_mod_calcs[this->finished_steps.size()] = NULL;
	this->curr_score_network = NULL;
	this->curr_compress_network = NULL;
	this->curr_compressed_s_input_sizes.clear();
	this->curr_compressed_scope_sizes.clear();
	this->input_layer.clear();
	this->input_sizes.clear();
	this->input_networks.clear();

	this->finished_steps.push_back(new_finished_step);

	if (this->finished_steps.size() == this->sequence_length) {
		this->state = STATE_DONE;
	} else {
		if (this->existing_actions[this->finished_steps.size()] == NULL) {
			this->curr_s_input_sizes.push_back(0);
			this->curr_scope_sizes.push_back(this->obs_sizes[this->finished_steps.size()]);

			this->curr_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_fold->fold_index++;
			this->curr_fold->migrate_weights();
			for (int f_index = this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
				if (this->existing_actions[f_index] != NULL) {
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

			this->last_state = STATE_SCORE;	// for STATE_SCORE, also set last_state to STATE_SCORE
			this->state = STATE_SCORE;
			this->state_iter = 0;
			this->sum_error = 0.0;
		} else {
			this->curr_input_network = this->curr_input_folds[this->finished_steps.size()];
			this->curr_input_folds[this->finished_steps.size()] = NULL;

			if (this->curr_scope_sizes.size() > 1) {
				this->test_input_network = new FoldNetwork(this->curr_input_network);
				this->test_input_network->subfold_index++;

				this->last_state = STATE_STEP_ADDED;
				this->state = STATE_INNER_SCOPE_INPUT;
				this->state_iter = 0;
				this->sum_error = 0.0;
				this->new_state_factor = 25;
			} else {
				this->curr_s_input_sizes.push_back(this->existing_actions[this->finished_steps.size()]->num_inputs);
				this->curr_scope_sizes.push_back(this->existing_actions[this->finished_steps.size()]->num_outputs);

				this->curr_fold->add_scope(this->curr_scope_sizes.back());
				this->curr_fold->fold_index++;
				this->curr_fold->migrate_weights();
				for (int f_index = this->finished_steps.size()+1; f_index < this->sequence_length; f_index++) {
					if (this->existing_actions[f_index] != NULL) {
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

				this->last_state = STATE_SCORE;	// for STATE_SCORE, also set last_state to STATE_SCORE
				this->state = STATE_SCORE;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	}
}
