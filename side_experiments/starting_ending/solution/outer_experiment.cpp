#include "outer_experiment.h"

#include "action_node.h"
#include "scope_node.h"
#include "sequence.h"

using namespace std;

OuterExperiment::OuterExperiment() {
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

	for (int s_index = 0; s_index < (int)this->curr_sequences.size(); s_index++) {
		if (this->curr_sequences[s_index] != NULL) {
			delete this->curr_sequences[s_index];
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

	for (int s_index = 0; s_index < (int)this->best_sequences.size(); s_index++) {
		if (this->best_sequences[s_index] != NULL) {
			delete this->best_sequences[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_root_scope_nodes.size(); s_index++) {
		if (this->best_root_scope_nodes[s_index] != NULL) {
			delete this->best_root_scope_nodes[s_index];
		}
	}
}
