#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::obs_step(vector<vector<double>>& flat_vals,
					vector<vector<vector<double>>>& inner_flat_vals,
					double target_val) {
	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	vector<vector<double>> state_vals;
	vector<vector<double>> s_input_vals;
	double predicted_score = this->average_score;

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->compound_actions[n_index] == NULL) {
			nodes[n_index]->activate(state_vals,
									 s_input_vals,
									 flat_vals[n_index],
									 predicted_score);
		} else {
			nodes[n_index]->activate(state_vals,
									 s_input_vals,
									 inner_flat_vals[n_index],
									 predicted_score);
		}

		fold_input.push_back(vector<double>());	// empty
		for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
			if (this->compound_actions[i_index] != NULL) {
				input_fold_inputs[i_index].push_back(vector<double>());	// empty
			}
		}
	}

	vector<double> obs_input;
	if (this->compound_actions[this->nodes.size()] == NULL) {
		obs_input = flat_vals[this->nodes.size()];
	} else {
		vector<double> action_input;
		if (this->action->num_inputs > 0) {
			for (int i_index = 0; i_index < (int)this->action_input_input_networks.size(); i_index++) {
				this->action_input_input_networks[i_index]->activate(state_vals[this->action_input_input_layer[i_index]],
																	 s_input_vals[this->action_input_input_layer[i_index]]);
				for (int s_index = 0; s_index < this->action_input_input_sizes[i_index]; s_index++) {
					s_input_vals[this->action_input_input_layer[i_index]+1].push_back(
						this->action_input_input_networks[i_index]->output->acti_vals[s_index]);
				}
			}

			this->small_action_input_network->activate(state_vals.back(),
													   s_input_vals.back());
			action_input.reserve(this->action->num_inputs);
			for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
				action_input.push_back(this->small_action_input_network->output->acti_vals[i_index]);
			}
		}
		double scope_predicted_score = 0.0;
		this->action->activate(inner_flat_vals[this->nodes.size()],
							   action_input,
							   scope_predicted_score);
		obs_input = this->action->outputs;
		obs_input.push_back(scope_predicted_score);
	}
	fold_input.push_back(vector<double>());	// empty
	for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
		if (this->compound_actions[i_index] != NULL) {
			input_fold_inputs[i_index].push_back(vector<double>());	// empty
		}
	}

	if (this->new_layer_size > 0) {
		this->obs_network->activate(obs_input);
		state_vals.push_back(vector<double>(this->new_layer_size));
		s_input_vals.push_back(vector<double>());
		for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
			state_vals.back()[s_index] = this->new_state_factor*this->obs_network->output->acti_vals[s_index];
		}
	}

	for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->compound_actions[f_index] == NULL) {
			fold_input.push_back(flat_vals[f_index]);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->compound_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(flat_vals[f_index]);
				}
			}
		} else {
			this->test_scope_input_folds[f_index]->activate(input_fold_inputs[f_index],
															state_vals);
			vector<double> scope_input(this->compound_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->compound_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->test_scope_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_predicted_score = 0.0;
			this->compound_actions[f_index]->activate(inner_flat_vals[f_index],
													  scope_input,
													  scope_predicted_score);

			vector<double> scope_outputs = this->compound_actions[f_index]->outputs;
			scope_outputs.push_back(scope_predicted_score);

			fold_input.push_back(scope_outputs);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->compound_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(scope_outputs);
				}
			}
		}
	}

	this->test_fold->activate(fold_input,
							  state_vals);

	vector<double> errors;
	errors.push_back((target_val-predicted_score) - this->test_fold->output->acti_vals[0]);
	this->sum_error += errors[0]*errors[0];

	if (this->stage == STAGE_LEARN) {
		if ((this->stage_iter+1)%10000 == 0) {
			cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		if (this->new_layer_size > 0) {
			// if (this->stage_iter <= 160000) {
			if (this->stage_iter <= 270000) {
				this->test_fold->backprop_last_state(errors, 0.01);
			} else {
				this->test_fold->backprop_last_state(errors, 0.002);
			}

			vector<double> last_scope_state_errors(this->test_scope_sizes.back());
			for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
				last_scope_state_errors[st_index] = this->test_fold->state_inputs.back()->errors[st_index];
				this->test_fold->state_inputs.back()->errors[st_index] = 0.0;
			}

			vector<vector<double>> scope_input_errors(this->sequence_length);
			vector<double> scope_predicted_score_errors(this->sequence_length, 0.0);
			for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
				if (this->compound_actions[f_index] != NULL) {
					scope_input_errors[f_index] = vector<double>(this->compound_actions[f_index]->num_outputs, 0.0);
					for (int i_index = 0; i_index < this->compound_actions[f_index]->num_outputs; i_index++) {
						scope_input_errors[f_index][i_index] = this->test_fold->flat_inputs[f_index]->errors[i_index];
						this->test_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
					}
					scope_predicted_score_errors[f_index] = this->test_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs];
					this->test_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs] = 0.0;
				}
			}
			for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
				if (this->compound_actions[f_index] != NULL) {
					vector<double> scope_output_errors;
					this->compound_actions[f_index]->backprop_loose_errors_with_no_weight_change(
						scope_input_errors[f_index],
						scope_output_errors,
						scope_predicted_score_errors[f_index]);
					// if (this->stage_iter <= 160000) {
					if (this->stage_iter <= 270000) {
						this->test_scope_input_folds[f_index]->backprop_last_state(scope_output_errors, 0.01);
					} else {
						this->test_scope_input_folds[f_index]->backprop_last_state(scope_output_errors, 0.002);
					}

					for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
						last_scope_state_errors[st_index] += this->test_scope_input_folds[f_index]->state_inputs.back()->errors[st_index];
						this->test_scope_input_folds[f_index]->state_inputs.back()->errors[st_index] = 0.0;
					}

					for (int ff_index = f_index-1; ff_index >= (int)this->nodes.size()+1; ff_index--) {
						if (this->compound_actions[ff_index] != NULL) {
							for (int i_index = 0; i_index < this->compound_actions[ff_index]->num_outputs; i_index++) {
								scope_input_errors[ff_index][i_index] += this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
								this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
							}
							scope_predicted_score_errors[ff_index] += this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs];
							this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs] = 0.0;
						}
					}
				}
			}

			// if (this->stage_iter <= 120000) {
			if (this->stage_iter <= 240000) {
				this->obs_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.05);
			// } else if (this->stage_iter <= 160000) {
			} else if (this->stage_iter <= 270000) {
				this->obs_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.01);
			} else {
				this->obs_network->backprop_weights_with_no_error_signal(
					last_scope_state_errors,
					0.002);
			}
		} else {
			// if (this->stage_iter <= 160000) {
			if (this->stage_iter <= 270000) {
				this->test_fold->backprop_no_state(errors, 0.01);
			} else {
				this->test_fold->backprop_no_state(errors, 0.002);
			}

			vector<vector<double>> scope_input_errors(this->sequence_length);
			vector<double> scope_predicted_score_errors(this->sequence_length, 0.0);
			for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
				if (this->compound_actions[f_index] != NULL) {
					scope_input_errors[f_index] = vector<double>(this->compound_actions[f_index]->num_outputs, 0.0);
					for (int i_index = 0; i_index < this->compound_actions[f_index]->num_outputs; i_index++) {
						scope_input_errors[f_index][i_index] = this->test_fold->flat_inputs[f_index]->errors[i_index];
						this->test_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
					}
					scope_predicted_score_errors[f_index] = this->test_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs];
					this->test_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs] = 0.0;
				}
			}
			for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
				if (this->compound_actions[f_index] != NULL) {
					vector<double> scope_output_errors;
					this->compound_actions[f_index]->backprop_loose_errors_with_no_weight_change(
						scope_input_errors[f_index],
						scope_output_errors,
						scope_predicted_score_errors[f_index]);
					// if (this->stage_iter <= 160000) {
					if (this->stage_iter <= 270000) {
						this->test_scope_input_folds[f_index]->backprop_no_state(scope_output_errors, 0.01);
					} else {
						this->test_scope_input_folds[f_index]->backprop_no_state(scope_output_errors, 0.002);
					}

					for (int ff_index = f_index-1; ff_index >= (int)this->nodes.size()+1; ff_index--) {
						if (this->compound_actions[ff_index] != NULL) {
							for (int i_index = 0; i_index < this->compound_actions[ff_index]->num_outputs; i_index++) {
								scope_input_errors[ff_index][i_index] += this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
								this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
							}
							scope_predicted_score_errors[ff_index] += this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs];
							this->test_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs] = 0.0;
						}
					}
				}
			}
		}
	}
}
