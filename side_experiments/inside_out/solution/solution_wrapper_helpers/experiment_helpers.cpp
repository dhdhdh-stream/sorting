#include "solution_wrapper.h"

#include <iostream>

#include "abstract_experiment.h"
#include "confusion.h"
#include "constants.h"
#include "new_scope_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;
	this->num_confusion_instances = 0;

	this->run_index++;
	#if defined(MDEBUG) && MDEBUG
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);
	this->confusion_context.push_back(NULL);

	if (this->solution->scopes[0]->new_scope_experiment != NULL) {
		this->solution->scopes[0]->new_scope_experiment->pre_activate(this);
	}
}

pair<bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL
				&& this->confusion_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(this);
				} else if (this->confusion_context[this->confusion_context.size() - 2] != NULL) {
					Confusion* confusion = this->confusion_context[this->confusion_context.size() - 2]->confusion;
					confusion->experiment_exit_step(this);
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
										this);
		} else if (this->confusion_context.back() != NULL) {
			Confusion* confusion = this->confusion_context.back()->confusion;
			confusion->experiment_step(obs,
									   action,
									   is_next,
									   this);
		} else {
			this->node_context.back()->experiment_step(obs,
													   action,
													   is_next,
													   this);
		}
	}

	return {is_done, action};
}

void SolutionWrapper::experiment_end(double result) {
	if (this->solution->scopes[0]->new_scope_experiment != NULL) {
		this->solution->scopes[0]->new_scope_experiment->back_activate(this);
	}

	if (this->curr_experiment == NULL) {
		create_experiment(this->scope_histories[0],
						  this->curr_experiment,
						  this);
	}
	sum_num_actions += this->num_actions;
	sum_num_confusion_instances += this->num_confusion_instances;
	if (this->run_index % CHECK_CONFUSION_ITER == 0) {
		double num_actions = (double)sum_num_actions / (double)CHECK_CONFUSION_ITER;
		double num_confusions = (double)sum_num_confusion_instances / (double)CHECK_CONFUSION_ITER;

		if (num_actions / (double)ACTIONS_PER_CONFUSION > num_confusions) {
			create_confusion(this->scope_histories[0],
							 this);
		}

		sum_num_actions = 0;
		sum_num_confusion_instances = 0;
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
	this->confusion_context.clear();

	if (this->experiment_history != NULL) {
		this->experiment_history->experiment->backprop(
			result,
			this);
		if (this->experiment_history->experiment->result == EXPERIMENT_RESULT_FAIL) {
			this->experiment_history->experiment->clean();
			delete this->experiment_history->experiment;

			this->curr_experiment = NULL;
		} else if (this->experiment_history->experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			this->experiment_history->experiment->clean();

			if (this->best_experiment == NULL) {
				this->best_experiment = this->experiment_history->experiment;
			} else {
				if (this->experiment_history->experiment->improvement > best_experiment->improvement) {
					delete this->best_experiment;
					this->best_experiment = this->experiment_history->experiment;
				} else {
					delete this->experiment_history->experiment;
				}
			}

			this->curr_experiment = NULL;

			improvement_iter++;
			if (improvement_iter >= IMPROVEMENTS_PER_ITER) {
				Scope* last_updated_scope = this->best_experiment->scope_context;

				this->best_experiment->add(this);
				delete this->best_experiment;
				this->best_experiment = NULL;

				clean_scope(last_updated_scope,
							this);

				if (last_updated_scope->nodes.size() >= SCOPE_EXCEEDED_NUM_NODES) {
					last_updated_scope->exceeded = true;

					check_generalize(last_updated_scope,
									 this);
				}
				if (last_updated_scope->nodes.size() <= SCOPE_RESUME_NUM_NODES) {
					last_updated_scope->exceeded = false;
				}

				this->solution->clean();

				this->solution->timestamp++;

				this->improvement_iter = 0;
			}
		}

		delete this->experiment_history;
		this->experiment_history = NULL;
	}
}
