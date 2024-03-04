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

	history->throw_id = run_helper.throw_id;

	context.pop_back();

	context.back().node = NULL;

	if (run_helper.exceeded_limit) {
		return;
	} else if (run_helper.throw_id != -1) {
		map<int, AbstractNode*>::iterator it = this->catches.find(run_helper.throw_id);
		if (it != this->catches.end()) {
			curr_node = it->second;

			for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
				bool is_selected = this->experiments[e_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper);
				if (is_selected) {
					return;
				}
			}

			run_helper.throw_id = -1;
		} else {
			curr_node = NULL;

			for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
				bool is_selected = this->experiments[e_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper);
				if (is_selected) {
					run_helper.throw_id = -1;
					return;
				}
			}
		}
	} else if (inner_exit_depth == -1) {
		curr_node = this->next_node;

		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
