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

	if (this->iters_since_update < UPDATE_NUM_ITERS) {
		this->run_type = RUN_TYPE_EXISTING;
	} else {
		this->run_type = RUN_TYPE_EXPLORE;
	}

	this->state = vector<double>(this->solution->num_states, 0.0);
	if (this->run_type == RUN_TYPE_EXISTING) {
		uniform_int_distribution<int> partial_distribution(0, 3);
		if (partial_distribution(generator) == 0) {
			this->partial_state = vector<double>(this->solution->num_states, 0.0);
		} else {
			this->partial_state.clear();
		}
	} else {
		this->partial_state.clear();	// for debug
	}

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
	this->experiment_context.push_back(NULL);

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
	if (this->run_type == RUN_TYPE_EXISTING) {
		this->solution->curr_score = 0.999*this->solution->curr_score + 0.001*result;

		update_helper(this->scope_histories[0]);
	}
	update_helper(result,
				  this);

	if (this->run_type == RUN_TYPE_EXPLORE) {
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
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	if (this->run_type == RUN_TYPE_EXPLORE) {
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
	}

	this->iters_since_update++;
	if (this->iters_since_update == UPDATE_NUM_ITERS) {
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->prev_solution->curr_score > this->solution->curr_score) {
		#endif /* MDEBUG */
			// temp
			cout << "reset" << endl;
			cout << "this->prev_solution->curr_num_resets: " << this->prev_solution->curr_num_resets << endl;
			cout << "this->prev_solution->curr_score: " << this->prev_solution->curr_score << endl;
			cout << "this->solution->curr_score: " << this->solution->curr_score << endl;

			this->prev_solution->curr_num_resets++;
			// if (this->prev_solution->curr_num_resets >= STUCK_NUM_ITERS) {
			// 	this->prev_solution->timestamp = -1;
			// }

			delete this->solution;
			this->solution = new Solution(this->prev_solution);
		}
	}
}
