#include "branch_experiment.h"

using namespace std;

BranchExperiment::BranchExperiment(vector<int> scope_context,
								   vector<int> node_context,
								   int num_steps,
								   vector<int> step_types,
								   vector<Sequence*> sequences,
								   int exit_depth,
								   int exit_node_id,
								   double seed_start_predicted_score,
								   double seed_start_scale_factor,
								   vector<double> seed_state_vals_snapshot,
								   ScopeHistory* seed_context_history,
								   ScoreNetwork* existing_misguess_network) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->num_steps = num_steps;
	this->step_types = step_types;
	this->sequences = sequences;
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			this->sequences->experiment = this;
			this->sequences->step_index = a_index;
		}
	}

	this->exit_depth = exit_depth;
	this->exit_node_id = exit_node_id;

	this->seed_start_predicted_score = seed_start_predicted_score;
	this->seed_start_scale_factor = seed_start_scale_factor;
	this->seed_state_vals_snapshot = seed_state_vals_snapshot;
	this->seed_context_history = seed_context_history;

	this->existing_misguess_network = existing_misguess_network;

	this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
}

BranchExperiment::~BranchExperiment() {
	// handle in transforms
}

void BranchExperiment::activate(vector<double>& flat_vals,
								vector<ForwardContextLayer>& context,
								RunHelper& run_helper,
								BranchExperimentHistory* history) {
	if (this->state == BRANCH_EXPERIMENT_STATE_EXPLORE) {
		explore_activate(flat_vals,
						 context,
						 run_helper,
						 history);
	} else if (this->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
		experiment_activate(flat_vals,
							context,
							run_helper,
							history);
	} else if (this->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
		clean_activate(flat_vals,
					   context,
					   run_helper,
					   history);
	} else if (this->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN) {
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

void BranchExperiment::backprop(vector<BackwardContextLayer>& context,
								double& scale_factor_error,
								RunHelper& run_helper,
								BranchExperimentHistory* history) {
	if (this->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
		experiment_backprop(context,
							run_helper,
							history);

		if (this->state_iter < 200000 && this->state_iter%10 == 0) {
			seed_activate();
		}
	} else if (this->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
		clean_backprop(context,
					   run_helper,
					   history);
	} else if (this->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN) {
		clean_backprop(context,
					   run_helper,
					   history);
	} else {
		wrapup_backprop(context,
						scale_factor_error,
						run_helper,
						history);
	}

	this->state_iter++;
	if (this->state == BRANCH_EXPERIMENT_STATE_EXPERIMENT) {
		if (this->state_iter == 1000000) {
			experiment_transform();

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
	} else if (this->state == BRANCH_EXPERIMENT_STATE_FIRST_CLEAN) {
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
	} else if (this->state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN) {
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

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->score_network_history = NULL;
	this->misguess_network_history = NULL;
}

BranchExperimentHistory::~BranchExperimentHistory() {
	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}

	if (this->misguess_network_history != NULL) {
		delete this->misguess_network_history;
	}

	for (int a_index = 0; a_index < (int)this->step_state_network_histories.size(); a_index++) {
		for (int s_index = 0; s_index < (int)this->step_state_network_histories[a_index].size(); s_index++) {
			if (this->step_state_network_histories[a_index][s_index] != NULL) {
				delete this->step_state_network_histories[a_index][s_index];
			}
		}
	}

	for (int a_index = 0; a_index < (int)this->step_score_network_histories.size(); a_index++) {
		if (this->step_score_network_histories[a_index] != NULL) {
			delete this->step_score_network_histories[a_index];
		}
	}

	for (int a_index = 0; a_index < (int)this->step_input_state_network_histories.size(); a_index++) {
		for (int s_index = 0; s_index < (int)this->step_input_state_network_histories[a_index].size(); s_index++) {
			for (int i_index = 0; i_index < (int)this->step_input_state_network_histories[a_index][s_index].size(); i_index++) {
				if (this->step_input_state_network_histories[a_index][s_index][i_index] != NULL) {
					delete this->step_input_state_network_histories[a_index][s_index][i_index];
				}
			}
		}
	}

	for (int a_index = 0; a_index < (int)this->sequence_histories.size(); a_index++) {
		if (this->sequence_histories[a_index] != NULL) {
			delete this->sequence_histories[a_index];
		}
	}

	for (int e_index = 0; e_index < (int)this->exit_network_histories.size(); e_index++) {
		if (this->exit_network_histories[e_index] != NULL) {
			delete this->exit_network_histories[e_index];
		}
	}
}
