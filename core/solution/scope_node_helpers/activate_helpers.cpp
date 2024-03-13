#include "scope_node.h"

#include "branch_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem* problem,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	history->scope_history = scope_history;
	context.back().scope_history = scope_history;

	int inner_exit_depth = 0;

	this->scope->activate(problem,
						  context,
						  inner_exit_depth,
						  run_helper,
						  scope_history);

	// temp
	if (scope_history->pass_through_experiment_history != NULL
			&& scope_history->pass_through_experiment_history->scope_history == NULL) {
		throw invalid_argument("weird");
	}

	history->throw_id = run_helper.throw_id;

	context.pop_back();

	context.back().node = NULL;

	if (run_helper.exceeded_limit) {
		// do nothing
	} else if (run_helper.throw_id != -1) {
		map<int, AbstractNode*>::iterator it = this->catches.find(run_helper.throw_id);
		if (it != this->catches.end()) {
			run_helper.throw_id = -1;

			curr_node = it->second;
		}
		// else do nothing

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	} else if (inner_exit_depth == 0
			|| inner_exit_depth == 1) {
		curr_node = this->next_node;

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	} else {
		exit_depth = inner_exit_depth-1;
	}
}
