#include "solution_wrapper.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::init(vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	// temp
	this->allow_predict = true;
	this->in_predict = false;

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(NUM_STATES);
	this->states.back().setConstant(0.0);

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->remaining_predict.push_back(vector<AbstractNode*>());

	this->solution->starting_scope->start_activate(obs,
												   this);
}

pair<bool,int> SolutionWrapper::step(vector<double> obs) {
	if (this->remaining_predict.back().size() > 0) {
		ActionNode* action_node = (ActionNode*)this->remaining_predict.back()[0];
		action_node->step_callback(obs,
								   this);
	} else {
		if (this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();
			action_node->step_callback(obs,
									   this);
		}
	}

	uniform_real_distribution<double> distribution(0.0, 1.0);

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->remaining_predict.back().size() > 0) {
			this->remaining_predict.back()[0]->step(
				obs,
				action,
				is_next,
				this);
		} else {
			if (this->allow_predict
					&& !this->in_predict) {
				if (this->solution->starting_scope_num_stabilize < NEW_SCOPE_NUM_STABILIZE) {
					if (distribution(generator) <= this->solution->predict_rate) {
						uniform_int_distribution<int> predict_select_distribution(0, 4);
						if (predict_select_distribution(generator) == 0) {
							if (this->scope_histories.back()->scope == this->solution->starting_scope) {
								if (this->node_context.back() == NULL) {
									if (distribution(generator) <= this->scope_histories.back()->scope->predict_rate) {
										uniform_int_distribution<int> existing_distribution(0, 1);
										if (existing_distribution(generator) != 0) {
											predict_helper(this);
											continue;
										}
									}
								} else {
									if (distribution(generator) <= this->node_context.back()->predict_rate) {
										uniform_int_distribution<int> existing_distribution(0, 1);
										if (existing_distribution(generator) != 0) {
											predict_helper(this);
											continue;
										}
									}
								}
							}
						}
					}
				} else {
					if (distribution(generator) <= this->solution->predict_rate) {
						uniform_int_distribution<int> predict_select_distribution(0, 4);
						if (predict_select_distribution(generator) == 0) {
							if (this->node_context.back() == NULL) {
								if (distribution(generator) <= this->scope_histories.back()->scope->predict_rate) {
									uniform_int_distribution<int> existing_distribution(0, 1);
									if (existing_distribution(generator) != 0) {
										predict_helper(this);
										continue;
									}
								}
							} else {
								if (distribution(generator) <= this->node_context.back()->predict_rate) {
									uniform_int_distribution<int> existing_distribution(0, 1);
									if (existing_distribution(generator) != 0) {
										predict_helper(this);
										continue;
									}
								}
							}
						}
					}
				}
			}

			if (this->node_context.back() == NULL) {
				if (this->scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					if (this->remaining_predict[this->remaining_predict.size() - 2].size() > 0) {
						ScopeNode* scope_node = (ScopeNode*)this->remaining_predict[this->remaining_predict.size() - 2][0];
						scope_node->exit_step(this);
					} else {
						ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
						scope_node->exit_step(this);
					}
				}
			} else {
				this->node_context.back()->step(obs,
												action,
												is_next,
												this);
			}
		}
	}

	return {is_done, action};
}

void SolutionWrapper::end() {
	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->remaining_predict.clear();

	this->states.clear();
}
