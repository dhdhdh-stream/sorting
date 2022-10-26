#include "test_node.h"

#include <cmath>
#include <iostream>

using namespace std;

TestNode::TestNode(vector<int> initial_scope_sizes,
				   FoldNetwork* original_fold,
				   int obs_size,
				   double max_allowable_error,
				   double max_needed_error) {
	this->obs_size = obs_size;
	this->max_allowable_error = max_allowable_error;
	this->max_needed_error = max_needed_error;

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

	this->compress_num_layers = 0;
	this->compress_new_size = 0;
	this->curr_compression_network = NULL;
	this->test_compression_network = NULL;
	this->small_compression_network = NULL;

	this->state = STATE_OBS;
	this->stage = STAGE_LEARN;
	this->stage_iter = 0;
	this->sum_error = 0.0;
}

TestNode::~TestNode() {
	// do nothing
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

		if (this->compress_num_layers > 0) {
			if (this->compress_new_size == 0) {
				for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
					state_vals.pop_back();
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
					this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]]);
					for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
						state_vals[this->input_layer[i_index]+1].push_back(
							this->input_networks[i_index]->output->acti_vals[s_index]);
					}
				}

				if (this->state >= STATE_COMPRESS_SMALL) {
					vector<double> compression_inputs;
					for (int l_index = (int)state_vals.size()-this->compress_num_layers; l_index < (int)state_vals.size(); l_index++) {
						for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
							compression_inputs.push_back(state_vals[l_index][st_index]);
						}
					}
					this->small_compression_network->activate(compression_inputs);

					for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
						state_vals.pop_back();
					}
					state_vals.push_back(vector<double>(this->compress_new_size));

					for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
						state_vals.back()[st_index] = this->small_compression_network->output->acti_vals[st_index];
					}
				} else if (this->state >= STATE_COMPRESS_STATE) {
					this->test_compression_network->activate(state_vals);

					for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
						state_vals.pop_back();
					}
					state_vals.push_back(vector<double>(this->compress_new_size));

					for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
						state_vals.back()[st_index] = this->test_compression_network->output->acti_vals[st_index];
					}
				}
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
			if ((this->stage_iter+1)%10000 == 0) {
				cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
				this->sum_error = 0.0;
			}

			this->sum_error += (target_val - predicted_score)*(target_val - predicted_score);

			vector<double> score_errors{target_val - predicted_score};
			if (this->state == STATE_SCORE) {
				if (this->stage_iter <= 240000) {
					this->test_score_network->backprop_weights_with_no_error_signal(
						score_errors,
						0.01);
				} else {
					this->test_score_network->backprop_weights_with_no_error_signal(
						score_errors,
						0.002);
				}
			} else if (this->state == STATE_SCORE_INPUT) {
				if (this->score_input_networks.size() == 0) {
					if (this->stage_iter <= 240000) {
						this->test_score_network->backprop_weights_with_no_error_signal(
							score_errors,
							0.01);
					} else {
						this->test_score_network->backprop_weights_with_no_error_signal(
							score_errors,
							0.002);
					}
				} else {
					if (this->stage_iter <= 240000) {
						this->test_score_network->backprop_new_state(
							this->score_input_layer.back()+1,
							this->score_input_sizes.back(),
							score_errors,
							0.01);
					} else {
						this->test_score_network->backprop_new_state(
							this->score_input_layer.back()+1,
							this->score_input_sizes.back(),
							score_errors,
							0.002);
					}
					vector<double> input_errors;
					input_errors.reserve(this->score_input_sizes.back());
					int layer_size = (int)this->test_score_network->state_inputs[this->score_input_layer.back()+1]->errors.size();
					for (int st_index = layer_size-this->score_input_sizes.back(); st_index < layer_size; st_index++) {
						input_errors.push_back(this->test_score_network->state_inputs[this->score_input_layer.back()+1]->errors[st_index]);
						this->test_score_network->state_inputs[this->score_input_layer.back()+1]->errors[st_index] = 0.0;
					}
					if (this->stage_iter <= 240000) {
						this->score_input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.01);
					} else {
						this->score_input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.002);
					}
				}
			} else {
				// this->state == STATE_SCORE_SMALL
				if (this->stage_iter <= 240000) {
					this->small_score_network->backprop_weights_with_no_error_signal(score_errors, 0.01);
				} else {
					this->small_score_network->backprop_weights_with_no_error_signal(score_errors, 0.002);
				}
			}
		} else if (this->stage == STAGE_MEASURE) {
			// STATE_SCORE or STATE_SCORE_INPUT
			this->sum_error += (target_val - predicted_score)*(target_val - predicted_score);
		} else {
			// this->stage == STAGE_TUNE
			if ((this->stage_iter+1)%10000 == 0) {
				cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
			}

			this->sum_error += (target_val - predicted_score)*(target_val - predicted_score);

			// predicted_score changes, so fold needs to be updated anyways
			this->curr_fold->activate(flat_inputs,
									  state_vals);

			vector<double> errors;
			errors.push_back((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);

			this->curr_fold->backprop(errors, 0.002);

			vector<vector<double>> state_errors(this->curr_scope_sizes.size());
			for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
				state_errors[sc_index].reserve(this->curr_scope_sizes[sc_index]);
				for (int st_index = 0; st_index < this->curr_scope_sizes[sc_index]; st_index++) {
					state_errors[sc_index].push_back(this->curr_fold->state_inputs[sc_index]->errors[st_index]);
					this->curr_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			double score_error = target_val - predicted_score;

			vector<double> score_errors{score_error};
			if (this->state == STATE_SCORE_INPUT) {
				this->test_score_network->backprop(score_errors, 0.002);
				for (int sc_index = this->test_score_network->fold_index+1; sc_index < (int)state_errors.size(); sc_index++) {
					for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
						state_errors[sc_index][st_index] += this->test_score_network->state_inputs[sc_index]->errors[st_index];
						this->test_score_network->state_inputs[sc_index]->errors[st_index] = 0.0;
					}
				}
				predicted_score -= this->test_score_network->output->acti_vals[0];
			} else {
				// this->state == STATE_SCORE_SMALL
				this->small_score_network->backprop(score_errors, 0.002);
				for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
					state_errors.back()[st_index] += this->small_score_network->input->errors[st_index];
					this->small_score_network->input->errors[st_index] = 0.0;
				}
				predicted_score -= this->small_score_network->output->acti_vals[0];
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

			this->obs_network->backprop_weights_with_no_error_signal(state_errors.back(), 0.002);
			state_errors.pop_back();

			for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
				nodes[n_index]->backprop(state_errors,
										 predicted_score,
										 target_val);
			}
		}
	} else {
		if (this->stage == STAGE_LEARN) {
			if ((this->stage_iter+1)%10000 == 0) {
				cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
				this->sum_error = 0.0;
			}

			if (this->state == STATE_OBS) {
				this->test_fold->activate(flat_inputs,
										  state_vals);

				vector<double> errors;
				errors.push_back((target_val-predicted_score) - this->test_fold->output->acti_vals[0]);
				this->sum_error += errors[0]*errors[0];

				if (this->stage_iter <= 240000) {
					this->test_fold->backprop_last_state(errors, 0.01);
				} else {
					this->test_fold->backprop_last_state(errors, 0.002);
				}

				if (this->new_layer_size > 0) {
					vector<double> last_scope_state_errors(this->test_scope_sizes.back());
					for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
						last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
						this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
					}

					if (this->stage_iter <= 240000) {
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
				this->test_fold->activate(flat_inputs,
										  state_vals);

				vector<double> errors;
				errors.push_back((target_val-predicted_score) - this->test_fold->output->acti_vals[0]);
				this->sum_error += errors[0]*errors[0];

				if (this->stage_iter <= 240000) {
					this->test_fold->backprop_last_state(errors, 0.01);
				} else {
					this->test_fold->backprop_last_state(errors, 0.002);
				}

				if (this->compress_new_size > 0) {
					vector<double> last_scope_state_errors(this->test_scope_sizes.back());
					for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
						last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
						this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
					}

					if (this->stage_iter <= 240000) {
						this->test_compression_network->backprop_weights_with_no_error_signal(
							last_scope_state_errors,
							0.01);
					} else {
						this->test_compression_network->backprop_weights_with_no_error_signal(
							last_scope_state_errors,
							0.002);
					}
				}
			} else if (this->state == STATE_COMPRESS_INPUT) {
				this->curr_fold->activate(flat_inputs,
										  state_vals);

				vector<double> errors;
				errors.push_back((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);
				this->sum_error += errors[0]*errors[0];

				this->curr_fold->backprop_last_state_with_no_weight_change(errors);

				vector<double> last_scope_state_errors(this->curr_scope_sizes.back());
				for (int st_index = 0; st_index < this->curr_scope_sizes.back(); st_index++) {
					last_scope_state_errors[st_index] = this->curr_fold->state_inputs.back()->errors[st_index];
					this->curr_fold->state_inputs.back()->errors[st_index] = 0.0;
				}

				if (this->input_networks.size() == 0) {
					if (this->stage_iter <= 240000) {
						this->test_compression_network->backprop_weights_with_no_error_signal(
							last_scope_state_errors,
							0.01);
					} else {
						this->test_compression_network->backprop_weights_with_no_error_signal(
							last_scope_state_errors,
							0.002);
					}
				} else {
					if (this->stage_iter <= 240000) {
						this->test_compression_network->backprop_new_state(
							this->input_layer.back()+1,
							this->input_sizes.back(),
							last_scope_state_errors,
							0.01);
					} else {
						this->test_compression_network->backprop_new_state(
							this->input_layer.back()+1,
							this->input_sizes.back(),
							last_scope_state_errors,
							0.002);
					}
					vector<double> input_errors;
					input_errors.reserve(this->input_sizes.back());
					int layer_size = (int)this->test_compression_network->state_inputs[this->input_layer.back()+1]->errors.size();
					for (int st_index = layer_size-this->input_sizes.back(); st_index < layer_size; st_index++) {
						input_errors.push_back(this->test_compression_network->state_inputs[this->input_layer.back()+1]->errors[st_index]);
						this->test_compression_network->state_inputs[this->input_layer.back()+1]->errors[st_index] = 0.0;
					}
					if (this->stage_iter <= 240000) {
						this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.01);
					} else {
						this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.002);
					}
				}
			} else {
				// this->state == STATE_COMPRESS_SMALL
				this->curr_fold->activate(flat_inputs,
										  state_vals);

				vector<double> errors;
				errors.push_back((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);
				this->sum_error += errors[0]*errors[0];

				if (this->stage_iter <= 240000) {
					this->curr_fold->backprop_last_state(errors, 0.01);
				} else {
					this->curr_fold->backprop_last_state(errors, 0.002);
				}

				vector<double> last_scope_state_errors(this->curr_scope_sizes.back());
				for (int st_index = 0; st_index < this->curr_scope_sizes.back(); st_index++) {
					last_scope_state_errors[st_index] = this->curr_fold->state_inputs.back()->errors[st_index];
					this->curr_fold->state_inputs.back()->errors[st_index] = 0.0;
				}

				if (this->stage_iter <= 240000) {
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
			if (this->state == STATE_OBS
					|| this->state == STATE_COMPRESS_STATE
					|| this->state == STATE_COMPRESS_SCOPE) {
				this->test_fold->activate(flat_inputs,
										  state_vals);

				this->sum_error += ((target_val-predicted_score) - this->test_fold->output->acti_vals[0])*((target_val-predicted_score) - this->test_fold->output->acti_vals[0]);
			} else {
				// STATE_COMPRESS_STATE or STATE_COMPRESS_SCOPE
				this->curr_fold->activate(flat_inputs,
										  state_vals);

				this->sum_error += ((target_val-predicted_score) - this->curr_fold->output->acti_vals[0])*((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);
			}
		} else {
			// this->stage == STAGE_TUNE
			if ((this->stage_iter+1) % 10000 == 0) {
				cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
			}

			this->curr_fold->activate(flat_inputs,
									  state_vals);

			vector<double> errors;
			errors.push_back((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);
			sum_error += errors[0]*errors[0];

			this->curr_fold->backprop(errors, 0.002);

			vector<vector<double>> state_errors(this->curr_scope_sizes.size());
			for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
				state_errors[sc_index].reserve(this->curr_scope_sizes[sc_index]);
				for (int st_index = 0; st_index < this->curr_scope_sizes[sc_index]; st_index++) {
					state_errors[sc_index].push_back(this->curr_fold->state_inputs[sc_index]->errors[st_index]);
					this->curr_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}
			double score_error = target_val - predicted_score;

			if (this->compress_num_layers > 0) {
				if (this->compress_new_size == 0) {
					for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
						state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
					}
				} else {
					if (this->state == STATE_COMPRESS_INPUT) {
						this->test_compression_network->backprop(state_errors.back(), 0.002);

						state_errors.pop_back();
						for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
							state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
						}

						for (int sc_index = this->test_compression_network->fold_index+1; sc_index < (int)state_errors.size(); sc_index++) {
							for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
								state_errors[sc_index][st_index] += this->test_compression_network->state_inputs[sc_index]->errors[st_index];
								this->test_compression_network->state_inputs[sc_index]->errors[st_index] = 0.0;
							}
						}
					} else {
						// this->state == STATE_FINAL_TUNE
						this->small_compression_network->backprop(state_errors.back(), 0.002);

						state_errors.pop_back();
						for (int sc_index = 0; sc_index < this->compress_num_layers; sc_index++) {
							state_errors.push_back(vector<double>(this->compressed_scope_sizes[sc_index], 0.0));
						}

						int input_index = 0;
						for (int l_index = (int)state_errors.size()-this->compress_num_layers; l_index < (int)state_errors.size(); l_index++) {
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
				}
			}

			vector<double> score_errors{score_error};
			this->small_score_network->backprop(score_errors, 0.002);
			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
				state_errors.back()[st_index] += this->small_score_network->input->errors[st_index];
				this->small_score_network->input->errors[st_index] = 0.0;
			}
			predicted_score -= this->small_score_network->output->acti_vals[0];

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

			this->obs_network->backprop_weights_with_no_error_signal(state_errors.back(), 0.002);
			state_errors.pop_back();

			for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
				nodes[n_index]->backprop(state_errors,
										 predicted_score,
										 target_val);
			}
		}
	}

	increment();
}

void TestNode::increment() {
	this->stage_iter++;

	if (this->state == STATE_OBS
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_OBS-STAGE_LEARN" << endl;
			cout << "starting STATE_OBS-STAGE_MEASURE" << endl;

			this->state = STATE_OBS;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_OBS
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < this->max_allowable_error
					|| this->new_layer_size == this->obs_size) {
				this->curr_scope_sizes = this->test_scope_sizes;

				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->new_layer_size == 0) {
					cout << "ending STATE_OBS-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "DONE" << endl;

					this->state = STATE_DONE;
				} else {
					cout << "ending STATE_OBS-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_SCORE-STAGE_LEARN" << endl;

					this->test_score_network = new SubFoldNetwork(this->curr_scope_sizes, 1);

					this->state = STATE_SCORE;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
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

				this->state = STATE_OBS;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_SCORE-STAGE_LEARN" << endl;
			cout << "starting STATE_SCORE-STAGE_MEASURE" << endl;

			this->state = STATE_SCORE;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_SCORE
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			this->curr_score_network = this->test_score_network;
			
			if (this->curr_scope_sizes.size() == 1) {
				cout << "ending STATE_SCORE-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_SMALL-STAGE_LEARN" << endl;

				delete this->curr_score_network;
				int input_size = this->curr_scope_sizes.back();
				this->small_score_network = new Network(input_size,
														10*input_size,
														1);

				this->state = STATE_SCORE_SMALL;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			} else {
				cout << "ending STATE_SCORE-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_INPUT-STAGE_LEARN" << endl;

				this->average_misguess = this->sum_error/10000;

				this->test_score_network = new SubFoldNetwork(this->curr_score_network);
				this->test_score_network->fold_index++;

				this->state = STATE_SCORE_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE_INPUT
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_SCORE_INPUT-STAGE_LEARN" << endl;
			cout << "starting STATE_SCORE_INPUT-STAGE_MEASURE" << endl;

			this->state = STATE_SCORE_INPUT;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_SCORE_INPUT
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < this->average_misguess+this->max_allowable_error
					|| (this->score_input_networks.size() > 0 && this->score_input_sizes.back() == this->curr_scope_sizes[this->score_input_layer.back()])) {
				cout << "ending STATE_SCORE_INPUT-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_INPUT-STAGE_TUNE" << endl;

				this->state = STATE_SCORE_INPUT;
				this->stage = STAGE_TUNE;
				this->stage_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				cout << "ending STATE_SCORE_INPUT-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_SCORE_INPUT-STAGE_LEARN" << endl;

				delete this->test_score_network;
				this->test_score_network = new SubFoldNetwork(this->curr_score_network);
				this->test_score_network->fold_index++;

				if (this->score_input_layer.size() > 0
						&& this->score_input_layer.back() == this->test_score_network->fold_index) {
					this->score_input_sizes.back()++;
					delete this->score_input_networks.back();
					this->score_input_networks.pop_back();
				} else {
					this->score_input_layer.push_back(this->test_score_network->fold_index);
					this->score_input_sizes.push_back(1);
				}
				int input_size = this->curr_scope_sizes[this->test_score_network->fold_index];
				this->score_input_networks.push_back(new Network(input_size,
																 10*input_size,
																 this->score_input_sizes.back()));
				this->test_score_network->add_state(this->test_score_network->fold_index+1,
													this->score_input_sizes.back());
				// add to curr_fold permanently 1 at a time
				this->curr_scope_sizes[this->test_score_network->fold_index+1]++;
				this->curr_fold->add_state(this->test_score_network->fold_index+1, 1);

				this->state = STATE_SCORE_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_SCORE_INPUT
			&& this->stage == STAGE_TUNE) {
		if (this->stage_iter >= 40000) {
			bool done = false;
			if (this->sum_error/40000 < this->max_needed_error) {
				done = true;
			} else if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->stage_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try++;
				} else {
					done = true;
				}
			}

			if (done) {
				if (this->curr_score_network != NULL) {
					delete this->curr_score_network;
				}
				this->curr_score_network = this->test_score_network;
				this->test_score_network = NULL;

				if (this->curr_score_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
					cout << "ending STATE_SCORE_INPUT-STAGE_TUNE" << endl;
					cout << "starting STATE_SCORE_SMALL-STAGE_LEARN" << endl;

					delete this->curr_score_network;
					int input_size = this->curr_scope_sizes.back();
					this->small_score_network = new Network(input_size,
															10*input_size,
															1);

					this->state = STATE_SCORE_SMALL;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_SCORE_INPUT-STAGE_TUNE" << endl;
					cout << "starting STATE_SCORE_INPUT-STAGE_LEARN" << endl;

					this->test_score_network = new SubFoldNetwork(this->curr_score_network);
					this->test_score_network->fold_index++;

					// start from size 1 if have previous
					if (this->score_input_networks.size() > 0) {
						this->score_input_layer.push_back(this->test_score_network->fold_index);
						this->score_input_sizes.push_back(1);
						int input_size = this->curr_scope_sizes[this->test_score_network->fold_index];
						this->score_input_networks.push_back(new Network(input_size,
																		 10*input_size,
																		 1));
						this->test_score_network->add_state(this->test_score_network->fold_index+1,
															1);
						// add to curr_fold permanently 1 at a time
						this->curr_scope_sizes[this->test_score_network->fold_index+1]++;
						this->curr_fold->add_state(this->test_score_network->fold_index+1, 1);
					}

					this->state = STATE_SCORE_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			}
		}
	} else if (this->state == STATE_SCORE_SMALL
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_SCORE_SMALL-STAGE_LEARN" << endl;
			cout << "starting STATE_SCORE_SMALL-STAGE_TUNE" << endl;

			this->state = STATE_SCORE_SMALL;
			this->stage = STAGE_TUNE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
			this->best_sum_error = -1.0;
		}
	} else if (this->state == STATE_SCORE_SMALL
			&& this->stage == STAGE_TUNE) {
		if (this->stage_iter >= 40000) {
			bool done = false;
			if (this->sum_error/40000 < this->max_needed_error) {
				done = true;
			} else if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->stage_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try++;
				} else {
					done = true;
				}
			}

			if (done) {
				cout << "ending STATE_SCORE_SMALL-STAGE_TUNE" << endl;
				cout << "starting STATE_COMPRESS_STATE-STAGE_LEARN" << endl;

				int sum_scope_sizes = 0;
				for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
					sum_scope_sizes += this->curr_scope_sizes[sc_index];
				}

				this->test_scope_sizes = this->curr_scope_sizes;
				this->compress_num_layers = (int)this->curr_scope_sizes.size();
				this->compressed_scope_sizes = vector<int>(this->curr_scope_sizes.size());
				for (int sc_index = (int)this->compressed_scope_sizes.size()-1; sc_index >= 0; sc_index--) {
					this->compressed_scope_sizes[sc_index] = this->test_scope_sizes.back();
					this->test_scope_sizes.pop_back();
				}
				this->compress_size = 1;
				this->compress_new_size = sum_scope_sizes-1;

				if (this->compress_new_size == 0) {
					this->test_fold = new FoldNetwork(this->curr_fold);
					while (this->test_fold->state_inputs.size() > 0) {
						this->test_fold->pop_scope();
					}
				} else {
					this->test_scope_sizes.push_back(this->compress_new_size);

					// use curr_scope_sizes for construction
					this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																		this->compress_new_size);

					this->test_fold = new FoldNetwork(this->curr_fold);
					while (this->test_fold->state_inputs.size() > 0) {
						this->test_fold->pop_scope();
					}
					this->test_fold->add_scope(this->compress_new_size);
				}

				this->state = STATE_COMPRESS_STATE;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_STATE
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_COMPRESS_STATE-STAGE_LEARN" << endl;
			cout << "starting STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;

			this->state = STATE_COMPRESS_STATE;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_STATE
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < this->max_allowable_error) {
				if (this->curr_compression_network != NULL) {
					delete this->curr_compression_network;
				}
				// may be NULL
				this->curr_compression_network = this->test_compression_network;
				this->test_compression_network = NULL;

				// update curr_fold without updating curr_scope_sizes
				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->compress_new_size == 0) {
					cout << "ending STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_FINAL_TUNE-STAGE_TUNE" << endl;

					this->curr_scope_sizes = this->test_scope_sizes;

					this->state = STATE_FINAL_TUNE;
					this->stage = STAGE_TUNE;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					cout << "ending STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_STATE-STAGE_LEARN" << endl;

					this->compress_size++;
					this->compress_new_size--;

					if (this->compress_new_size == 0) {
						this->test_scope_sizes.pop_back();

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
					} else {
						this->test_scope_sizes.back()--;

						// use curr_scope_sizes for construction
						this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																			this->compress_new_size);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compress_new_size);
					}

					this->state = STATE_COMPRESS_STATE;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			} else {
				if (this->test_compression_network != NULL) {
					delete this->test_compression_network;
					this->test_compression_network = NULL;
				}

				delete this->test_fold;
				this->test_fold = NULL;

				// undo previous increment
				if (this->compress_new_size == 0) {
					this->test_scope_sizes.push_back(1);
				} else {
					this->test_scope_sizes.back()++;
				}
				this->compress_size--;
				this->compress_new_size++;

				if (this->compress_size == 0) {
					cout << "ending STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_FINAL_TUNE-STAGE_TUNE" << endl;

					this->compress_num_layers = 0;
					this->compress_new_size = 0;
					this->compressed_scope_sizes.clear();

					this->state = STATE_FINAL_TUNE;
					this->stage = STAGE_TUNE;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < this->compress_num_layers-1; sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[this->curr_scope_sizes.size()-1-sc_index];
					}
					if (this->compress_size > sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SMALL-STAGE_LEARN" << endl;

						this->curr_scope_sizes = this->test_scope_sizes;

						delete this->curr_compression_network;
						int input_size = 0;
						for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.size(); sc_index++) {
							input_size += this->compressed_scope_sizes[sc_index];
						}
						this->small_compression_network = new Network(input_size,
																	  10*input_size,
																	  this->compress_new_size);

						this->state = STATE_COMPRESS_SMALL;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					} else if (this->compress_size == sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE-STAGE_LEARN" << endl;

						this->compress_num_layers--;
						this->compress_new_size = 0;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					} else {
						cout << "ending STATE_COMPRESS_STATE-STAGE_MEASURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE-STAGE_LEARN" << endl;

						this->compress_num_layers--;
						this->compress_new_size = sum_scope_sizes - this->compress_size;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);
						this->test_scope_sizes.push_back(this->compress_new_size);

						// use curr_scope_sizes for construction
						this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																			this->compress_new_size);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);
						this->test_fold->add_scope(this->compress_new_size);

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					}
				}
			}
		}
	} else if (this->state == STATE_COMPRESS_SCOPE
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_COMPRESS_SCOPE-STAGE_LEARN" << endl;
			cout << "starting STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;

			this->state = STATE_COMPRESS_SCOPE;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_SCOPE
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < this->max_allowable_error) {
				if (this->curr_compression_network != NULL) {
					delete this->curr_compression_network;
				}
				// may be NULL
				this->curr_compression_network = this->test_compression_network;
				this->test_compression_network = NULL;

				// update curr_fold without updating curr_scope_sizes
				delete this->curr_fold;
				this->curr_fold = this->test_fold;
				this->test_fold = NULL;

				if (this->compress_new_size == 0) {
					cout << "ending STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_FINAL_TUNE-STAGE_TUNE" << endl;

					this->curr_scope_sizes = this->test_scope_sizes;

					this->state = STATE_FINAL_TUNE;
					this->stage = STAGE_TUNE;
					this->stage_iter = 0;
					this->sum_error = 0.0;
					this->best_sum_error = -1.0;
				} else {
					int sum_scope_sizes = 0;
					for (int sc_index = 0; sc_index < this->compress_num_layers-1; sc_index++) {
						sum_scope_sizes += this->curr_scope_sizes[this->curr_scope_sizes.size()-1-sc_index];
					}
					if (this->compress_size > sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_INPUT_STAGE_LEARN" << endl;

						// this->curr_scope_sizes > 1
						this->curr_scope_sizes = this->test_scope_sizes;

						this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
						this->test_compression_network->fold_index++;

						this->state = STATE_COMPRESS_INPUT;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					} else if (this->compress_size == sum_scope_sizes) {
						cout << "ending STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE-STAGE_LEARN" << endl;

						this->compress_num_layers--;
						this->compress_new_size = 0;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					} else {
						cout << "ending STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;
						cout << "error: " << this->sum_error/10000 << endl;
						cout << "starting STATE_COMPRESS_SCOPE-STAGE_LEARN" << endl;

						this->compress_num_layers--;
						this->compress_new_size = sum_scope_sizes - this->compress_size;

						this->test_scope_sizes.pop_back();
						this->test_scope_sizes.push_back(this->compressed_scope_sizes[0]);
						this->test_scope_sizes.push_back(this->compress_new_size);

						// use curr_scope_sizes for construction
						this->test_compression_network = new SubFoldNetwork(this->curr_scope_sizes,
																			this->compress_new_size);

						this->test_fold = new FoldNetwork(this->curr_fold);
						this->test_fold->pop_scope();
						this->test_fold->add_scope(this->compressed_scope_sizes[0]);
						this->test_fold->add_scope(this->compress_new_size);

						this->compressed_scope_sizes.erase(this->compressed_scope_sizes.begin());

						this->state = STATE_COMPRESS_SCOPE;
						this->stage = STAGE_LEARN;
						this->stage_iter = 0;
						this->sum_error = 0.0;
					}
				}
			} else {
				if (this->test_compression_network != NULL) {
					delete this->test_compression_network;
					this->test_compression_network = NULL;
				}

				delete this->test_fold;
				this->test_fold = NULL;

				// undo previous uncompressed scope
				this->compress_num_layers++;
				if (this->compress_new_size > 0) {
					this->test_scope_sizes.pop_back();
				}
				this->compress_new_size += this->test_scope_sizes.back();
				this->compressed_scope_sizes.insert(this->compressed_scope_sizes.begin(), this->test_scope_sizes.back());
				this->test_scope_sizes.pop_back();
				this->test_scope_sizes.push_back(this->compress_new_size);

				this->curr_scope_sizes = this->test_scope_sizes;

				if (this->curr_scope_sizes.size() == 1) {
					cout << "ending STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_SMALL-STAGE_LEARN" << endl;

					delete this->curr_compression_network;
					int input_size = 0;
					for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.size(); sc_index++) {
						input_size += this->compressed_scope_sizes[sc_index];
					}
					this->small_compression_network = new Network(input_size,
																  10*input_size,
																  this->compress_new_size);

					this->state = STATE_COMPRESS_SMALL;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_COMPRESS_SCOPE-STAGE_MEASURE" << endl;
					cout << "error: " << this->sum_error/10000 << endl;
					cout << "starting STATE_COMPRESS_INPUT-STAGE_LEARN" << endl;

					this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
					this->test_compression_network->fold_index++;

					this->state = STATE_COMPRESS_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			}
		}
	} else if (this->state == STATE_COMPRESS_INPUT
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_COMPRESS_INPUT-STAGE_LEARN" << endl;
			cout << "starting STATE_COMPRESS_INPUT-STAGE_MEASURE" << endl;

			this->state = STATE_COMPRESS_INPUT;
			this->stage = STAGE_MEASURE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
		}
	} else if (this->state == STATE_COMPRESS_INPUT
			&& this->stage == STAGE_MEASURE) {
		if (this->stage_iter >= 10000) {
			if (this->sum_error/10000 < this->max_allowable_error
					|| (this->input_networks.size() > 0 && this->input_sizes.back() == this->curr_scope_sizes[this->input_layer.back()])) {
				cout << "ending STATE_COMPRESS_INPUT-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_COMPRESS_INPUT-STAGE_TUNE" << endl;

				this->state = STATE_COMPRESS_INPUT;
				this->stage = STAGE_TUNE;
				this->stage_iter = 0;
				this->sum_error = 0.0;
				this->best_sum_error = -1.0;
			} else {
				cout << "ending STATE_COMPRESS_INPUT-STAGE_MEASURE" << endl;
				cout << "error: " << this->sum_error/10000 << endl;
				cout << "starting STATE_COMPRESS_INPUT-STAGE_LEARN" << endl;

				delete this->test_compression_network;
				this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
				this->test_compression_network->fold_index++;

				if (this->input_layer.size() > 0
						&& this->input_layer.back() == this->test_compression_network->fold_index) {
					this->input_sizes.back()++;
					delete this->input_networks.back();
					this->input_networks.pop_back();
				} else {
					this->input_layer.push_back(this->test_compression_network->fold_index);
					this->input_sizes.push_back(1);
				}
				int input_size = this->curr_scope_sizes[this->test_compression_network->fold_index];
				this->input_networks.push_back(new Network(input_size,
														   10*input_size,
														   this->input_sizes.back()));
				this->test_compression_network->add_state(this->test_compression_network->fold_index+1,
														  this->input_sizes.back());
				if (this->test_compression_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
					this->compressed_scope_sizes[0]++;
				} else {
					// add to curr_fold permanently 1 at a time
					this->curr_scope_sizes[this->test_compression_network->fold_index+1]++;
					this->curr_fold->add_state(this->test_compression_network->fold_index+1, 1);
				}

				this->state = STATE_COMPRESS_INPUT;
				this->stage = STAGE_LEARN;
				this->stage_iter = 0;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == STATE_COMPRESS_INPUT
			&& this->stage == STAGE_TUNE) {
		if (this->stage_iter >= 40000) {
			bool done = false;
			if (this->sum_error/40000 < this->max_needed_error) {
				done = true;
			} else if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->stage_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try++;
				} else {
					done = true;
				}
			}

			if (done) {
				if (this->curr_compression_network != NULL) {
					delete this->curr_compression_network;
				}
				this->curr_compression_network = this->test_compression_network;
				this->test_compression_network = NULL;

				if (this->curr_compression_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
					cout << "ending STATE_COMPRESS_INPUT-STAGE_TUNE" << endl;
					cout << "starting STATE_COMPRESS_SMALL-STAGE_LEARN" << endl;

					delete this->curr_compression_network;
					int input_size = 0;
					for (int sc_index = 0; sc_index < (int)this->compressed_scope_sizes.size(); sc_index++) {
						input_size += this->compressed_scope_sizes[sc_index];
					}
					this->small_compression_network = new Network(input_size,
																  10*input_size,
																  this->compress_new_size);

					this->state = STATE_COMPRESS_SMALL;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				} else {
					cout << "ending STATE_COMPRESS_INPUT-STAGE_TUNE" << endl;
					cout << "starting STATE_COMPRESS_INPUT-STAGE_LEARN" << endl;

					this->test_compression_network = new SubFoldNetwork(this->curr_compression_network);
					this->test_compression_network->fold_index++;

					// start from size 1 if have previous
					if (this->input_networks.size() > 0) {
						this->input_layer.push_back(this->test_compression_network->fold_index);
						this->input_sizes.push_back(1);
						int input_size = this->curr_scope_sizes[this->test_compression_network->fold_index];
						this->input_networks.push_back(new Network(input_size,
																   10*input_size,
																   1));
						this->test_compression_network->add_state(this->test_compression_network->fold_index+1,
																  1);
						if (this->test_compression_network->fold_index == (int)this->curr_scope_sizes.size()-2) {
							this->compressed_scope_sizes[0]++;
						} else {
							// add to curr_fold permanently 1 at a time
							this->curr_scope_sizes[this->test_compression_network->fold_index+1]++;
							this->curr_fold->add_state(this->test_compression_network->fold_index+1, 1);
						}
					}

					this->state = STATE_COMPRESS_INPUT;
					this->stage = STAGE_LEARN;
					this->stage_iter = 0;
					this->sum_error = 0.0;
				}
			}
		}
	} else if (this->state == STATE_COMPRESS_SMALL
			&& this->stage == STAGE_LEARN) {
		if (this->stage_iter >= 300000) {
			cout << "ending STATE_COMPRESS_SMALL-STAGE_LEARN" << endl;
			cout << "starting STATE_FINAL_TUNE-STAGE_TUNE" << endl;

			this->state = STATE_FINAL_TUNE;
			this->stage = STAGE_TUNE;
			this->stage_iter = 0;
			this->sum_error = 0.0;
			this->best_sum_error = -1.0;
		}
	} else if (this->state == STATE_FINAL_TUNE
			&& this->stage == STAGE_TUNE) {
		if (this->stage_iter >= 40000) {
			bool done = false;
			if (this->sum_error/40000 < this->max_needed_error) {
				done = true;
			} else if (this->best_sum_error == -1.0) {
				this->best_sum_error = this->sum_error;
				this->sum_error = 0.0;
				this->stage_iter = 0;
				this->tune_try = 0;
			} else {
				if (this->sum_error < this->best_sum_error) {
					this->best_sum_error = this->sum_error;
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try = 0;
				} else if (this->tune_try < 2) {
					this->sum_error = 0.0;
					this->stage_iter = 0;
					this->tune_try++;
				} else {
					done = true;
				}
			}

			if (done) {
				cout << "ending STATE_FINAL_TUNE-STAGE_TUNE" << endl;
				cout << "DONE" << endl;

				this->state = STATE_DONE;
			}
		}
	}
}
