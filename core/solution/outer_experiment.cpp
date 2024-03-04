#include "outer_experiment.h"

#include "action_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

OuterExperiment::OuterExperiment() {
	this->type = EXPERIMENT_TYPE_OUTER;

	this->average_remaining_experiments_from_start = 1.0;

	this->state = OUTER_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->curr_score = 0.0;

	this->best_score = 0.0;

	this->result = EXPERIMENT_RESULT_NA;
}

OuterExperiment::~OuterExperiment() {
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
}

bool OuterExperiment::activate(AbstractNode*& curr_node,
							   Problem* problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper) {
	return false;
}

OuterExperimentHistory::OuterExperimentHistory(OuterExperiment* experiment) {
	this->experiment = experiment;
}
