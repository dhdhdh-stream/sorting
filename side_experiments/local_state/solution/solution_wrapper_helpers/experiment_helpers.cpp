#include "solution_wrapper.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int DIVERSITY_RANGE = 2;
#else
const int DIVERSITY_RANGE = 20;
#endif /* MDEBUG */

void SolutionWrapper::experiment_init(vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->iters_since_update < UPDATE_NUM_ITERS) {
		this->run_type = RUN_TYPE_EXISTING;
	} else {
		uniform_int_distribution<int> type_distribution(0, 1);
		this->run_type = type_distribution(generator);
	}

	uniform_int_distribution<int> diversity_distribution(0, DIVERSITY_RANGE-1);
	this->diversity_index = diversity_distribution(generator);

	this->run_num_actions = 0;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(this->solution->starting_scope->num_states);
	this->states.back().setConstant(0.0);

	this->solution->starting_scope->experiment_start_activate(
		obs,
		this);
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
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
	bool fetch_action = false;

	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.back()->experiment_callback_histories.size() > 0) {
				Scope* scope = this->scope_histories.back()->scope;
				scope->end_score_network->activate(this->states.back());
				double signal = scope->end_score_network->output->acti_vals(0);
				for (int c_index = 0; c_index < (int)this->scope_histories.back()->experiment_callback_histories.size(); c_index++) {
					ExploreExperimentHistory* explore_experiment_history = (ExploreExperimentHistory*)this->scope_histories.back()->experiment_callback_histories[c_index];
					int index = this->scope_histories.back()->experiment_callback_indexes[c_index];
					explore_experiment_history->signal_histories[index] = signal;
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
	if (this->run_type == RUN_TYPE_EXISTING) {
		update_helper(result,
					  this);
	}

	if (this->explore_experiment_histories.size() == 0) {
		if (this->run_type == RUN_TYPE_EXPLORE) {
			create_experiment(this->scope_histories[0],
							  this);
		}
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

	this->train_scope_histories.push_back(this->scope_histories[0]);
	this->train_target_val_histories.push_back(result);
	this->train_run_type_histories.push_back(this->run_type);
	if (this->train_scope_histories.size() >= BATCH_SIZE) {
		train_helper(this);
	}

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	this->states.clear();

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
	if (this->iters_since_update == UPDATE_NUM_ITERS) {
		if (this->solution->curr_score > this->best_solution->curr_score) {
			delete this->best_solution;
			this->best_solution = new Solution(this->solution);
		} else {
			if (this->solution->timestamp >= this->best_solution->timestamp + STUCK_NUM_ITERS) {
				this->best_solution->timestamp = -1;

				delete this->solution;
				this->solution = new Solution(this->best_solution);
			}
		}
	}
}
