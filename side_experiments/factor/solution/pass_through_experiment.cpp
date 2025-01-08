#include "pass_through_experiment.h"

#include <limits>

using namespace std;

PassThroughExperiment::PassThroughExperiment(Scope* scope_context,
											 AbstractNode* node_context,
											 bool is_branch) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;

	this->curr_sum_score = 0.0;
	this->best_score = numeric_limits<double>::lowest();

	this->state_iter = -1;
	this->explore_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void PassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

PassThroughExperimentHistory::PassThroughExperimentHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;
}
