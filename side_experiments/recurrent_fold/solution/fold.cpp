#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

Fold::Fold(vector<int> scope_context,
		   vector<int> node_context,
		   int exit_depth,
		   int sequence_length,
		   vector<bool> is_inner_scope,
		   vector<int> existing_scope_ids,
		   int existing_sequence_length,
		   double* existing_average_score,
		   double* existing_score_variance,
		   double* existing_predicted_score_variance,
		   double* existing_average_misguess,
		   double* existing_misguess_variance) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->exit_depth = exit_depth;

	this->sequence_length = sequence_length;
	this->is_inner_scope = is_inner_scope;
	this->existing_scope_ids = existing_scope_ids;

	this->existing_sequence_length = existing_sequence_length;
	this->existing_average_score = existing_average_score;
	this->existing_score_variance = existing_score_variance;
	this->existing_predicted_score_variance = existing_predicted_score_variance;
	this->existing_average_misguess = existing_average_misguess;
	this->existing_misguess_variance = existing_misguess_variance;

	Scope* score_scope = solution->scopes[0];
	this->num_score_local_states = score_scope->num_local_states;
	this->num_score_input_states = score_scope->num_input_states;
	Scope* sequence_scope = solution->scopes[scope_context[this->exit_depth]];
	this->num_sequence_local_states = sequence_scope->num_local_states;
	this->num_sequence_input_states = sequence_scope->num_input_states;

	this->sum_inner_inputs = 0;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->is_inner_scope[f_index]) {
			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			this->inner_input_start_indexes.push_back(this->sum_inner_inputs);
			this->num_inner_inputs.push_back(inner_scope->num_input_states);
			this->sum_inner_inputs += inner_scope->num_input_states;
		} else {
			this->inner_input_start_indexes.push_back(-1);
			this->num_inner_inputs.push_back(-1);
		}
	}

	this->test_num_new_outer_states = 1;
	// test_outer_state_networks starts empty
	this->test_starting_score_network = new StateNetwork(1,
														 this->num_score_local_states,
														 this->num_score_input_states,
														 0,
														 this->test_num_new_outer_states,
														 20);

	this->test_num_new_inner_states = 1;
	int total_num_states = this->sum_inner_inputs
		+ this->test_num_new_inner_states
		+ this->num_sequence_local_states
		+ this->num_sequence_input_states
		+ this->test_num_new_outer_states;
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		this->test_state_networks.push_back(vector<StateNetwork*>());
		if (this->is_inner_scope[f_index]) {
			Scope* inner_scope = solution->scopes[this->existing_scope_ids[f_index]];
			for (int s_index = 0; s_index < total_num_states; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(0,
																			  this->num_sequence_local_states,
																			  this->num_sequence_input_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->test_num_new_outer_states,
																			  20));
			}
		} else {
			for (int s_index = 0; s_index < total_num_states; s_index++) {
				this->test_state_networks[f_index].push_back(new StateNetwork(1,
																			  this->num_sequence_local_states,
																			  this->num_sequence_input_states,
																			  this->sum_inner_inputs+this->test_num_new_inner_states,
																			  this->test_num_new_outer_states,
																			  20));
			}
		}

		this->test_score_networks.push_back(new StateNetwork(0,
															 this->num_sequence_local_states,
															 this->num_sequence_input_states,
															 this->sum_inner_inputs+this->test_num_new_inner_states,
															 this->test_num_new_outer_states,
															 20));
	}

	this->new_noticably_better = 0;
	this->existing_noticably_better = 0;

	this->is_recursive = 0;

	this->test_average_score = 0.0;
	this->test_score_variance = 0.0;
	this->test_average_misguess = 0.0;
	this->test_misguess_variance = 0.0;

	this->state = STATE_EXPLORE;
	this->state_iter = 0;
	this->sum_error = 0.0;
}

Fold::~Fold() {
	// do nothing
}

// void Fold::clean_activate(vector<vector<double>>& flat_vals,
// 						  double& predicted_score) {
// 	vector<double> curr_state_vals(this->curr_num_states, 0.0);
// 	vector<double> test_state_vals(this->curr_num_states, 0.0);

// 	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_input = *flat_vals.begin();
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (this->curr_state_not_needed_locally[f_index][i_index]) {
// 						state_network_input.push_back(0.0);
// 					} else {
// 						state_network_input.push_back(curr_state_vals[i_index]);
// 					}
// 				}
// 				this->curr_state_networks[f_index][s_index]->activate(state_network_input);
// 				curr_state_vals[s_index] += this->curr_state_networks[f_index][s_index]->output->acti_vals[0];
// 			}
// 		}
// 		for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 			if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_input = *flat_vals.begin();
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (this->test_state_not_needed_locally[f_index][i_index]) {
// 						state_network_input.push_back(0.0);
// 					} else {
// 						state_network_input.push_back(test_state_vals[i_index]);
// 					}
// 				}
// 				this->test_state_networks[f_index][s_index]->activate(state_network_input);
// 				test_state_vals[s_index] += this->test_state_networks[f_index][s_index]->output->acti_vals[0];
// 			}
// 		}
// 		flat_vals.erase(flat_vals.begin());

// 		vector<double> curr_score_network_input;
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (this->curr_state_not_needed_locally[f_index][i_index]) {
// 				curr_score_network_input.push_back(0.0);
// 			} else {
// 				curr_score_network_input.push_back(curr_state_vals[i_index]);
// 			}
// 		}
// 		this->curr_score_networks[f_index]->activate(curr_score_network_input);
// 		predicted_score += this->curr_score_networks[f_index]->output->acti_vals[0];

// 		vector<double> test_score_network_input;
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (this->test_state_not_needed_locally[f_index][i_index]) {
// 				test_score_network_input.push_back(0.0);
// 			} else {
// 				test_score_network_input.push_back(test_state_vals[i_index]);
// 			}
// 		}
// 		this->test_score_networks[f_index]->activate(test_score_network_input);

// 		for (int s_index = 0; s_index < this->curr_num_states_cleared[f_index]; s_index++) {
// 			curr_state_vals[s_index] = 0.0;
// 		}
// 		for (int s_index = 0; s_index < this->test_num_states_cleared[f_index]; s_index++) {
// 			test_state_vals[s_index] = 0.0;
// 		}
// 	}
// }

// void Fold::clean_backprop(double target_val,
// 						  double final_misguess,
// 						  double& predicted_score) {
// 	vector<double> curr_state_errors(this->curr_num_states, 0.0);
// 	vector<double> test_state_errors(this->curr_num_states, 0.0);

// 	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
// 		double test_score_network_error = this->curr_score_networks[f_index]->output->acti_vals[0]
// 			- this->test_score_networks[f_index]->output->acti_vals[0];
// 		this->sum_error += abs(test_score_network_error);
// 		vector<double> test_score_network_errors{test_score_network_error};
// 		if (this->state_iter <= 130000) {
// 			this->test_score_networks[f_index]->backprop(test_score_network_errors, 0.01);
// 		} else {
// 			this->test_score_networks[f_index]->backprop(test_score_network_errors, 0.002);
// 		}
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (!this->test_state_not_needed_locally[f_index][i_index]) {
// 				test_state_errors[i_index] += this->test_score_networks[f_index]->input->errors[i_index];
// 				this->test_score_networks[f_index]->input->errors[i_index] = 0.0;
// 			}
// 		}

// 		vector<double> curr_score_network_errors{target_val - predicted_score};
// 		this->curr_score_networks[f_index]->backprop(curr_score_network_errors, 0.002);
// 		for (int i_index = 0; i_index < this->curr_num_states; i_index++) {
// 			if (!this->curr_state_not_needed_locally[f_index][i_index]) {
// 				curr_state_errors[i_index] += this->curr_score_networks[f_index]->input->errors[i_index];
// 				this->curr_score_networks[f_index]->input->errors[i_index] = 0.0;
// 			}
// 		}

// 		predicted_score -= this->curr_score_networks[f_index]->output->acti_vals[0];

// 		for (int s_index = this->curr_num_states-1; s_index >= 0; s_index--) {
// 			if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_errors{test_state_errors[s_index]};
// 				if (this->state_iter <= 130000) {
// 					this->test_state_networks[f_index][s_index]->backprop(state_network_errors, 0.01);
// 				} else {
// 					this->test_state_networks[f_index][s_index]->backprop(state_network_errors, 0.002);
// 				}
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (!this->test_state_not_needed_locally[f_index][i_index]) {
// 						test_state_errors[i_index] += this->test_state_networks[f_index][s_index]->input->errors[1+i_index];
// 						this->test_state_networks[f_index][s_index]->input->errors[1+i_index] = 0.0;
// 					}
// 				}
// 			}
// 		}

// 		for (int s_index = this->curr_num_states-1; s_index >= 0; s_index--) {
// 			if (!this->curr_state_networks_not_needed[f_index][s_index]) {
// 				vector<double> state_network_errors{curr_state_errors[s_index]};
// 				this->curr_state_networks[f_index][s_index]->backprop(state_network_errors, 0.002);
// 				for (int i_index = 0; i_index < s_index+1; i_index++) {
// 					if (!this->curr_state_not_needed_locally[f_index][i_index]) {
// 						curr_state_errors[i_index] += this->curr_state_networks[f_index][s_index]->input->errors[1+i_index];
// 						this->curr_state_networks[f_index][s_index]->input->errors[1+i_index] = 0.0;
// 					}
// 				}
// 			}
// 		}
// 	}

// 	clean_increment();
// }

// void Fold::clean_increment() {
// 	this->state_iter++;

// 	if (this->state_iter == 150000) {
// 		bool step_success = this->sum_error/this->sequence_length/10000 < 0.01;
// 		if (step_success) {
// 			cout << "SUCCESS" << endl;

// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					if (!this->curr_state_networks_not_needed[f_index][s_index]) {
// 						delete this->curr_state_networks[f_index][s_index];
// 					}
// 				}

// 				delete this->curr_score_networks[f_index];
// 			}

// 			this->curr_state_networks_not_needed = this->test_state_networks_not_needed;
// 			this->curr_state_not_needed_locally = this->test_state_not_needed_locally;
// 			this->curr_num_states_cleared = this->test_num_states_cleared;
// 			this->curr_state_networks = this->test_state_networks;
// 			this->curr_score_networks = this->test_score_networks;
// 		} else {
// 			cout << "FAILURE" << endl;

// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 						delete this->test_state_networks[f_index][s_index];
// 					}
// 				}

// 				delete this->test_score_networks[f_index];
// 			}
// 		}

// 		this->test_state_networks_not_needed = this->curr_state_networks_not_needed;
// 		this->test_state_not_needed_locally = this->curr_state_not_needed_locally;
// 		this->test_num_states_cleared = this->curr_num_states_cleared;

// 		bool next_step_done = false;
// 		while (!next_step_done) {
// 			if (this->state == STATE_REMOVE_NETWORK) {
// 				if (this->clean_state_index == 0) {
// 					this->state = STATE_REMOVE_STATE;
// 					this->clean_state_index = this->curr_num_states-1;

// 					if (this->curr_state_networks_not_needed[this->clean_step_index][this->clean_state_index]) {
// 						cout << "STATE_REMOVE_STATE " << this->clean_step_index << " " << this->clean_state_index << endl;

// 						next_step_done = true;
// 						this->test_state_not_needed_locally[this->clean_step_index][this->clean_state_index] = true;
// 					}
// 				} else {
// 					this->clean_state_index--;

// 					cout << "STATE_REMOVE_NETWORK " << this->clean_step_index << " " << this->clean_state_index << endl;

// 					next_step_done = true;
// 					this->test_state_networks_not_needed[this->clean_step_index][this->clean_state_index] = true;
// 				}
// 			} else if (this->state == STATE_REMOVE_STATE) {
// 				if (this->clean_state_index == 0) {
// 					this->state = STATE_CLEAR_STATE;

// 					if (this->clean_step_index == 0) {
// 						// do nothing -- this->curr_num_states_cleared[0] = 0
// 					} else {
// 						// just a small optimization that doesn't impact results
// 						this->curr_num_states_cleared[this->clean_step_index] = this->curr_num_states_cleared[this->clean_step_index-1];
// 						for (int s_index = 0; s_index < this->curr_num_states_cleared[this->clean_step_index]; s_index++) {
// 							if (!this->curr_state_networks_not_needed[this->clean_step_index][s_index]) {
// 								this->curr_num_states_cleared[this->clean_step_index] = s_index;
// 								break;
// 							}
// 						}
// 					}

// 					// don't worry about clean_state_index for STATE_CLEAR_STATE

// 					if (this->curr_num_states_cleared[this->clean_step_index] == this->curr_num_states-1) {
// 						// let next section handle
// 					} else {
// 						cout << "STATE_CLEAR_STATE " << this->clean_step_index << " " << this->test_num_states_cleared[this->clean_step_index] << endl;

// 						next_step_done = true;
// 						this->test_num_states_cleared[this->clean_step_index]++;
// 					}
// 				} else {
// 					this->clean_state_index--;

// 					if (this->curr_state_networks_not_needed[this->clean_step_index][this->clean_state_index]) {
// 						cout << "STATE_REMOVE_STATE " << this->clean_step_index << " " << this->clean_state_index << endl;

// 						next_step_done = true;
// 						this->test_state_not_needed_locally[this->clean_step_index][this->clean_state_index] = true;
// 					}
// 				}
// 			} else {
// 				// this->state == STATE_CLEAR_STATE
// 				if (!step_success || this->curr_num_states_cleared[this->clean_step_index] == this->curr_num_states-1) {
// 					if (this->clean_step_index == this->sequence_length-1) {
// 						cout << "STATE_DONE" << endl;

// 						next_step_done = true;
// 						this->state = STATE_DONE;
// 					} else {
// 						this->state = STATE_REMOVE_NETWORK;

// 						this->clean_step_index++;
// 						this->clean_state_index = this->curr_num_states-1;

// 						cout << "STATE_REMOVE_NETWORK " << this->clean_step_index << " " << this->clean_state_index << endl;

// 						next_step_done = true;
// 						this->test_state_networks_not_needed[this->clean_step_index][this->clean_state_index] = true;
// 					}
// 				} else {
// 					// TODO: potentially speed up by checking both previous num_states_cleared and state_networks_not_needed
// 					cout << "STATE_CLEAR_STATE " << this->clean_step_index << " " << this->test_num_states_cleared[this->clean_step_index] << endl;

// 					next_step_done = true;
// 					this->test_num_states_cleared[this->clean_step_index]++;
// 				}
// 			}
// 		}

// 		if (this->state != STATE_DONE) {
// 			for (int f_index = 0; f_index < this->sequence_length; f_index++) {
// 				for (int s_index = 0; s_index < this->curr_num_states; s_index++) {
// 					if (!this->test_state_networks_not_needed[f_index][s_index]) {
// 						this->test_state_networks[f_index][s_index] = new Network(this->curr_state_networks[f_index][s_index]);
// 					} else {
// 						this->test_state_networks[f_index][s_index] = NULL;
// 					}
// 				}

// 				this->test_score_networks[f_index] = new Network(this->curr_score_networks[f_index]);
// 			}
// 		}

// 		this->state_iter = 0;
// 		this->sum_error = 0.0;
// 	} else {
// 		if (this->state_iter%10000 == 0) {
// 			cout << "this->state_iter: " << this->state_iter << endl;
// 			cout << "this->sum_error: " << this->sum_error << endl;
// 			cout << endl;
// 			this->sum_error = 0.0;
// 		}
// 	}
// }

FoldHistory::FoldHistory(Fold* fold) {
	this->fold = fold;

	// TODO: special case curr vs. test
	int num_total_states = fold->sum_inner_inputs
		+ fold->curr_num_new_inner_states
		+ fold->num_sequence_local_states
		+ fold->num_sequence_input_states
		+ fold->curr_num_new_outer_states;
	this->state_network_histories = vector<vector<StateNetworkHistory*>>(
		fold->sequence_length, vector<StateNetworkHistory*>(num_total_states, NULL));
	this->inner_scope_histories = vector<ScopeHistory*>(fold->sequence_length, NULL);
	this->score_network_updates = vector<double>(fold->sequence_length);
	this->score_network_histories = vector<StateNetworkHistory*>(fold->sequence_length, NULL);
}
