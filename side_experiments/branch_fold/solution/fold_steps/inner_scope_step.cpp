// TODO: remove

#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::inner_scope_step(vector<vector<double>>& flat_vals,
							vector<vector<vector<double>>>& inner_flat_vals,
							double target_val) {
	// if ((this->stage_iter+1)%10000 == 0) {
	// 	cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
	// 	this->sum_error = 0.0;
	// }

	// vector<vector<double>> fold_input;
	// vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

	// vector<vector<double>> state_vals;
	// vector<vector<double>> s_input_vals;
	// double predicted_score = this->average_score;

	// for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
	// 	if (this->compound_actions[n_index] == NULL) {
	// 		nodes[n_index]->activate(state_vals,
	// 								 s_input_vals,
	// 								 flat_vals[n_index],
	// 								 predicted_score);
	// 	} else {
	// 		nodes[n_index]->activate(state_vals,
	// 								 s_input_vals,
	// 								 inner_flat_vals[n_index],
	// 								 predicted_score);
	// 	}

	// 	fold_input.push_back(vector<double>());	// empty
	// 	for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
	// 		if (this->compound_actions[i_index] != NULL) {
	// 			input_fold_inputs[i_index].push_back(vector<double>());	// empty
	// 		}
	// 	}
	// }

	// vector<double> action_input;
	// if (this->action->num_inputs > 0) {
	// 	for (int i_index = 0; i_index < (int)this->action_input_input_networks.size(); i_index++) {
	// 		this->action_input_input_networks[i_index]->activate(state_vals[this->action_input_input_layer[i_index]],
	// 															 s_input_vals[this->action_input_input_layer[i_index]]);
	// 		for (int s_index = 0; s_index < this->action_input_input_sizes[i_index]; s_index++) {
	// 			s_input_vals[this->action_input_input_layer[i_index]+1].push_back(
	// 				this->action_input_input_networks[i_index]->output->acti_vals[s_index]);
	// 		}
	// 	}

	// 	this->small_action_input_network->activate(state_vals.back(),
	// 											   s_input_vals.back());
	// 	action_input.reserve(this->action->num_inputs);
	// 	for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
	// 		action_input.push_back(this->small_action_input_network->output->acti_vals[i_index]);
	// 	}
	// }
	// this->action->activate(inner_flat_vals[this->nodes.size()],
	// 					   action_input,
	// 					   predicted_score);

	// fold_input.push_back(this->action->outputs);
	// for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
	// 	if (this->compound_actions[i_index] != NULL) {
	// 		input_fold_inputs[i_index].push_back(this->action->outputs);
	// 	}
	// }

	// // // set as predicted_score error
	// // this->sum_error += (target_val-predicted_score)*(target_val-predicted_score);

	// for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
	// 	if (this->compound_actions[f_index] == NULL) {
	// 		fold_input.push_back(flat_vals[f_index]);
	// 		for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
	// 			if (this->compound_actions[i_index] != NULL) {
	// 				input_fold_inputs[i_index].push_back(flat_vals[f_index]);
	// 			}
	// 		}
	// 	} else {
	// 		this->curr_scope_input_folds[f_index]->activate(input_fold_inputs[f_index],
	// 														state_vals);
	// 		vector<double> scope_input(this->compound_actions[f_index]->num_inputs);
	// 		for (int i_index = 0; i_index < this->compound_actions[f_index]->num_inputs; i_index++) {
	// 			scope_input[i_index] = this->curr_scope_input_folds[f_index]->output->acti_vals[i_index];
	// 		}

	// 		this->compound_actions[f_index]->activate(inner_flat_vals[f_index],
	// 												  scope_input,
	// 												  predicted_score);

	// 		fold_input.push_back(this->compound_actions[f_index]->outputs);
	// 		for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
	// 			if (this->compound_actions[i_index] != NULL) {
	// 				input_fold_inputs[i_index].push_back(this->compound_actions[f_index]->outputs);
	// 			}
	// 		}
	// 	}
	// }

	// this->curr_fold->activate(fold_input,
	// 						  state_vals);

	// double combined_score = predicted_score + this->curr_fold->output->acti_vals[0];

	// vector<double> errors;
	// errors.push_back(target_val - combined_score);

	// // temp
	// this->sum_error += errors[0]*errors[0];

	// if (this->stage_iter <= 320000) {
	// 	this->curr_fold->backprop_no_state(errors, 0.01);
	// } else {
	// 	this->curr_fold->backprop_no_state(errors, 0.002);
	// }

	// vector<vector<double>> scope_input_errors(this->sequence_length);
	// for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size(); f_index--) {
	// 	// include this->nodes.size()
	// 	if (this->compound_actions[f_index] != NULL) {
	// 		scope_input_errors[f_index] = vector<double>(this->compound_actions[f_index]->num_outputs, 0.0);
	// 		for (int i_index = 0; i_index < this->compound_actions[f_index]->num_outputs; i_index++) {
	// 			scope_input_errors[f_index][i_index] = this->curr_fold->flat_inputs[f_index]->errors[i_index];
	// 			this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
	// 		}
	// 	}
	// }
	// for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
	// 	if (this->compound_actions[f_index] != NULL) {
	// 		vector<double> scope_output_errors;
	// 		this->compound_actions[f_index]->backprop_loose_errors_with_no_weight_change(
	// 			scope_input_errors[f_index],
	// 			scope_output_errors,
	// 			target_val - combined_score);
	// 		if (this->stage_iter <= 320000) {
	// 			this->curr_scope_input_folds[f_index]->backprop_no_state(scope_output_errors, 0.01);
	// 		} else {
	// 			this->curr_scope_input_folds[f_index]->backprop_no_state(scope_output_errors, 0.002);
	// 		}

	// 		for (int ff_index = f_index-1; ff_index >= (int)this->nodes.size(); ff_index--) {
	// 			// include this->nodes.size()
	// 			if (this->compound_actions[ff_index] != NULL) {
	// 				for (int i_index = 0; i_index < this->compound_actions[ff_index]->num_outputs; i_index++) {
	// 					scope_input_errors[ff_index][i_index] = this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
	// 					this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	// vector<double> action_input_errors;	// doesn't matter
	// if (this->stage_iter <= 240000) {
	// 	this->action->backprop(scope_input_errors[this->nodes.size()],
	// 						   action_input_errors,
	// 						   predicted_score,
	// 						   target_val,
	// 						   0.05);
	// } else if (this->stage_iter <= 320000) {
	// 	this->action->backprop(scope_input_errors[this->nodes.size()],
	// 						   action_input_errors,
	// 						   predicted_score,
	// 						   target_val,
	// 						   0.01);
	// } else {
	// 	this->action->backprop(scope_input_errors[this->nodes.size()],
	// 						   action_input_errors,
	// 						   predicted_score,
	// 						   target_val,
	// 						   0.002);
	// }
}
