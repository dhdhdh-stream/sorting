#include "branch_experiment.h"

#include "action_node.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchExperiment::BranchExperiment(vector<int>& scope_context,
								   vector<int>& node_context) {
	this->scope_context = scope_context;
	this->node_context = node_context;

	this->average_remaining_experiments_from_start = 0.0;
	this->average_instances_per_run = 1.0;

	this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
	this->state_iter = 0;

	this->best_surprise = 1.0;

	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	this->average_score = parent_scope->average_score;
	this->average_misguess = parent_scope->average_misguess;
	this->misguess_variance = parent_scope->misguess_variance;

	this->branch_count = 0;
	this->combined_score = 0.0;

	this->obs_experiment = NULL;
}

BranchExperiment::~BranchExperiment() {
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

	for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
			it != this->score_state_scales.end(); it++) {
		delete it->second;
	}

	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		delete this->new_score_states[s_index];
	}

	if (this->obs_experiment != NULL) {
		delete this->obs_experiment;
	}
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;
}

BranchExperimentHistory::~BranchExperimentHistory() {
	for (int s_index = 0; s_index < (int)this->sequence_histories.size(); s_index++) {
		if (this->sequence_histories[s_index] != NULL) {
			delete this->sequence_histories[s_index];
		}
	}
}
