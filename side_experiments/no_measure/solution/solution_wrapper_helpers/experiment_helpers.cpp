#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

const int STUCK_NUM_ITERS = 10;

void SolutionWrapper::experiment_init() {
	this->iter++;

	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	uniform_int_distribution<int> explore_distribution(0, 1);
	if (explore_distribution(generator) == 0) {
		this->should_explore = true;
	} else {
		this->should_explore = false;
	}

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);
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
	if (this->should_explore) {
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
						keep_experiment->node_context->experiment = NULL;
						delete keep_experiment;

						keep_experiment = it->first;
					} else {
						it->first->node_context->experiment = NULL;
						delete it->first;
					}
				}
			}
		} else {
			set<Scope*> updated_scopes;
			for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
					it != this->explore_experiment_histories.end(); it++) {
				it->first->backprop(result,
									it->second,
									this,
									updated_scopes);
			}

			if (updated_scopes.size() > 0) {
				for (set<Scope*>::iterator it = updated_scopes.begin();
						it != updated_scopes.end(); it++) {
					clean_scope(*it);
				}

				this->solution->timestamp++;
				// if ((int)this->solution->improvement_history.size() >= STUCK_NUM_ITERS) {
				// 	double prev_val = this->solution->improvement_history[this->solution->improvement_history.size() - STUCK_NUM_ITERS];
				// 	bool improved = false;
				// 	for (int h_index = 0; h_index < STUCK_NUM_ITERS-1; h_index++) {
				// 		if (this->solution->improvement_history[this->solution->improvement_history.size() - 1 - h_index] > prev_val) {
				// 			improved = true;
				// 			break;
				// 		}
				// 	}

				// 	if (!improved) {
				// 		this->solution->timestamp = -1;
				// 	}
				// }
			}
		}

		for (map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it = this->explore_experiment_histories.begin();
				it != this->explore_experiment_histories.end(); it++) {
			delete it->second;
		}
		this->explore_experiment_histories.clear();
	} else {
		update_helper(this->scope_histories[0],
					  result);

		if (this->score_histories.size() < HISTORIES_NUM_SAVE) {
			this->score_histories.push_back(result);
		} else {
			this->score_histories[this->history_index] = result;

			this->history_index++;
			if (this->history_index >= HISTORIES_NUM_SAVE) {
				this->history_index = 0;
			}
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
