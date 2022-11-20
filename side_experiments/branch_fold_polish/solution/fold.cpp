#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

Fold::Fold(int sequence_length,
		   vector<Scope*> existing_actions,
		   vector<int> obs_sizes,
		   int output_size,
		   // TODO: flip state and s_input order everywhere else
		   int starting_s_input_size,
		   int starting_state_size) {
	this->sequence_length = sequence_length;
	this->obs_sizes = obs_sizes;	// for now, -1 also signals is existing action
	this->existing_actions = existing_actions;
	this->output_size = output_size;

	this->starting_score_network = new FoldNetwork(1,
												   starting_s_input_size,
												   starting_state_size,
												   20);

	// don't worry about starting_compress_network initially

	this->scope_average_mod_calcs = vector<Network*>(this->sequence_length, NULL);
	this->scope_scale_mod_calcs = vector<Network*>(this->sequence_length, NULL);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->scope_average_mod_calcs[f_index] = new Network(0, 0, 1);
			this->scope_scale_mod_calcs[f_index] = new Network(0, 0, 1);
		}
	}

	this->end_average_mod_calc = new Network(0, 0, 1);
	this->end_scale_mod_calc = new Network(0, 0, 1);

	this->curr_s_input_sizes.push_back(starting_s_input_size);
	this->curr_scope_sizes.push_back(starting_state_size);

	vector<double> flat_sizes;
	vector<vector<double>> input_flat_sizes(this->sequence_length);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		int flat_size;
		if (this->existing_actions[f_index] == NULL) {
			flat_size = this->obs_sizes[f_index];
		} else {
			flat_size = this->existing_actions[f_index]->num_outputs;
		}
		flat_sizes.push_back(flat_size);
		for (int ff_index = f_index+1; ff_index < this->sequence_length; ff_index++) {
			if (this->existing_actions[ff_index] == NULL) {
				input_flat_sizes[ff_index].push_back(flat_size);
			}
		}
	}

	this->curr_fold = new FoldNetwork(flat_sizes,
									  1,
									  starting_s_input_size,
									  starting_state_size,
									  100);
	this->curr_scope_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			this->curr_scope_input_folds[f_index] = new FoldNetwork(input_flat_sizes[f_index],
																	this->existing_actions[f_index]->num_inputs,
																	starting_s_input_size,
																	starting_state_size,
																	50);
		}
	}
	this->curr_end_fold = new FoldNetwork(flat_sizes,
										  this->output_size,
										  starting_s_input_size,
										  starting_state_size,
										  50);

	this->test_fold = NULL;
	this->test_input_folds = vector<FoldNetwork*>(this->sequence_length, NULL);
	this->test_end_fold = NULL;

	this->phase = PHASE_FLAT;
	this->stage_iter = 0;
	this->sum_error = 0.0;
}

Fold::~Fold() {
	if (this->starting_score_network != NULL) {
		delete this->starting_score_network;
	}
	if (this->starting_compress_network != NULL) {
		delete this->starting_compress_network;
	}

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			if (this->scope_average_mod_calcs[f_index] != NULL) {
				delete this->scope_average_mod_calcs[f_index];
			}
			if (this->scope_scale_mod_calcs[f_index] != NULL) {
				delete this->scope_scale_mod_calcs[f_index];
			}
		}
	}

	if (this->end_average_mod_calc != NULL) {
		delete this->end_average_mod_calc;
	}
	if (this->end_scale_mod_calc != NULL) {
		delete this->end_scale_mod_calc;
	}

	if (this->curr_fold != NULL) {
		delete this->curr_fold;
	}
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			if (this->curr_input_folds[f_index] != NULL) {
				delete this->curr_input_folds[f_index];
			}
		}
	}
	if (this->curr_end_fold != NULL) {
		delete this->curr_end_fold;
	}

	if (this->test_fold != NULL) {
		delete this->test_fold;
	}
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		if (this->existing_actions[f_index] != NULL) {
			if (this->test_input_folds[f_index] != NULL) {
				delete this->test_input_folds[f_index];
			}
		}
	}
	if (this->test_end_fold != NULL) {
		delete this->test_end_fold;
	}
}

void Fold::activate(vector<vector<double>>& flat_vals,
					vector<double>& local_s_input_vals,
					vector<double>& local_state_vals,
					vector<double>& output_state_vals,
					double& predicted_score,
					double& scale_factor,
					double& new_scale_factor) {
	if (this->phase == PHASE_FLAT) {
		flat_step_activate(flat_vals,
						   local_s_input_vals,
						   local_state_vals,
						   output_state_vals,
						   predicted_score,
						   scale_factor,
						   new_scale_factor);
	} else {
		switch(this->state) {
			case STATE_INNER_SCOPE_INPUT_INPUT:

				break;
			case STATE_INNER_SCOPE_TUNE:

				break;
		}
	}
}

void Fold::backprop() {


	increment();
}
