#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchExperiment::BranchExperiment(vector<int> scope_context,
								   vector<int> node_context) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->parent_pass_through_experiment = NULL;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);
	this->i_scope_histories.reserve(solution->curr_num_datapoints);
	this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->best_surprise = 0.0;

	this->combined_score = 0.0;
	this->branch_count = 0;
	this->branch_possible = 0;
}

BranchExperiment::~BranchExperiment() {
	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_possible_scopes.size(); s_index++) {
		if (this->best_possible_scopes[s_index] != NULL) {
			delete this->best_possible_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}
}

BranchExperimentInstanceHistory::BranchExperimentInstanceHistory(BranchExperiment* experiment) {
	this->experiment = experiment;
}

BranchExperimentInstanceHistory::BranchExperimentInstanceHistory(BranchExperimentInstanceHistory* original) {
	this->experiment = original->experiment;

	BranchExperiment* branch_experiment = (BranchExperiment*)original->experiment;
	this->step_histories = vector<void*>(branch_experiment->best_step_types.size());
	for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
		if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* original_action_node_history = (ActionNodeHistory*)original->step_histories[s_index];
			this->step_histories[s_index] = new ActionNodeHistory(original_action_node_history);
		} else {
			PossibleScopeNodeHistory* original_possible_scope_node_history = (PossibleScopeNodeHistory*)original->step_histories[s_index];
			this->step_histories[s_index] = new PossibleScopeNodeHistory(original_possible_scope_node_history);
		}
	}
}

BranchExperimentInstanceHistory::~BranchExperimentInstanceHistory() {
	BranchExperiment* branch_experiment = (BranchExperiment*)this->experiment;
	for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
		if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)this->step_histories[s_index];
			delete action_node_history;
		} else {
			PossibleScopeNodeHistory* possible_scope_node_history = (PossibleScopeNodeHistory*)this->step_histories[s_index];
			delete possible_scope_node_history;
		}
	}
}

BranchExperimentOverallHistory::BranchExperimentOverallHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
