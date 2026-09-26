#include "solution_wrapper.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->iters_since_update < UPDATE_FULL_NUM_ITERS) {
		this->should_explore = false;
	} else {
		this->should_explore = true;
	}

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	if (this->experiment_context.back() == NULL
			&& this->node_context.back() != NULL
			&& this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		action_node->experiment_step_callback(obs,
											  this);
	}

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
					scope_node->experiment_exit_step(obs,
													 this);
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
	if (!this->should_explore) {
		update_helper(this,
					  result);
	}

	if (this->explore_experiment_histories.size() == 0) {
		create_experiment(this->scope_histories[0],
						  this);
	} else if (this->explore_experiment_histories.size() >= 2) {
		ExploreExperiment* keep_experiment = NULL;
		for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
				it != this->explore_experiment_histories.end(); it++) {
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

	if (!this->should_explore) {
		this->train_scope_histories.push_back(this->scope_histories[0]);
		this->train_target_val_histories.push_back(result);
		if (this->train_scope_histories.size() >= BATCH_SIZE) {
			train_helper(this);
		}
	} else {
		/**
		 * - training on explore significantly hurts results
		 *   - even if, e.g., training only post explore
		 */
		delete this->scope_histories[0];
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	if (this->explore_experiment_histories.size() == 1) {
		for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
				it != this->explore_experiment_histories.end(); it++) {
			it->first->backprop(result,
								it->second,
								this);
		}
	}

	for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
			it != this->explore_experiment_histories.end(); it++) {
		delete it->second;
	}
	this->explore_experiment_histories.clear();

	this->iters_since_update++;
	if (this->iters_since_update == UPDATE_HALF_NUM_ITERS) {
		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			Scope* scope = this->solution->scopes[s_index];
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_BRANCH) {
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->just_added = false;
				}
			}
		}
	}

	if ((int)this->to_add.size() > this->solution->timestamp) {
		if (this->solution->curr_score > this->solution->curr_score) {
			delete this->best_solution;
			this->best_solution = new Solution(this->solution);
		}

		for (int e_index = 0; e_index < (int)this->to_add.size(); e_index++) {
			this->to_add[e_index]->add(this);
			delete this->to_add[e_index];
		}

		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			Scope* scope = this->solution->scopes[s_index];
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				switch (it->second->type) {
				case NODE_TYPE_NOOP:
					{
						NoopNode* noop_node = (NoopNode*)it->second;
						if (noop_node->experiment != NULL) {
							delete noop_node->experiment;
							noop_node->experiment = NULL;
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;
						if (action_node->experiment != NULL) {
							delete action_node->experiment;
							action_node->experiment = NULL;
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)it->second;
						if (scope_node->experiment != NULL) {
							delete scope_node->experiment;
							scope_node->experiment = NULL;
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)it->second;
						if (branch_node->original_experiment != NULL) {
							delete branch_node->original_experiment;
							branch_node->original_experiment = NULL;
						}
						if (branch_node->branch_experiment != NULL) {
							delete branch_node->branch_experiment;
							branch_node->branch_experiment = NULL;
						}
					}
					break;
				}
			}
		}

		this->solution->timestamp++;

		this->iters_since_update = 0;
		this->new_since_update = 0;
		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			Scope* scope = this->solution->scopes[s_index];
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_BRANCH) {
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->original_network->clear_momentum();
					branch_node->branch_network->clear_momentum();
				}
			}
		}
		for (int h_index = 0; h_index < (int)this->train_scope_histories.size(); h_index++) {
			delete this->train_scope_histories[h_index];
		}
		this->train_scope_histories.clear();
		this->train_target_val_histories.clear();
	}
}
