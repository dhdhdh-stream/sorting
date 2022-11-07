#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::score_tune_step(vector<vector<double>>& flat_vals,
						   vector<vector<vector<double>>>& inner_flat_vals,
						   double target_val) {
	if ((this->stage_iter+1)%10000 == 0) {
		cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
	}

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
			this->action_input_network->activate(state_vals.back(),
												 s_input_vals.back());
			action_input.reserve(this->action->num_inputs);
			for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
				action_input.push_back(this->action_input_network->output->acti_vals[i_index]);
			}
		}
		this->action->activate(inner_flat_vals[this->nodes.size()],
							   action_input,
							   predicted_score);
		obs_input = this->action->outputs;
	}
	fold_input.push_back(vector<double>());	// empty
	for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
		if (this->compound_actions[i_index] != NULL) {
			input_fold_inputs[i_index].push_back(vector<double>());	// empty
		}
	}

	this->obs_network->activate(obs_input);
	state_vals.push_back(vector<double>(this->new_layer_size));
	s_input_vals.push_back(vector<double>());
	for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
		state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
	}

	for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
		this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]],
													  s_input_vals[this->score_input_layer[i_index]]);
		for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
			s_input_vals[this->score_input_layer[i_index]+1].push_back(
				this->score_input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	this->small_score_network->activate(state_vals.back(),
										s_input_vals.back());
	predicted_score += this->small_score_network->output->acti_vals[0];

	// this->sum_error += (target_val - predicted_score)*(target_val - predicted_score);

	for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->compound_actions[f_index] == NULL) {
			fold_input.push_back(flat_vals[f_index]);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->compound_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(flat_vals[f_index]);
				}
			}
		} else {
			this->curr_scope_input_folds[f_index]->activate(input_fold_inputs[f_index]);
			vector<double> scope_input(this->compound_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->compound_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_scope_input_folds[f_index]->output->acti_vals[i_index];
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

	// predicted_score changes, so fold needs to be updated anyways
	this->curr_fold->activate(fold_input,
							  state_vals);

	vector<double> errors;
	errors.push_back((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);

	// temp
	this->sum_error += errors[0]*errors[0];

	if (this->stage_iter <= 120000) {
		this->curr_fold->backprop_fold(errors, 0.01);
	} else {
		this->curr_fold->backprop_fold(errors, 0.002);
	}

	vector<vector<double>> state_errors(this->curr_scope_sizes.size());
	for (int sc_index = 0; sc_index < (int)this->curr_scope_sizes.size(); sc_index++) {
		state_errors[sc_index].reserve(this->curr_scope_sizes[sc_index]);
		for (int st_index = 0; st_index < this->curr_scope_sizes[sc_index]; st_index++) {
			state_errors[sc_index].push_back(this->curr_fold->state_inputs[sc_index]->errors[st_index]);
			this->curr_fold->state_inputs[sc_index]->errors[st_index] = 0.0;
		}
	}
	vector<vector<double>> s_input_errors;
	s_input_errors.reserve(this->curr_s_input_sizes.size());
	for (int sc_index = 0; sc_index < (int)this->curr_s_input_sizes.size(); sc_index++) {
		s_input_errors.push_back(vector<double>(this->curr_s_input_sizes[sc_index], 0.0));
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	vector<double> scope_predicted_score_errors(this->sequence_length, 0.0);
	for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
		if (this->compound_actions[f_index] != NULL) {
			scope_input_errors[f_index] = vector<double>(this->compound_actions[f_index]->num_outputs, 0.0);
			for (int i_index = 0; i_index < this->compound_actions[f_index]->num_outputs; i_index++) {
				scope_input_errors[f_index][i_index] = this->curr_fold->flat_inputs[f_index]->errors[i_index];
				this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
			}
			scope_predicted_score_errors[f_index] += this->curr_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs];
			this->curr_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs] = 0.0;
		}
	}
	for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
		if (this->compound_actions[f_index] != NULL) {
			vector<double> scope_output_errors;
			this->compound_actions[f_index]->backprop_errors_with_no_weight_change(
				scope_input_errors[f_index],
				scope_output_errors,
				scope_predicted_score_errors[f_index]);
			this->curr_scope_input_folds[f_index]->backprop_fold(scope_output_errors, 0.002);

			for (int sc_index = 0; sc_index < (int)state_errors.size(); sc_index++) {
				for (int st_index = 0; st_index < (int)state_errors[sc_index].size(); st_index++) {
					state_errors[sc_index][st_index] = this->curr_scope_input_folds[f_index]->state_inputs[sc_index]->errors[st_index];
					this->curr_scope_input_folds[f_index]->state_inputs[sc_index]->errors[st_index] = 0.0;
				}
			}

			for (int ff_index = f_index-1; ff_index >= (int)this->nodes.size()+1; ff_index--) {
				if (this->compound_actions[ff_index] != NULL) {
					for (int i_index = 0; i_index < this->compound_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] = this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
					scope_predicted_score_errors[ff_index] += this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs];
					this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs] = 0.0;
				}
			}
		}
	}

	vector<double> score_errors{target_val - predicted_score};
	this->small_score_network->backprop(score_errors, 0.002);
	for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
		state_errors.back()[st_index] += this->small_score_network->state_input->errors[st_index];
		this->small_score_network->state_input->errors[st_index] = 0.0;
	}
	for (int st_index = 0; st_index < (int)s_input_errors.back().size(); st_index++) {
		s_input_errors.back()[st_index] += this->small_score_network->s_input_input->errors[st_index];
		this->small_score_network->s_input_input->errors[st_index] = 0.0;
	}
	predicted_score -= this->small_score_network->output->acti_vals[0];

	for (int i_index = (int)this->score_input_networks.size()-1; i_index >= 0; i_index--) {
		vector<double> score_input_errors(this->score_input_sizes[i_index]);
		for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
			score_input_errors[this->score_input_sizes[i_index]-1-s_index] = s_input_errors[this->score_input_layer[i_index]+1].back();
			s_input_errors[this->score_input_layer[i_index]+1].pop_back();
		}
		this->score_input_networks[i_index]->backprop(score_input_errors, 0.002);
		for (int st_index = 0; st_index < (int)state_errors[this->score_input_layer[i_index]].size(); st_index++) {
			state_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->state_input->errors[st_index];
			this->score_input_networks[i_index]->state_input->errors[st_index] = 0.0;
		}
		for (int st_index = 0; st_index < (int)s_input_errors[this->score_input_layer[i_index]].size(); st_index++) {
			s_input_errors[this->score_input_layer[i_index]][st_index] += this->score_input_networks[i_index]->s_input_input->errors[st_index];
			this->score_input_networks[i_index]->s_input_input->errors[st_index] = 0.0;
		}
	}

	this->obs_network->backprop(state_errors.back(), 0.002);
	state_errors.pop_back();
	s_input_errors.pop_back();

	if (this->compound_actions[this->nodes.size()] != NULL) {
		vector<double> action_input_errors(this->action->num_outputs);
		for (int o_index = 0; o_index < this->action->num_outputs; o_index++) {
			action_input_errors[o_index] = this->obs_network->input->errors[o_index];
			this->obs_network->input->errors[o_index] = 0.0;
		}
		vector<double> action_output_errors;
		this->action->backprop(action_input_errors,
							   action_output_errors,
							   predicted_score,
							   target_val);

		if (this->action->num_inputs > 0) {
			this->action_input_network->backprop(action_output_errors, 0.002);
			for (int st_index = 0; st_index < (int)state_errors.back().size(); st_index++) {
				state_errors.back()[st_index] += this->action_input_network->state_input->errors[st_index];
				this->action_input_network->state_input->errors[st_index] = 0.0;
			}
			for (int st_index = 0; st_index < (int)s_input_errors.back().size(); st_index++) {
				s_input_errors.back()[st_index] += this->action_input_network->s_input_input->errors[st_index];
				this->action_input_network->s_input_input->errors[st_index] = 0.0;
			}
		}
	}

	for (int n_index = (int)nodes.size()-1; n_index >= 0; n_index--) {
		nodes[n_index]->backprop(state_errors,
								 s_input_errors,
								 predicted_score,
								 target_val);
	}
}
