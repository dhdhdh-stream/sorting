#include "pass_through_experiment.h"

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "solution.h"
#include "state.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(
		vector<int> scope_context,
		vector<int> node_context) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->average_remaining_experiments_from_start = 1.0;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);
	this->i_scope_histories.reserve(solution->curr_num_datapoints);
	this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE;
	this->state_iter = 0;

	this->curr_score = 0.0;

	this->best_score = 0.0;

	this->branch_experiment = NULL;
}

PassThroughExperiment::~PassThroughExperiment() {
	for (int s_index = 0; s_index < (int)this->curr_actions.size(); s_index++) {
		if (this->curr_actions[s_index] != NULL) {
			delete this->curr_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_potential_scopes.size(); s_index++) {
		if (this->curr_potential_scopes[s_index] != NULL) {
			delete this->curr_potential_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_potential_scopes.size(); s_index++) {
		if (this->best_potential_scopes[s_index] != NULL) {
			delete this->best_potential_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}

	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

PassThroughExperimentInstanceHistory::PassThroughExperimentInstanceHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->branch_experiment_history = NULL;
}

PassThroughExperimentInstanceHistory::PassThroughExperimentInstanceHistory(
		PassThroughExperimentInstanceHistory* original) {
	this->experiment = original->experiment;

	PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)original->experiment;
	this->pre_step_histories = vector<void*>(original->pre_step_histories.size());
	for (int s_index = 0; s_index < (int)original->pre_step_histories.size(); s_index++) {
		if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* original_action_node_history = (ActionNodeHistory*)original->pre_step_histories[s_index];
			this->pre_step_histories[s_index] = new ActionNodeHistory(original_action_node_history);
		} else {
			PotentialScopeNodeHistory* original_potential_scope_node_history = (PotentialScopeNodeHistory*)original->pre_step_histories[s_index];
			this->pre_step_histories[s_index] = new PotentialScopeNodeHistory(original_potential_scope_node_history);
		}
	}

	if (original->branch_experiment_history == NULL) {
		this->branch_experiment_history = NULL;
	} else {
		BranchExperimentInstanceHistory* original_branch_experiment_history = (BranchExperimentInstanceHistory*)original->branch_experiment_history;
		this->branch_experiment_history = new BranchExperimentInstanceHistory(original_branch_experiment_history);
	}

	this->post_step_histories = vector<void*>(original->post_step_histories.size());
	for (int h_index = 0; h_index < (int)original->post_step_histories.size(); h_index++) {
		int s_index = (int)pass_through_experiment->best_step_types.size() - (int)original->post_step_histories.size() + h_index;
		if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* original_action_node_history = (ActionNodeHistory*)original->post_step_histories[h_index];
			this->post_step_histories[h_index] = new ActionNodeHistory(original_action_node_history);
		} else {
			PotentialScopeNodeHistory* original_potential_scope_node_history = (PotentialScopeNodeHistory*)original->post_step_histories[h_index];
			this->post_step_histories[h_index] = new PotentialScopeNodeHistory(original_potential_scope_node_history);
		}
	}
}

PassThroughExperimentInstanceHistory::~PassThroughExperimentInstanceHistory() {
	PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;

	for (int s_index = 0; s_index < (int)this->pre_step_histories.size(); s_index++) {
		if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)this->pre_step_histories[s_index];
			delete action_node_history;
		} else {
			PotentialScopeNodeHistory* potential_scope_node_history = (PotentialScopeNodeHistory*)this->pre_step_histories[s_index];
			delete potential_scope_node_history;
		}
	}

	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}

	for (int h_index = 0; h_index < (int)this->post_step_histories.size(); h_index++) {
		int s_index = (int)pass_through_experiment->best_step_types.size() - (int)this->post_step_histories.size() + h_index;
		if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)this->post_step_histories[h_index];
			delete action_node_history;
		} else {
			PotentialScopeNodeHistory* potential_scope_node_history = (PotentialScopeNodeHistory*)this->post_step_histories[h_index];
			delete potential_scope_node_history;
		}
	}
}

PassThroughExperimentOverallHistory::PassThroughExperimentOverallHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}
