#include "test_node.h"

#include <iostream>

using namepsace std;

void TestNode::activate(vector<vector<double>>& state_vals,
						vector<double>& obs) {
	if (this->new_layer_size > 0) {
		this->obs_network->activate(obs);
		state_vals.push_back(vector<double>(this->new_layer_size));
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}
	}

	for (int i_index = 0; i_index < (int)score_input_networks.size(); i_index++) {
		this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]]);
		for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
			state_vals[this->score_input_layer[i_index]+1].push_back(
				this->score_input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	if (this->state >= STATE_SCORE_SMALL) {
		this->small_score_network->activate(state_vals.back());
	} else if (this->state >= STATE_SCORE) {
		this->score_network->activate(state_vals);
	}

	if (this->compress_size > 0) {
		for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
			this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]]);
			for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
				state_vals[this->input_layer[i_index]+1].push_back(
					this->input_networks[i_index]->output->acti_vals[s_index]);
			}
		}

		if (this->state >= STATE_COMPRESS_SMALL) {
			vector<double> compression_inputs;
			for (int l_index = state_vals.size()-this->compress_num_layers; l_index < state_vals.size(); l_index++) {
				for (int st_index = 0; st_index < state_vals[l_index]; st_index++) {
					compression_inputs.push_back(state_vals[l_index][st_index]);
				}
			}
			this->small_compression_network->activate(compression_inputs);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_size));

			for (int st_index = 0; st_index < (int)state_vals.back(); st_index++) {
				state_vals.back()[st_index] = this->compression_networks[c_index]->output->acti_vals[st_index];
			}
		} else if (this->state >= STATE_COMPRESS_STATE) {
			this->compression_network->activate(state_vals);

			int sum_scope_sizes = 0;
			for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
				sum_scope_sizes += (int)state_vals.back().size();
				state_vals.pop_back();
				scopes_on.pop_back();
			}
			state_vals.push_back(vector<double>(sum_scope_sizes - this->compress_size));

			for (int st_index = 0; st_index < (int)state_vals.back(); st_index++) {
				state_vals.back()[st_index] = this->compression_network->output->acti_vals[st_index];
			}
		}
	}
}

void TestNode::process(vector<vector<double>>& flat_inputs,
					   vector<vector<double>>& state_vals,
					   double target_val,
					   vector<Node*>& nodes) {
	if (this->state == STATE_SCORE
			|| this->state == STATE_SCORE_INPUT
			|| this->state == STATE_SCORE_SMALL) {
		if (this->stage == STAGE_LEARN) {
			if ((this->state_iter+1)%10000 == 0) {
				cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			}

			if (this->state >= STATE_SCORE_SMALL) {
				this->sum_error += abs(target_val - this->small_score_network->output->acti_vals[0]);
				vector<double> score_errors;
				score_errors.push_back(target_val - this->small_score_network->output->acti_vals[0]);
				if (this->state_iter <= 240000) {
					this->small_score_network->backprop_weights_with_no_error_signal(score_errors, 0.01);
				} else {
					this->small_score_network->backprop_weights_with_no_error_signal(score_errors, 0.002);
				}
			} else {
				this->sum_error += abs(target_val - this->score_network->output->acti_vals[0]);
				if (this->state_iter <= 240000) {
					this->score_network->backprop_weights_with_no_error_signal(target_val, 0.01);
				} else {
					this->score_network->backprop_weights_with_no_error_signal(target_val, 0.002);
				}
			}
		} else if (this->stage == STAGE_MEASURE) {
			// STATE_SCORE or STATE_SCORE_INPUT
			this->sum_error += abs(target_val - this->score_network->output->acti_vals[0]);
		} else {
			// STATE_SCORE_SMALL && STAGE_TUNE
			vector<vector<double>> state_errors;
			state_errors.reserve(this->curr_scope_sizes.size());
			for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
				state_errors.push_back(vector<double>(this->curr_scope_sizes[sc_index], 0.0));
			}

			vector<double> score_errors;
			score_errors.push_back(target_val - this->small_score_network->output->acti_vals[0]);
			this->small_score_network->backprop(score_errors, 0.002);
			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
				state_errors.back()[st_index] = this->small_score_network->input->errors[st_index];
				this->small_score_network->input->errors[st_index] = 0.0;
			}

			// *** //
		}
	} else {
		if (this->stage == STAGE_LEARN) {

		} else if (this->stage == STAGE_MEASURE) {

		} else {
			// this->stage == STAGE_TUNE

		}
	}
}
