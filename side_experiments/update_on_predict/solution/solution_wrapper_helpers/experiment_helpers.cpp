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

	#if defined(MDEBUG) && MDEBUG
	uniform_int_distribution<int> is_explore_distribution(0, 1);
	#else
	uniform_int_distribution<int> is_explore_distribution(0, 4);
	#endif /* MDEBUG */
	if (is_explore_distribution(generator) == 0) {
		this->is_explore = true;
	} else {
		this->is_explore = false;
	}

	uniform_int_distribution<int> experiment_distribution(0, DIVERSITY-1);
	this->experiment_index = experiment_distribution(generator);

	uniform_int_distribution<int> allow_predict_distribution(0, 9);
	if (allow_predict_distribution(generator) != 0) {
		this->allow_predict = true;
	} else {
		this->allow_predict = false;
	}
	this->in_predict = false;

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(NUM_STATES);
	this->states.back().setConstant(0.0);

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);
	this->remaining_predict.push_back(vector<AbstractNode*>());

	this->solution->starting_scope->experiment_start_activate(
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
		}
	}

	uniform_real_distribution<double> distribution(0.0, 1.0);

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->experiment_context.back() != NULL) {
			AbstractExperiment* experiment = this->experiment_context.back()->experiment;
			experiment->experiment_step(obs,
										action,
										is_next,
										this);
		} else if (this->remaining_predict.back().size() > 0) {
			this->remaining_predict.back()[0]->experiment_step(
				obs,
				action,
				is_next,
				this);
		} else {
			if (this->allow_predict
					&& !this->in_predict) {
				if (this->scope_histories.back()->scope->predict_num_stabilize >= NEW_SCOPE_NUM_STABILIZE) {
					if (distribution(generator) <= this->solution->predict_rate) {
						uniform_int_distribution<int> predict_select_distribution(0, 4);
						if (predict_select_distribution(generator) == 0) {
							if (this->node_context.back() == NULL) {
								if (distribution(generator) <= this->scope_histories.back()->scope->predict_rate) {
									this->run_predict_scope_histories.push_back(this->scope_histories.back());
									this->run_predict_indexes.push_back(this->scope_histories.back()->node_histories.size()-1);

									uniform_int_distribution<int> existing_distribution(0, 1);
									if (existing_distribution(generator) != 0) {
										this->run_predict_is_existing.push_back(false);

										predict_helper(this);
										continue;
									} else {
										this->run_predict_is_existing.push_back(true);
									}
								}
							} else {
								if (distribution(generator) <= this->node_context.back()->predict_rate) {
									this->run_predict_scope_histories.push_back(this->scope_histories.back());
									this->run_predict_indexes.push_back(this->scope_histories.back()->node_histories.size()-1);

									uniform_int_distribution<int> existing_distribution(0, 1);
									if (existing_distribution(generator) != 0) {
										this->run_predict_is_existing.push_back(false);

										predict_helper(this);
										continue;
									} else {
										this->run_predict_is_existing.push_back(true);
									}
								}
							}
						}
					}
				}
			}

			if (this->node_context.back() == NULL
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
					}
				}
			} else {
				this->node_context.back()->experiment_step(obs,
														   action,
														   is_next,
														   this);
			}
		}
	}

	return {is_done, action};
}

void SolutionWrapper::experiment_end(double result) {
	train_predict_helper(this,
						 result);

	if (!this->is_explore) {
		if (this->allow_predict) {
			this->solution->predict_score = 0.999*this->solution->predict_score + 0.001*result;
		} else {
			this->solution->non_predict_score = 0.999*this->solution->non_predict_score + 0.001*result;

			this->solution->run_average_num_actions = 0.999*this->solution->run_average_num_actions + 0.001*this->num_actions;
			this->solution->predict_rate = 1.0 / pow(this->solution->run_average_num_actions, 0.3);
		}

		/**
		 * - training on explore significantly hurts results
		 *   - even if, e.g., training only post explore
		 */
		update_helper(this,
					  result);
	}

	for (int s_index = 0; s_index < (int)this->ramp_tracker.size(); s_index++) {
		this->ramp_tracker[s_index].clear();
	}

	if (this->experiment_histories[this->experiment_index].size() == 0) {
		if (!this->allow_predict) {
			create_experiment(this->scope_histories[0],
							  this);
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
	this->remaining_predict.clear();

	this->states.clear();

	if (this->is_explore) {
		if (this->experiment_histories[this->experiment_index].size() >= 2) {
			AbstractExperiment* keep_experiment = NULL;
			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[this->experiment_index].begin();
					it != this->experiment_histories[this->experiment_index].end(); it++) {
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
		} else {
			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[this->experiment_index].begin();
					it != this->experiment_histories[this->experiment_index].end(); it++) {
				it->first->backprop(result,
									it->second,
									this);
			}
		}
	} else {
		for (int e_index = 0; e_index < DIVERSITY; e_index++) {
			if (this->experiment_histories[e_index].size() >= 2) {
				AbstractExperiment* keep_experiment = NULL;
				for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[e_index].begin();
						it != this->experiment_histories[e_index].end(); it++) {
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
			} else {
				for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[e_index].begin();
						it != this->experiment_histories[e_index].end(); it++) {
					it->first->backprop(result,
										it->second,
										this);
				}
			}
		}
	}

	for (int e_index = 0; e_index < DIVERSITY; e_index++) {
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[e_index].begin();
				it != this->experiment_histories[e_index].end(); it++) {
			delete it->second;
		}
		this->experiment_histories[e_index].clear();
	}

	this->iters_since_update++;
}
