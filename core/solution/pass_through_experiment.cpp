#include "pass_through_experiment.h"

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(
		vector<Scope*> scope_context,
		vector<AbstractNode*> node_context,
		bool is_branch) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->curr_score = 0.0;

	this->best_score = numeric_limits<double>::lowest();

	this->branch_experiment = NULL;

	this->new_is_better = true;

	this->result = EXPERIMENT_RESULT_NA;
}

PassThroughExperiment::~PassThroughExperiment() {
	for (int s_index = 0; s_index < (int)this->curr_actions.size(); s_index++) {
		if (this->curr_actions[s_index] != NULL) {
			delete this->curr_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_existing_scopes.size(); s_index++) {
		if (this->curr_existing_scopes[s_index] != NULL) {
			delete this->curr_existing_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_potential_scopes.size(); s_index++) {
		if (this->curr_potential_scopes[s_index] != NULL) {
			delete this->curr_potential_scopes[s_index]->scope;
			delete this->curr_potential_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_existing_scopes.size(); s_index++) {
		if (this->best_existing_scopes[s_index] != NULL) {
			delete this->best_existing_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_potential_scopes.size(); s_index++) {
		if (this->best_potential_scopes[s_index] != NULL) {
			delete this->best_potential_scopes[s_index]->scope;
			delete this->best_potential_scopes[s_index];
		}
	}

	if (this->branch_experiment != NULL) {
		delete this->branch_experiment;
	}
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
		} else if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* original_scope_node_history = (ScopeNodeHistory*)original->pre_step_histories[s_index];
			this->pre_step_histories[s_index] = new ScopeNodeHistory(original_scope_node_history);
		} else {
			ScopeNodeHistory* original_scope_node_history = (ScopeNodeHistory*)original->pre_step_histories[s_index];
			this->pre_step_histories[s_index] = new ScopeNodeHistory(original_scope_node_history);
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
		} else if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* original_scope_node_history = (ScopeNodeHistory*)original->post_step_histories[h_index];
			this->post_step_histories[h_index] = new ScopeNodeHistory(original_scope_node_history);
		} else {
			ScopeNodeHistory* original_scope_node_history = (ScopeNodeHistory*)original->post_step_histories[h_index];
			this->post_step_histories[h_index] = new ScopeNodeHistory(original_scope_node_history);
		}
	}
}

PassThroughExperimentInstanceHistory::~PassThroughExperimentInstanceHistory() {
	PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;

	for (int s_index = 0; s_index < (int)this->pre_step_histories.size(); s_index++) {
		if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)this->pre_step_histories[s_index];
			delete action_node_history;
		} else if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)this->pre_step_histories[s_index];
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)this->pre_step_histories[s_index];
			delete scope_node_history;
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
		} else if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)this->post_step_histories[h_index];
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)this->post_step_histories[h_index];
			delete scope_node_history;
		}
	}
}

PassThroughExperimentOverallHistory::PassThroughExperimentOverallHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->branch_experiment_history = NULL;
}

PassThroughExperimentOverallHistory::~PassThroughExperimentOverallHistory() {
	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}
}
