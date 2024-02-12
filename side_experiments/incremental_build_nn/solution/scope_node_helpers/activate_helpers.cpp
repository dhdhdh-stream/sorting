#include "scope_node.h"

#include "branch_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem* problem,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	context.back().scope_history = scope_history;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->scope->activate(problem,
						  context,
						  inner_exit_depth,
						  inner_exit_node,
						  run_helper,
						  scope_history);

	context.pop_back();

	context.back().node = NULL;

	if (!run_helper.exceeded_limit
			&& inner_exit_depth == -1) {
		curr_node = this->next_node;

		if (this->experiment != NULL) {
			if (this->experiment->type == EXPERIMENT_TYPE_BRANCH) {
				BranchExperiment* branch_experiment = (BranchExperiment*)this->experiment;
				branch_experiment->activate(curr_node,
											problem,
											context,
											exit_depth,
											exit_node,
											run_helper,
											history->experiment_history);
			} else {
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;
				pass_through_experiment->activate(curr_node,
												  problem,
												  context,
												  exit_depth,
												  exit_node,
												  run_helper,
												  history->experiment_history);
			}
		}
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
