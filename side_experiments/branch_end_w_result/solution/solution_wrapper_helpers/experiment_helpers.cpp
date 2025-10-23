#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);
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
					scope_node->experiment_exit_step(this);
				}
			}
		} else if (this->experiment_context.back() != NULL) {
			delete this->experiment_context.back();
			this->experiment_context.back() = NULL;
		} else {
			this->node_context.back() = NULL;
		}
	}

	this->experiment_history->experiment->backprop(
		result,
		this);

	delete this->experiment_history;
	this->experiment_history = NULL;

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	if (this->curr_branch_experiment->result == EXPERIMENT_RESULT_FAIL) {
		this->curr_branch_experiment->clean();
		delete this->curr_branch_experiment;

		this->curr_branch_experiment = NULL;
	} else if (this->curr_branch_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
		this->curr_branch_experiment->clean();

		if (this->best_branch_experiment == NULL) {
			this->best_branch_experiment = this->curr_branch_experiment;
		} else {
			if (this->curr_branch_experiment->improvement > this->best_branch_experiment->improvement) {
				delete this->best_branch_experiment;
				this->best_branch_experiment = this->curr_branch_experiment;
			} else {
				delete this->curr_branch_experiment;
			}
		}

		this->curr_branch_experiment = NULL;

		this->improvement_iter++;
		if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
			Scope* last_updated_scope = this->best_branch_experiment->scope_context;

			this->best_branch_experiment->add(this);

			clean_scope(last_updated_scope,
						this);

			this->solution->clean();

			this->solution->timestamp++;

			update_scores(this->best_branch_experiment->new_scope_histories,
						  this->best_branch_experiment->new_target_val_histories,
						  this);

			delete this->best_branch_experiment;
			this->best_branch_experiment = NULL;

			this->improvement_iter = 0;
		}
	}
}
