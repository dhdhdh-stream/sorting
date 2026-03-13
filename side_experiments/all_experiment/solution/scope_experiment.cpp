#include "scope_experiment.h"

#include "scope.h"

using namespace std;

ScopeExperiment::ScopeExperiment(ObsNode* node_context,
								 Scope* new_scope,
								 AbstractNode* exit_next_node) {
	this->type = EXPERIMENT_TYPE_SCOPE;

	this->node_context = node_context;
	this->new_scope = new_scope;
	this->exit_next_node = exit_next_node;

	this->sum_scores = 0.0;
	this->count = 0;

	this->total_sum_scores = 0.0;
	this->total_count = 0;
}

ScopeExperiment::~ScopeExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

ScopeExperimentHistory::ScopeExperimentHistory(ScopeExperiment* experiment) {
	this->is_hit = false;
}

ScopeExperimentState::ScopeExperimentState(ScopeExperiment* experiment) {
	this->experiment = experiment;
}
