#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "new_scope_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::result_init() {
	if (is_new_scope_iter(this)) {
		if (this->curr_new_scope_experiment != NULL) {
			if (this->curr_new_scope_experiment->curr_experiment != NULL) {
				this->experiment_history = new NewScopeExperimentHistory(
					this->curr_new_scope_experiment->curr_experiment);
			}
		}
	} else {
		if (this->curr_branch_experiment != NULL) {
			this->experiment_history = new BranchExperimentHistory(this->curr_branch_experiment);
		}
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
	this->experiment_context.push_back(NULL);
}

pair<bool,int> SolutionWrapper::result_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->result_exit_step(this);
				}
			}
		} else if (this->experiment_context.back() != NULL) {
			AbstractExperiment* experiment = this->experiment_context.back()->experiment;
			bool fetch_action;	// unused
			experiment->experiment_step(obs,
										action,
										is_next,
										fetch_action,
										this);
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
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				break;
			} else {
				if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->result_exit_step(this);
				}
			}
		} else if (this->experiment_context.back() != NULL) {
			delete this->experiment_context.back();
			this->experiment_context.back() = NULL;
		} else {
			this->node_context.back() = NULL;
		}
	}

	if (is_new_scope_iter(this)) {
		if (this->curr_new_scope_experiment == NULL) {
			create_new_scope_overall_experiment(this->scope_histories[0],
												this);

			delete this->scope_histories[0];

			this->scope_histories.clear();
			this->node_context.clear();
			this->experiment_context.clear();

			return false;
		} else if (this->curr_new_scope_experiment->curr_experiment == NULL) {
			create_new_scope_experiment(this->scope_histories[0],
										this);

			delete this->scope_histories[0];

			this->scope_histories.clear();
			this->node_context.clear();
			this->experiment_context.clear();

			return false;
		} else {
			if (this->experiment_history->is_hit) {
				this->existing_result = result;

				delete this->scope_histories[0];

				this->scope_histories.clear();
				this->node_context.clear();
				this->experiment_context.clear();

				return true;
			} else {
				this->experiment_history->experiment->backprop(
					result,
					this);

				delete this->experiment_history;
				this->experiment_history = NULL;

				this->scope_histories.clear();
				this->node_context.clear();
				this->experiment_context.clear();

				return false;
			}
		}
	} else {
		if (this->curr_branch_experiment == NULL) {
			create_branch_experiment(this->scope_histories[0],
									 this);

			delete this->scope_histories[0];

			this->scope_histories.clear();
			this->node_context.clear();
			this->experiment_context.clear();

			return false;
		} else {
			if (this->experiment_history->is_hit) {
				this->existing_result = result;

				delete this->scope_histories[0];

				this->scope_histories.clear();
				this->node_context.clear();
				this->experiment_context.clear();

				return true;
			} else {
				this->experiment_history->experiment->backprop(
					result,
					this);

				delete this->experiment_history;
				this->experiment_history = NULL;

				this->scope_histories.clear();
				this->node_context.clear();
				this->experiment_context.clear();

				return false;
			}
		}
	}
}
