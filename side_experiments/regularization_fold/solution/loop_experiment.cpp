#include "loop_experiment.h"

#include <iostream>

#include "constants.h"
#include "exit_network.h"
#include "score_network.h"
#include "sequence.h"

using namespace std;

LoopExperiment::LoopExperiment(vector<int> scope_context,
							   vector<int> node_context,
							   Sequence* sequence,
							   ScoreNetwork* existing_misguess_network) {
	this->type = EXPERIMENT_TYPE_LOOP;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->sequence = sequence;

	this->existing_misguess_network = existing_misguess_network;

	this->state = EXPERIMENT_STATE_EXPLORE;
}

LoopExperiment::~LoopExperiment() {
	// handle in transforms
}

void LoopExperiment::activate(vector<double>& flat_vals,
							  vector<ForwardContextLayer>& context,
							  RunHelper& run_helper,
							  LoopExperimentHistory* history) {
	if (this->state == EXPERIMENT_STATE_EXPLORE) {
		explore_activate(flat_vals,
						 context,
						 run_helper);
	} else if (this->state == EXPERIMENT_STATE_EXPERIMENT) {
		experiment_activate(flat_vals,
							context,
							run_helper,
							history);
	} else if (this->state == EXPERIMENT_STATE_MEASURE) {
		measure_activate(flat_vals,
						 context,
						 run_helper);
	} else if (this->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		clean_activate(flat_vals,
					   context,
					   run_helper,
					   history);
	} else if (this->state == EXPERIMENT_STATE_SECOND_CLEAN) {
		clean_activate(flat_vals,
					   context,
					   run_helper,
					   history);
	} else {
		wrapup_activate(flat_vals,
						context,
						run_helper,
						history);
	}
}

void LoopExperiment::backprop(vector<BackwardContextLayer>& context,
							  double& scale_factor_error,
							  RunHelper& run_helper,
							  LoopExperimentHistory* history) {
	if (this->state == EXPERIMENT_STATE_EXPERIMENT) {
		experiment_backprop(context,
							run_helper,
							history);
	} else if (this->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		clean_backprop(context,
					   run_helper,
					   history);
	} else if (this->state == EXPERIMENT_STATE_SECOND_CLEAN) {
		clean_backprop(context,
					   run_helper,
					   history);
	} else {
		wrapup_backprop(context,
						scale_factor_error,
						run_helper,
						history);
	}

	// EXPERIMENT_STATE_MEASURE handled outside

	this->state_iter++;
	if (this->state == EXPERIMENT_STATE_EXPERIMENT) {
		if (this->state_iter == 500000) {
			this->state = EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
			this->sum_error = 0.0;

			run_helper.explore_phase = EXPLORE_PHASE_NONE;
			// set on transitions to prevent network backprops
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		if (this->state_iter == 200000) {
			first_clean_transform();

			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else if (this->state == EXPERIMENT_STATE_SECOND_CLEAN) {
		if (this->state_iter == 200000) {
			second_clean_transform();

			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		} else {
			if (this->state_iter%10000 == 0) {
				cout << "this->state_iter: " << this->state_iter << endl;
				cout << "this->sum_error: " << this->sum_error << endl;
				cout << endl;
				this->sum_error = 0.0;
			}
		}
	} else {
		if (this->state_iter == 100000) {
			wrapup_transform();

			run_helper.explore_phase = EXPLORE_PHASE_NONE;
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

LoopExperimentHistory::LoopExperimentHistory(LoopExperiment* experiment) {
	this->experiment = experiment;

	this->halt_score_network_history = NULL;
	this->halt_misguess_network_history = NULL;
}

LoopExperimentHistory::~LoopExperimentHistory() {
	for (int i_index = 0; i_index < (int)this->continue_score_network_histories.size(); i_index++) {
		delete this->continue_score_network_histories[i_index];
	}

	for (int i_index = 0; i_index < (int)this->continue_misguess_network_histories.size(); i_index++) {
		delete this->continue_misguess_network_histories[i_index];
	}

	for (int i_index = 0; i_index < (int)this->sequence_histories.size(); i_index++) {
		delete this->sequence_histories[i_index];
	}

	if (this->halt_score_network_history != NULL) {
		delete this->halt_score_network_history;
	}

	if (this->halt_misguess_network_history != NULL) {
		delete this->halt_misguess_network_history;
	}

	for (int e_index = 0; e_index < (int)this->exit_network_histories.size(); e_index++) {
		delete this->exit_network_histories[e_index];
	}
}
