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

	uniform_int_distribution<int> diversity_distribution(0, DIVERSITY_RANGE-1);
	this->diversity_index = diversity_distribution(generator);

	this->explore_scope_history = NULL;
	this->explore_index = -1;
	this->scope_count = 0;

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(NUM_STATES);
	this->states.back().setConstant(0.0);

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);

	this->solution->starting_scope->experiment_start_activate(
		obs,
		this);
}

pair<bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	if (this->experiment_context.back() != NULL) {
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

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->explore_index == -1) {
				bool match_scope = false;
				if (this->solution->cycle_index == -1) {
					if (this->scope_histories.back()->scope == this->solution->starting_scope) {
						match_scope = true;
					}
				} else {
					if (this->scope_histories.back()->scope->id == this->solution->scope_index) {
						match_scope = true;
					}
				}
				if (match_scope) {
					uniform_int_distribution<int> select_distribution(0, this->scope_count);
					this->scope_count++;
					if (select_distribution(generator) == 0) {
						this->explore_scope_history = this->scope_histories.back();
					}
				}
			}

			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->experiment_context[this->experiment_context.size() - 2] != NULL) {
					AbstractExperiment* experiment = this->experiment_context[this->experiment_context.size() - 2]->experiment;
					experiment->experiment_exit_step(obs,
													 this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->experiment_exit_step(obs,
													 this);
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
	if (this->experiments[this->diversity_index] == NULL
			|| this->experiments[this->diversity_index]->is_gather_existing()) {
		update_helper(this,
					  result);

		this->existing_since_update++;
	} else {
		this->new_since_update++;
	}

	if (this->experiments[this->diversity_index] == NULL) {
		create_experiment(this->scope_histories[0],
						  this->diversity_index,
						  this);
	}

	if (this->experiments[this->diversity_index] == NULL
			|| this->experiments[this->diversity_index]->is_gather_existing()) {
		this->existing_scope_histories.push_back(this->scope_histories[0]);
		this->existing_target_val_histories.push_back(result);
	} else {
		if (!this->experiments[this->diversity_index]->is_measure()
				&& this->explore_scope_history != NULL) {
			ScopeHistory* scope_history = this->explore_scope_history->train_copy();
			this->explore_scope_histories.push_back(scope_history);
			this->explore_index_histories.push_back(this->explore_index);
			this->explore_target_val_histories.push_back(result);
		}

		delete this->scope_histories[0];
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	this->states.clear();

	if (this->existing_scope_histories.size() >= BATCH_SIZE) {
		train_existing_helper(this);
	}
	if (this->explore_scope_histories.size() >= BATCH_SIZE) {
		train_explore_helper(this);
	}

	if (this->experiments[this->diversity_index] == NULL
			|| this->experiments[this->diversity_index]->is_gather_existing()) {
		for (int d_index = 0; d_index < DIVERSITY_RANGE; d_index++) {
			bool is_add = false;
			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[d_index].begin();
					it != this->experiment_histories[d_index].end(); it++) {
				it->first->backprop(result,
									it->second,
									this,
									is_add);
			}
			if (is_add) {
				break;
			}
		}
	} else {
		bool is_add = false;
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[this->diversity_index].begin();
				it != this->experiment_histories[this->diversity_index].end(); it++) {
			it->first->backprop(result,
								it->second,
								this,
								is_add);
		}
	}

	for (int d_index = 0; d_index < DIVERSITY_RANGE; d_index++) {
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[d_index].begin();
				it != this->experiment_histories[d_index].end(); it++) {
			delete it->second;
		}
		this->experiment_histories[d_index].clear();
	}
}
