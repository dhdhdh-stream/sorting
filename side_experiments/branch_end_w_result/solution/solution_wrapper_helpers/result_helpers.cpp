#include "solution_wrapper.h"

#include "branch_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::result_init() {
	if (this->curr_branch_experiment != NULL) {
		this->experiment_history = new BranchExperimentHistory(this->curr_branch_experiment);
	}

	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
}

pair<bool,int> SolutionWrapper::result_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
				scope_node->result_exit_step(this);
			}
		} else {
			this->node_context.back()->result_step(obs,
												   action,
												   is_next,
												   this);
		}
	}

	return {is_done, action};
}

bool SolutionWrapper::result_end(double result) {
	while (true) {
		if (this->node_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				break;
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
				scope_node->exit_step(this);
			}
		} else {
			this->node_context.back() = NULL;
		}
	}

	if (this->curr_branch_experiment == NULL) {
		create_branch_experiment(this->scope_histories[0],
								 this);

		delete this->scope_histories[0];

		this->scope_histories.clear();
		this->node_context.clear();

		return false;
	} else {
		delete this->scope_histories[0];

		this->scope_histories.clear();
		this->node_context.clear();

		if (this->experiment_history->is_hit) {
			this->existing_result = result;

			return true;
		} else {
			delete this->experiment_history;
			this->experiment_history = NULL;

			return false;
		}
	}
}
