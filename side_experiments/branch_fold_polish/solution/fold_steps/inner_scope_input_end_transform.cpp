#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::inner_scope_input_end() {
	if (this->sum_error/10000 < 0.001
			|| (this->inner_input_input_networks.size() > 0
				&& this->inner_input_input_sizes.back() == (this->curr_s_input_sizes[this->inner_input_input_layer.back()]
					+ this->curr_scope_sizes[this->inner_input_input_layer.back()]))) {
		delete this->curr_input_network;
		this->curr_input_network = this->test_input_network;
		this->test_input_network = NULL;

		if (this->curr_input_network->subfold_index == (int)this->curr_scope_sizes.size()-2) {
			if (this->existing_actions[this->finished_steps.size()] == NULL) {
				this->curr_s_input_sizes.push_back(0);
				this->curr_scope_sizes.push_back(this->obs_sizes[this->finished_steps.size()]);
			} else {
				this->curr_s_input_sizes.push_back(0);
				this->curr_scope_sizes.push_back(this->existing_actions[this->finished_steps.size()]->num_outputs);
			}

			this->curr_fold->add_scope(this->curr_scope_sizes.back());
			this->curr_fold->fold_index++;
			this->curr_fold->migrate_weights();	// TODO: migrate from fold_index to last_state for all hidden layer
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
			this->test_input_network = new FoldNetwork(this->curr_input_network);
			this->test_input_network->subfold_index++;

			// start from size 1 if have previous
			if (this->inner_input_input_networks.size() > 0) {
				this->inner_input_input_layer.push_back(this->test_input_network->subfold_index);
				this->inner_input_input_sizes.push_back(1);
				this->inner_input_input_networks.push_back(new FoldNetwork(1,
																		   this->curr_s_input_sizes[this->test_input_network->subfold_index],
																		   this->curr_scope_sizes[this->test_input_network->subfold_index],
																		   20));
				// add to curr_s_input_sizes permanently 1 at a time
				this->curr_s_input_sizes[this->test_input_network->subfold_index+1]++;
			}

			this->test_input_network->set_s_input_size(this->curr_s_input_sizes[
				this->test_input_network->subfold_index+1]);

			this->last_state = STATE_INNER_SCOPE_INPUT;
			this->state = STATE_INNER_SCOPE_INPUT;
			this->state_iter = 0;
			this->sum_error = 0.0;
			this->new_state_factor = 25;
		}
	} else {
		delete this->test_input_network;
		this->test_input_network = new FoldNetwork(this->curr_input_network);
		this->test_input_network->subfold_index++;

		if (this->inner_input_input_layer.size() > 0
				&& this->inner_input_input_layer.back() == this->test_input_network->subfold_index) {
			this->inner_input_input_sizes.back()++;
			delete this->inner_input_input_networks.back();
			this->inner_input_input_networks.pop_back();
		} else {
			this->inner_input_input_layer.push_back(this->test_input_network->subfold_index);
			this->inner_input_input_sizes.push_back(1);
		}
		this->inner_input_input_networks.push_back(new FoldNetwork(this->inner_input_input_sizes.back(),
																   this->curr_s_input_sizes[this->test_input_network->subfold_index],
																   this->curr_scope_sizes[this->test_input_network->subfold_index],
																   20));
		// add to curr_s_input_sizes permanently 1 at a time
		this->curr_s_input_sizes[this->test_input_network->subfold_index+1]++;
		this->test_input_network->set_s_input_size(this->curr_s_input_sizes[
			this->test_input_network->subfold_index+1]);

		// no change to last_state
		this->state = STATE_INNER_SCOPE_INPUT;
		this->state_iter = 0;
		this->sum_error = 0.0;
		this->new_state_factor = 25;
	}
}
