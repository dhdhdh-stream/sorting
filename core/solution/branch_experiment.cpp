#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchExperiment::BranchExperiment(vector<int> scope_context,
								   vector<int> node_context) {
	this->scope_context = scope_context;
	this->node_context = node_context;

	Scope* containing_scope = solution->scopes[this->scope_context.back()];

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->containing_scope_num_input_states = containing_scope->num_input_states;
	this->containing_scope_num_local_states = containing_scope->num_local_states;

	this->existing_average_score = 0.0;
	this->existing_average_misguess = 0.0;
	this->existing_starting_state_vals = new Eigen::MatrixXd(NUM_DATAPOINTS,
		this->containing_scope_num_input_states + this->containing_scope_num_local_states);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		for (int s_index = 0; s_index < this->containing_scope_num_input_states + this->containing_scope_num_local_states; s_index++) {
			(*this->existing_starting_state_vals)(d_index, s_index) = 0.0;
		}
	}
	this->existing_target_vals = vector<double>(NUM_DATAPOINTS);

	this->existing_selected_count = 0;
	this->existing_selected_sum_score = 0.0;

	this->best_surprise = 1.0;

	uniform_int_distribution<int> recursion_protection_distribution(0, 4);
	if (recursion_protection_distribution(generator) != 0) {
		this->recursion_protection = true;
	} else {
		this->recursion_protection = false;
	}
	this->need_recursion_protection = false;

	this->new_starting_state_vals = NULL;

	this->combined_score = 0.0;
	this->branch_count = 0;
	this->branch_possible = 0;

	this->pass_through_misguess = 0.0;

	this->pass_through_selected_count = 0;
	this->pass_through_score = 0.0;
}

BranchExperiment::~BranchExperiment() {
	if (this->existing_starting_state_vals != NULL) {
		delete this->existing_starting_state_vals;
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_sequences.size(); s_index++) {
		if (this->best_sequences[s_index] != NULL) {
			delete this->best_sequences[s_index];
		}
	}

	while (this->new_starting_scope_histories.size() > 0) {
		delete this->new_starting_scope_histories.front();
		this->new_starting_scope_histories.pop_front();
	}

	if (this->new_starting_state_vals != NULL) {
		delete this->new_starting_state_vals;
	}

	while (this->new_ending_scope_histories.size() > 0) {
		delete this->new_ending_scope_histories.front();
		this->new_ending_scope_histories.pop_front();
	}

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperimentHistory* original) {
	this->experiment = original->experiment;

	this->action_histories = vector<ActionNodeHistory*>(this->experiment->best_step_types.size(), NULL);
	this->sequence_histories = vector<SequenceHistory*>(this->experiment->best_step_types.size(), NULL);
	for (int s_index = 0; s_index < (int)this->experiment->best_step_types.size(); s_index++) {
		if (this->experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->action_histories[s_index] = new ActionNodeHistory(original->action_histories[s_index]);
		} else {
			this->sequence_histories[s_index] = new SequenceHistory(original->sequence_histories[s_index]);
		}
	}
}

BranchExperimentHistory::~BranchExperimentHistory() {
	for (int s_index = 0; s_index < (int)this->action_histories.size(); s_index++) {
		if (this->action_histories[s_index] != NULL) {
			delete this->action_histories[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->sequence_histories.size(); s_index++) {
		if (this->sequence_histories[s_index] != NULL) {
			delete this->sequence_histories[s_index];
		}
	}
}
