#include "solution_wrapper.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "noop_node.h"
#include "obs_network.h"
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

	this->has_explore = false;

	this->state.setConstant(0.0);

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);

	this->starting_obs = obs;
	this->solution->obs_network->activate(this->state,
										  obs);
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
	if (result > this->solution->max_val) {
		this->solution->max_val = result;
		this->solution->score_network_max_val = this->solution->max_val + (this->solution->max_val - this->solution->min_val)/2.0;
		this->solution->score_network_min_val = this->solution->min_val - (this->solution->max_val - this->solution->min_val)/2.0;
	} else if (result < this->solution->min_val) {
		this->solution->min_val = result;
		this->solution->score_network_max_val = this->solution->max_val + (this->solution->max_val - this->solution->min_val)/2.0;
		this->solution->score_network_min_val = this->solution->min_val - (this->solution->max_val - this->solution->min_val)/2.0;
	}

	if (this->iters_since_update < UPDATE_NUM_ITERS) {
		update_helper(this,
					  result);
	}

	if (this->iters_since_update < CREATE_EXPERIMENT_NUM_ITERS) {
		for (int d_index = 0; d_index < DIVERSITY_RANGE; d_index++) {
			if (this->experiment_histories[d_index].size() == 0) {
				create_experiment(this->scope_histories[0],
								  d_index,
								  this);
			}
		}
	}

	for (int d_index = 0; d_index < DIVERSITY_RANGE; d_index++) {
		if (this->experiment_histories[d_index].size() >= 2) {
			AbstractExperiment* keep_experiment = NULL;
			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[d_index].begin();
					it != this->experiment_histories[d_index].end(); it++) {
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
	}

	if (this->iters_since_update < UPDATE_NUM_ITERS) {
		this->existing_scope_histories.push_back(this->scope_histories[0]);
		this->existing_target_val_histories.push_back(result);
	} else {
		if (this->has_explore) {
			this->explore_starting_obs_histories.push_back(this->starting_obs);
			this->explore_scope_histories.push_back(this->scope_histories[0]);
			this->explore_target_val_histories.push_back(result);
		} else {
			delete this->scope_histories[0];
		}
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	if (this->existing_scope_histories.size() >= EXISTING_BATCH_SIZE) {
		train_existing_helper(this);
	}
	if (this->explore_scope_histories.size() >= EXPLORE_BATCH_SIZE) {
		train_explore_helper(this);
	}

	for (int d_index = 0; d_index < DIVERSITY_RANGE; d_index++) {
		if (this->experiment_histories[d_index].size() == 1) {
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
	}

	for (int d_index = 0; d_index < DIVERSITY_RANGE; d_index++) {
		for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories[d_index].begin();
				it != this->experiment_histories[d_index].end(); it++) {
			delete it->second;
		}
		this->experiment_histories[d_index].clear();
	}

	this->iters_since_update++;
	if (this->iters_since_update == UPDATE_NUM_ITERS) {
		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			Scope* scope = this->solution->scopes[s_index];
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				switch (it->second->type) {
				case NODE_TYPE_NOOP:
					{
						NoopNode* noop_node = (NoopNode*)it->second;
						for (int e_index = 0; e_index < (int)noop_node->experiments.size(); e_index++) {
							noop_node->experiments[e_index]->train_existing_helper(this);
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;
						for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
							action_node->experiments[e_index]->train_existing_helper(this);
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)it->second;
						for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
							scope_node->experiments[e_index]->train_existing_helper(this);
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)it->second;
						for (int e_index = 0; e_index < (int)branch_node->original_experiments.size(); e_index++) {
							branch_node->original_experiments[e_index]->train_existing_helper(this);
						}
						for (int e_index = 0; e_index < (int)branch_node->branch_experiments.size(); e_index++) {
							branch_node->branch_experiments[e_index]->train_existing_helper(this);
						}
					}
					break;
				}
			}
		}
	}
}
