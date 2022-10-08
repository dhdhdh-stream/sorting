#include "test_node.h"

#include <iostream>

using namepsace std;

TestNode::TestNode(vector<int> initial_scope_sizes,
				   FoldNetwork* original_fold,
				   int obs_size) {
	this->obs_size = obs_size;

	this->curr_scope_sizes = initial_scope_sizes;
	this->curr_fold = new FoldNetwork(original_fold);

	this->test_scope_sizes = this->curr_scope_sizes;
	this->test_fold = new FoldNetwork(this->curr_fold);
	this->test_fold->fold_index++;

	this->new_layer_size = 0;
	this->obs_network = NULL;

	this->curr_score_network = NULL;
	this->test_score_network = NULL;
	this->small_score_network = NULL;

	this->curr_compression_network = NULL;
	this->test_compression_network = NULL;
	this->small_compression_network = NULL;

	this->stage = STAGE_LEARN;
	this->state = STATE_OBS;
	this->state_iter = 0;
	this->sum_error = 0.0;
}

void TestNode::activate(vector<vector<double>>& state_vals,
						vector<double>& obs,
						double& predicted_score) {
	if (this->new_layer_size > 0) {
		this->obs_network->activate(obs);
		state_vals.push_back(vector<double>(this->new_layer_size));
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
		}
	}

	for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
		this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]]);
		for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
			state_vals[this->score_input_layer[i_index]+1].push_back(
				this->score_input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	if (this->state >= STATE_SCORE_SMALL) {
		this->small_score_network->activate(state_vals.back());
		predicted_score += this->small_score_network->output->acti_vals[0];
	} else if (this->state >= STATE_SCORE) {
		this->test_score_network->activate(state_vals);
		predicted_score += this->test_score_network->output->acti_vals[0];
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
			this->test_compression_network->activate(state_vals);

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
					   double& predicted_score,
					   double target_val,
					   vector<Node*>& nodes) {
	if (this->state == STATE_SCORE
			|| this->state == STATE_SCORE_INPUT
			|| this->state == STATE_SCORE_SMALL) {
		if (this->stage == STAGE_LEARN) {
			if ((this->state_iter+1)%10000 == 0) {
				cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			}

			this->sum_error += abs(target_val - predicted_score);

			vector<double> score_errors{target_val - predicted_score};
			if (this->state == STATE_SCORE) {
				if (this->state_iter <= 240000) {
					this->test_score_network->backprop_weights_with_no_error_signal(
						score_errors,
						0.01);
				} else {
					this->test_score_network->backprop_weights_with_no_error_signal(
						score_errors,
						0.002);
				}
			} else if (this->state == STATE_SCORE_INPUT) {
				if (this->state_iter <= 240000) {
					this->test_score_network->backprop_new_state(this->score_input_layer.back(),
																 this->score_input_sizes.back(),
																 score_errors,
																 0.01);
				} else {
					this->test_score_network->backprop_new_state(this->score_input_layer.back(),
																 this->score_input_sizes.back(),
																 score_errors,
																 0.002);
				}
				vector<double> input_errors;
				input_errors.reserve(this->score_input_sizes.back());
				int layer_size = this->test_score_network->state_inputs[this->score_input_layer.back()+1].size();
				for (int st_index = layer_size-this->score_input_sizes.back(); st_index < layer_size; st_index++) {
					input_errors.push_back(this->test_score_network->state_inputs[this->score_input_layer.back()+1]->errors[st_index]);
					this->test_score_network->state_inputs[this->score_input_layer.back()+1]->errors[st_index] = 0.0;
				}
				if (this->state_iter <= 240000) {
					this->score_input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.01);
				} else {
					this->score_input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.002);
				}
			} else {
				// this->state == STATE_SCORE_SMALL
				if (this->state_iter <= 240000) {
					this->small_score_network->backprop_weights_with_no_error_signal(score_errors, 0.01);
				} else {
					this->small_score_network->backprop_weights_with_no_error_signal(score_errors, 0.002);
				}
			}
		} else if (this->stage == STAGE_MEASURE) {
			// STATE_SCORE or STATE_SCORE_INPUT
			this->sum_error += abs(target_val - predicted_score);
		} else {
			// this->stage == STAGE_TUNE
			if ((this->state_iter+1)%10000 == 0) {
				cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			}

			this->sum_error += abs(target_val - predicted_score);

			vector<vector<double>> state_errors;
			state_errors.reserve(this->curr_scope_sizes.size());
			for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
				state_errors.push_back(vector<double>(this->curr_scope_sizes[sc_index], 0.0));
			}
			double score_error = target_val - predicted_score;

			vector<double> score_errors{score_error};
			if (this->state == STATE_SCORE_INPUT) {
				this->test_score_network->backprop(score_errors, 0.002);
				for (int sc_index = this->test_score_network->fold_index+1; sc_index < state_errors.size(); sc_index++) {
					for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
						state_errors[sc_index][st_index] += this->test_score_network->state_inputs[sc_index]->errors[st_index];
						this->test_score_network->state_inputs[sc_index]->errors[st_index] = 0.0;
					}
				}
				score_error -= this->test_score_network->output->acti_vals[0];
			} else {
				// this->state == STATE_SCORE_SMALL
				this->small_score_network->backprop(score_errors, 0.002);
				for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
					state_errors.back()[st_index] += this->small_score_network->input->errors[st_index];
					this->small_score_network->input->errors[st_index] = 0.0;
				}
				score_error -= this->small_score_network->output->acti_vals[0];
			}

			for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
				vector<double> score_input_errors(this->score_input_sizes[i_index]);
				for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
					score_input_errors[this->score_input_sizes[i_index]-1-s_index] = state_errors[this->score_input_layer[i_index]+1].back();
					state_errors[this->score_input_layer[i_index]+1].pop_back();
				}
				this->score_input_networks[i_index]->backprop(score_input_errors, 0.002);
				for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
					state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->input->errors[st_index];
					this->score_input_networks[i_index]->input->errors[st_index] = 0.0;
				}
			}

			if (this->new_layer_size > 0) {
				this->obs_network->backprop_weights_with_no_error_signal(state_errors.back(), 0.002);
				state_errors.pop_back();
			}

			for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
				nodes[n_index]->backprop(state_errors,
										 score_error);
			}
		}
	} else {
		if (this->stage == STAGE_LEARN) {
			if ((this->state_iter+1)%10000 == 0) {
				cout << this->state_iter << " sum_error: " << this->sum_error << endl;
				this->sum_error = 0.0;
			}

			this->test_fold->activate(flat_inputs,
									  state_vals,
									  predicted_score);

			vector<double> errors;
			errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
			this->sum_error += abs(errors[0]);

			if (this->state_iter <= 240000) {
				this->test_fold->backprop_last_state(errors, 0.01);
			} else {
				this->test_fold->backprop_last_state(errors, 0.002);
			}

			vector<double> last_scope_state_errors(this->test_scope_sizes.back());
			for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
				last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
				this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
			}

			if (this->state == STATE_OBS) {
				if (this->new_layer_size > 0) {
					if (this->state_iter <= 240000) {
						this->obs_network->backprop_weights_with_no_error_signal(
							last_scope_state_errors,
							0.01);
					} else {
						this->obs_network->backprop_weights_with_no_error_signal(
							last_scope_state_errors,
							0.002);
					}
				}
			} else if (this->state == STATE_COMPRESS_STATE
					|| this->state == STATE_COMPRESS_SCOPE) {
				if (this->state_iter <= 240000) {
					this->test_compression_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.01);
				} else {
					this->test_compression_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.002);
				}
			} else if (this->state == STATE_COMPRESS_INPUT) {
				if (this->state_iter <= 240000) {
					this->test_compression_network->backprop_new_state(
						this->input_layer.back(),
						this->input_sizes.back(),
						last_scope_state_errors,
						0.01);
				} else {
					this->test_compression_network->backprop_new_state(
						this->input_layer.back(),
						this->input_sizes.back(),
						last_scope_state_errors,
						0.002);
				}
				vector<double> input_errors;
				input_errors.reserve(this->input_sizes.back());
				int layer_size = this->test_compression_network->state_inputs[this->input_layer.back()+1].size();
				for (int st_index = layer_size-this->input_sizes.back(); st_index < layer_size; st_index++) {
					input_errors.push_back(this->test_compression_network->state_inputs[this->input_layer.back()+1]->errors[st_index]);
					this->test_compression_network->state_inputs[this->input_layer.back()+1]->errors[st_index] = 0.0;
				}
				if (this->state_iter <= 240000) {
					this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.01);
				} else {
					this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.002);
				}
			} else {
				// this->state == STATE_COMPRESS_SMALL
				if (this->state_iter <= 240000) {
					this->small_compression_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.01);
				} else {
					this->small_compression_network->backprop_weights_with_no_error_signal(
						last_scope_state_errors,
						0.002);
				}
			}
		} else if (this->stage == STAGE_MEASURE) {
			this->test_fold->activate(flat_inputs,
									  state_vals,
									  predicted_score);

			this->sum_error += abs(target_val - this->test_fold->output->acti_vals[0]);
		} else {
			// this->stage == STAGE_TUNE
			if ((this->state_iter+1) % 10000 == 0) {
				cout << this->state_iter << " sum_error: " << this->sum_error << endl;
			}

			this->test_fold->activate(flat_inputs,
									  state_vals,
									  predicted_score);

			vector<double> errors;
			errors.push_back(target_val - this->test_fold->output->acti_vals[0]);
			sum_error += abs(errors[0]);

			this->test_fold->backprop_full_state(errors, 0.002);

			vector<vector<double>> state_errors(this->test_scope_sizes.size());
			for (int sc_index = 0; sc_index < (int)this->test_scope_sizes.size(); sc_index++) {
				state_errors[sc_index].reserve(this->test_scope_sizes[sc_index]);
				for (int st_index = 0; st_index < this->test_scope_sizes[sc_index]; st_index++) {
					state_errors[sc_index].push_back(this->test_fold->state_inputs[sc_index]->errors[st_index]);
					this->test_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			double score_error = target_val - predicted_score;

			// this->compress_size > 0
			if (this->state == STATE_COMPRESS_INPUT) {
				this->test_compression_network->backprop(state_errors.back(), 0.002);

				state_errors.pop_back();
				for (int sc_index = this->compress_num_layers-1; sc_index >= 0; sc_index--) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
				}

				for (int sc_index = this->test_compression_network->fold_index+1; sc_index < state_errors.size(); sc_index++) {
					for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
						state_errors[sc_index][st_index] += this->test_compression_network->state_inputs[sc_index]->errors[st_index];
						this->test_compression_network->state_inputs[sc_index]->errors[st_index] = 0.0;
					}
				}
			} else {
				// this->state == STATE_COMPRESS_SMALL
				this->small_compression_network->backprop(state_errors.back(), 0.002);

				state_errors.pop_back();
				for (int sc_index = this->compress_num_layers-1; sc_index >= 0; sc_index--) {
					state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
				}

				int input_index = 0;
				for (int l_index = state_errors.size()-this->compress_num_layers; l_index < state_errors.size(); l_index++) {
					for (int st_index = 0; st_index < (int)state_errors[l_index].size(); st_index++) {
						state_errors[l_index][st_index] += this->small_compression_network->input->errors[input_index];
						this->small_compression_network->input->errors[input_index] = 0.0;
						input_index++;
					}
				}
			}

			for (int i_index = (int)this->input_networks.size()-1; i_index >= 0; i_index--) {
				vector<double> input_errors(this->input_sizes[i_index]);
				for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
					input_errors[this->input_sizes[i_index]-1-s_index] = state_errors[this->input_layer[i_index]+1].back();
					state_errors[this->input_layer[i_index]+1].pop_back();
				}
				this->input_networks[i_index]->backprop(input_errors, 0.002);
				for (int st_index = 0; st_index < (int)state_errors[this->input_layer[i_index]].size(); st_index++) {
					state_errors[this->input_layer[i_index]][st_index] += this->input_networks[i_index]->input->errors[st_index];
					this->input_networks[i_index]->input->errors[st_index] = 0.0;
				}
			}

			vector<double> score_errors{score_error};
			this->small_score_network->backprop(score_errors, 0.002);
			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
				state_errors.back()[st_index] += this->small_score_network->input->errors[st_index];
				this->small_score_network->input->errors[st_index] = 0.0;
			}
			score_error -= this->small_score_network->output->acti_vals[0];

			for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
				vector<double> score_input_errors(this->score_input_sizes[i_index]);
				for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
					score_input_errors[this->score_input_sizes[i_index]-1-s_index] = state_errors[this->score_input_layer[i_index]+1].back();
					state_errors[this->score_input_layer[i_index]+1].pop_back();
				}
				this->score_input_networks[i_index]->backprop(score_input_errors,
															  TARGET_MAX_UPDATE);
				for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
					state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->input->errors[st_index];
					this->score_input_networks[i_index]->input->errors[st_index] = 0.0;
				}
			}

			if (this->new_layer_size > 0) {
				this->obs_network->backprop_weights_with_no_error_signal(state_errors.back(), 0.002);
				state_errors.pop_back();
			}

			for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
				nodes[n_index]->backprop(state_errors,
										 score_error);
			}
		}
	}

	increment();
}

void TestNode::increment() {
	this->stage_iter++;

	if (this->state == STATE_OBS
			|| this->stage == STAGE_LEARN) {
		if (this->state_iter >= 300000) {
			cout << "ending STATE_OBS-STAGE_LEARN" << endl;
			cout << "starting STATE_OBS-STAGE_MEASURE" << endl;

			this->state = STATE_OBS;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_OBS
			|| this->stage == STAGE_MEASURE) {
		if (this->state_iter >= 10000) {
			if (this->sum_error/10000 < 1.2*this->curr_fold->average_error) {
				cout << "ending STATE_OBS-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE-STAGE_LEARN" << endl;

				this->curr_scope_sizes = this->test_scope_sizes;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->sum_error/10000 < this->curr_fold->average_error) {
					this->curr_fold->average_error = this->sum_error/10000;
				}

				this->test_score_network = new SubFoldNetwork(this->curr_scope_sizes, 1);

				this->stage = STATE_SCORE;
				this->state = STATE_LEARN;
				this->state_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "ending STATE_OBS-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_OBS-STAGE_LEARN" << endl;

				this->new_layer_size++;

				this->test_scope_sizes = this->curr_scope_sizes;
				this->test_scope_sizes.push_back(this->new_layer_size);

				delete this->test_fold;
				this->test_fold = new FoldNetwork(this->curr_fold);
				this->test_fold->fold_index++;
				this->test_fold->add_scope(this->new_layer_size);

				if (this->obs_network != NULL) {
					delete this->obs_network;
				}
				this->obs_network = new Network(this->obs_size,
												10*this->obs_size,
												this->new_layer_size);

				this->stage = STAGE_LEARN;
				this->state = STATE_OBS;
				this->state_iter = 0;
				this->sum_error = 0.0;
			}
		}
	}
}