#include "repetition_experiment.h"

#include "action_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

RepetitionExperiment::RepetitionExperiment(
		AbstractNode* node_context,
		Scope* new_scope) {
	this->type = EXPERIMENT_TYPE_REPETITION;

	this->scope_context = node_context->parent;
	this->node_context = node_context;
	this->is_branch = false;
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			this->exit_next_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			this->exit_next_node = scope_node->next_node;
		}
		break;
	}

	this->node_context->experiment = this;

	this->new_scope = new_scope;

	this->total_sum_scores = 0.0;
	this->total_count = 0;

	this->state = REPETITION_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

RepetitionExperiment::~RepetitionExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

RepetitionExperimentHistory::RepetitionExperimentHistory(
		RepetitionExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

RepetitionExperimentState::RepetitionExperimentState(
		RepetitionExperiment* experiment) {
	this->experiment = experiment;
}
