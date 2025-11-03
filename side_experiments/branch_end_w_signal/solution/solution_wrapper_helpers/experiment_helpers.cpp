#include "solution_wrapper.h"

#include <iostream>

#include "branch_end_node.h"
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

void SolutionWrapper::experiment_init() {
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
	this->experiment_context.push_back(NULL);

	scope_history->pre_obs = this->problem->get_observations();
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;
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
					scope_node->experiment_exit_step(this);
				}
			}
		} else if (this->experiment_context.back() != NULL) {
			AbstractExperiment* experiment = this->experiment_context.back()->experiment;
			experiment->experiment_step(obs,
										action,
										is_next,
										fetch_action,
										this);
		} else {
			this->node_context.back()->experiment_step(obs,
													   action,
													   is_next,
													   this);
		}
	}

	return tuple<bool,bool,int>{is_done, fetch_action, action};
}

void SolutionWrapper::set_action(int action) {
	AbstractExperiment* experiment = this->experiment_context.back()->experiment;
	experiment->set_action(action,
						   this);
}

void SolutionWrapper::experiment_end(double result) {
	this->scope_histories[0]->post_obs = this->problem->get_observations();

	if (this->curr_experiment == NULL) {
		create_experiment(this->scope_histories[0],
						  this);
	} else {
		this->experiment_history->experiment->backprop(
			result,
			this);

		delete this->experiment_history;
		this->experiment_history = NULL;

		if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
			this->curr_experiment->clean();
			delete this->curr_experiment;

			this->curr_experiment = NULL;
		} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			this->curr_experiment->clean();

			if (this->best_experiment == NULL) {
				this->best_experiment = this->curr_experiment;
			} else {
				double curr_impact = this->curr_experiment->improvement;
				double best_impact = this->best_experiment->improvement;
				if (curr_impact > best_impact) {
					delete this->best_experiment;
					this->best_experiment = this->curr_experiment;
				} else {
					delete this->curr_experiment;
				}
			}

			this->curr_experiment = NULL;

			this->improvement_iter++;
			bool is_done = false;
			if (this->solution->timestamp % 3 == 0) {
				if (this->improvement_iter >= BRANCH_IMPROVEMENTS_PER_ITER) {
					is_done = true;
				}
			} else {
				if (this->improvement_iter >= PASS_THROUGH_IMPROVEMENTS_PER_ITER) {
					is_done = true;
				}
			}
			if (is_done) {
				Scope* last_updated_scope = this->best_experiment->scope_context;

				this->best_experiment->add(this);

				this->solution->curr_score = this->best_experiment->calc_new_score();

				delete this->best_experiment;
				this->best_experiment = NULL;

				clean_scope(last_updated_scope,
							this);

				this->solution->clean_scopes();

				for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
					this->solution->scopes[s_index]->update_signals();
				}

				this->solution->timestamp++;
				if (this->solution->timestamp >= RUN_TIMESTEPS) {
					this->solution->timestamp = -1;
				}

				this->improvement_iter = 0;
			}
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
