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

void Fold::update_score_activate(vector<double>& local_state_vals,
								 vector<double>& input_vals,
								 vector<int>& context_iter,
								 vector<ContextHistory*> context_histories,
								 RunHelper& run_helper,
								 FoldHistory* history) {
	if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		remove_outer_scope_update_score_activate(local_state_vals,
												 input_vals,
												 context_iter,
												 context_histories,
												 run_helper,
												 history);
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_NETWORK) {
		remove_outer_network_update_score_activate(local_state_vals,
												   input_vals,
												   context_iter,
												   context_histories,
												   run_helper,
												   history);
	} else if (this->state == FOLD_STATE_REMOVE_INNER_NETWORK) {
		clean_update_score_activate(local_state_vals,
									input_vals,
									context_iter,
									context_histories,
									run_helper,
									history);
	} else if (this->state == FOLD_STATE_REMOVE_INNER_STATE) {
		clean_update_score_activate(local_state_vals,
									input_vals,
									context_iter,
									context_histories,
									run_helper,
									history);
	} else {
		// this->state == FOLD_STATE_CLEAR_INNER_STATE
		clean_update_score_activate(local_state_vals,
									input_vals,
									context_iter,
									context_histories,
									run_helper,
									history);
	}
}

void Fold::update_sequence_activate(vector<double>& local_state_vals,
									vector<double>& input_vals,
									vector<vector<double>>& flat_vals,
									double& predicted_score,
									double& scale_factor,
									RunHelper& run_helper,
									FoldHistory* history) {
	if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		remove_outer_scope_update_sequence_activate(local_state_vals,
													input_vals,
													flat_vals,
													predicted_score,
													scale_factor,
													run_helper,
													history);
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_NETWORK) {
		remove_outer_network_update_sequence_activate(local_state_vals,
													  input_vals,
													  flat_vals,
													  predicted_score,
													  scale_factor,
													  run_helper,
													  history);
	} else if (this->state == FOLD_STATE_REMOVE_INNER_NETWORK) {
		clean_update_sequence_activate(local_state_vals,
									   input_vals,
									   flat_vals,
									   predicted_score,
									   scale_factor,
									   run_helper,
									   history);
	} else if (this->state == FOLD_STATE_REMOVE_INNER_STATE) {
		clean_update_sequence_activate(local_state_vals,
									   input_vals,
									   flat_vals,
									   predicted_score,
									   scale_factor,
									   run_helper,
									   history);
	} else {
		// this->state == FOLD_STATE_CLEAR_INNER_STATE
		clean_update_sequence_activate(local_state_vals,
									   input_vals,
									   flat_vals,
									   predicted_score,
									   scale_factor,
									   run_helper,
									   history);
	}
}

void Fold::update_backprop(double target_val,
						   double final_misguess,
						   double& predicted_score,
						   double& scale_factor,
						   FoldHistory* history) {
	clean_update_backprop(target_val,
						  final_misguess,
						  predicted_score,
						  scale_factor,
						  history);

	update_increment();
}

void Fold::update_increment() {
	this->state_iter++;

	if (this->state == FOLD_STATE_REMOVE_OUTER_SCOPE) {
		if (this->state_iter == 150000) {
			remove_outer_scope_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_OUTER_NETWORK) {
		if (this->state_iter == 150000) {
			remove_outer_network_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_NETWORK) {
		if (this->state_iter == 150000) {
			remove_inner_network_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == FOLD_STATE_REMOVE_INNER_STATE) {
		if (this->state_iter == 150000) {
			remove_inner_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else {
		// this->state == FOLD_STATE_CLEAR_INNER_STATE
		if (this->state_iter == 150000) {
			clear_inner_state_end();
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	}
}

FoldHistory::FoldHistory(Fold* fold) {
	this->fold = fold;

	this->starting_score_network_history = NULL;
	this->test_starting_score_network_history = NULL;
}

FoldHistory::~FoldHistory() {
	for (int n_index = 0; n_index < this->outer_state_network_histories.size(); n_index++) {
		for (int o_index = 0; o_index < (int)this->outer_state_network_histories[n_index].size(); o_index++) {
			if (this->outer_state_network_histories[n_index][o_index] != NULL) {
				delete this->outer_state_network_histories[n_index][o_index];
			}
		}
	}

	for (int n_index = 0; n_index < this->test_outer_state_network_histories.size(); n_index++) {
		for (int o_index = 0; o_index < (int)this->test_outer_state_network_histories[n_index].size(); o_index++) {
			if (this->test_outer_state_network_histories[n_index][o_index] != NULL) {
				delete this->test_outer_state_network_histories[n_index][o_index];
			}
		}
	}

	for (int f_index = 0; f_index < (int)this->state_network_histories.size(); f_index++) {
		for (int s_index = 0; s_index < (int)this->state_network_histories[f_index].size(); s_index++) {
			if (this->state_network_histories[f_index][s_index] != NULL) {
				delete this->state_network_histories[f_index][s_index];
			}
		}
	}

	for (int f_index = 0; f_index < (int)this->inner_scope_histories.size(); f_index++) {
		if (this->inner_scope_histories[f_index] != NULL) {
			delete this->inner_scope_histories[f_index];
		}
	}

	for (int f_index = 0; f_index < (int)this->score_network_histories.size(); f_index++) {
		if (this->score_network_histories[f_index] != NULL) {
			delete this->score_network_histories[f_index];
		}
	}
}
