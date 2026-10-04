#include "solution_wrapper.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::experiment_init(vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	uniform_int_distribution<int> update_distribution(0, 4);
	if (update_distribution(generator) == 0) {
		this->run_type = RUN_TYPE_UPDATE;
	} else {
		this->run_type = RUN_TYPE_PREDICT;
	}
	this->has_damage = false;

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(NUM_STATES);
	this->states.back().setConstant(0.0);

	this->num_actions = 0;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->outer_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->outer_scope->nodes[0]);
	this->experiment_context.push_back(NULL);
	this->remaining_predict.push_back(vector<AbstractNode*>());

	this->solution->outer_scope->experiment_start_activate(
		obs,
		this);
}

pair<bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	if (this->remaining_predict.back().size() > 0) {
		ActionNode* action_node = (ActionNode*)this->remaining_predict.back()[0];
		action_node->experiment_step_callback(obs,
											  this);
	} else if (this->experiment_context.back() != NULL) {
		AbstractExperiment* experiment = this->experiment_context.back()->experiment;
		experiment->experiment_step_callback(obs,
											 this);
	} else {
		if (this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();
			action_node->experiment_step_callback(obs,
												  this);

			uniform_int_distribution<int> predict_distribution(0, 9);
			if (predict_distribution(generator) == 0) {
				predict_helper(this);
			}
		}
	}

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->remaining_predict.back().size() > 0) {
			this->remaining_predict.back()[0]->experiment_step(
				obs,
				action,
				is_next,
				this);
		} else if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->remaining_predict[this->remaining_predict.size() - 2].size() > 0) {
					ScopeNode* scope_node = (ScopeNode*)this->remaining_predict[this->remaining_predict.size() - 2][0];
					scope_node->experiment_exit_step(obs,
													 this);
				} else if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(obs,
													 this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->experiment_exit_step(obs,
													 this);

					uniform_int_distribution<int> predict_distribution(0, 9);
					if (predict_distribution(generator) == 0) {
						predict_helper(this);
					}
				}
			}
		} else if (this->experiment_context.back() != NULL) {
			AbstractExperiment* experiment = this->experiment_context.back()->experiment;
			experiment->experiment_step(obs,
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
	if (this->run_type == RUN_TYPE_UPDATE) {
		if (this->experiment_histories.size() == 0) {
			create_experiment(this->scope_histories[0],
							  this);
		}

		update_helper(this,
					  result);

		this->existing_scope_histories.push_back(this->scope_histories[0]);
		this->existing_target_val_histories.push_back(result);

		this->update_since_update++;
	} else {
		train_predict_helper(this,
							 result);

		delete this->scope_histories[0];

		if (this->has_damage) {
			this->damage_since_update++;
		} else {
			this->safe_since_update++;
		}
	}

	if (this->experiment_histories.size() >= 2) {
		AbstractExperiment* keep_experiment = NULL;
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories.begin();
				it != this->experiment_histories.end(); it++) {
			if (keep_experiment == NULL) {
				keep_experiment = it->first;
			} else {
				if (it->first->further_than(keep_experiment)) {
					delete keep_experiment;

					keep_experiment = it->first;
				} else {
					delete it->first;
				}
			}
		}
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
	this->remaining_predict.clear();

	this->states.clear();

	if (this->existing_scope_histories.size() >= BATCH_SIZE) {
		train_existing_helper(this);
	}

	if (this->experiment_histories.size() == 1) {
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories.begin();
				it != this->experiment_histories.end(); it++) {
			it->first->backprop(result,
								it->second,
								this);
		}
	}

	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}
	this->experiment_histories.clear();
}
