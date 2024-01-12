#include "outer_experiment.h"

#include "action_node.h"
#include "potential_scope_node.h"
#include "scope_node.h"

using namespace std;

OuterExperiment::OuterExperiment() {
	this->type = EXPERIMENT_TYPE_OUTER;

	this->average_remaining_experiments_from_start = 1.0;

	this->state = OUTER_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE;
	this->state_iter = 0;

	this->curr_score = 0.0;

	this->best_score = 0.0;
}

OuterExperiment::~OuterExperiment() {
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

	for (int s_index = 0; s_index < (int)this->curr_root_scope_nodes.size(); s_index++) {
		if (this->curr_root_scope_nodes[s_index] != NULL) {
			delete this->curr_root_scope_nodes[s_index];
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

	for (int s_index = 0; s_index < (int)this->best_root_scope_nodes.size(); s_index++) {
		if (this->best_root_scope_nodes[s_index] != NULL) {
			delete this->best_root_scope_nodes[s_index];
		}
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

OuterExperimentOverallHistory::OuterExperimentOverallHistory(
		OuterExperiment* experiment) {
	this->experiment = experiment;
}
