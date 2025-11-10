#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

const int RUN_TIMESTEPS = 100;

void SolutionWrapper::result_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_experiment != NULL) {
		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)this->curr_experiment;
				this->experiment_history = new BranchExperimentHistory(branch_experiment);
			}
			break;
		case EXPERIMENT_TYPE_PASS_THROUGH:
			{
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->curr_experiment;
				this->experiment_history = new PassThroughExperimentHistory(pass_through_experiment);
			}
			break;
		}
	}

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);

	scope_history->pre_obs = this->problem->get_observations();
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
	bool is_continue;

	this->scope_histories[0]->post_obs = this->problem->get_observations();

	add_existing_samples_helper(this->scope_histories[0]);

	if (this->curr_experiment == NULL) {
		create_experiment(this->scope_histories[0],
						  this);

		delete this->scope_histories[0];

		is_continue = false;
	} else {
		this->existing_result = result;

		this->experiment_history->experiment->result_backprop(
			result,
			this);

		if (this->experiment_history->is_hit) {
			is_continue = true;
		} else {
			delete this->experiment_history;
			this->experiment_history = NULL;

			is_continue = false;
		}
	}

	this->scope_histories.clear();
	this->node_context.clear();

	return is_continue;
}
