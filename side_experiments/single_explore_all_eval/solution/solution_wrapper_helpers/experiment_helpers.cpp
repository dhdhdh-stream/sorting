#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "eval_experiment.h"
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

const int OUTER_ITERS = 8;

const int TARGET_NODES_PER_EVAL = 10;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_explore_experiment != NULL) {
		this->explore_experiment_history = new ExploreExperimentHistory(this->curr_explore_experiment);

		// this->is_damage = this->curr_explore_experiment->is_damage;
		this->is_damage = false;
	} else {
		this->eval_iter++;

		uniform_int_distribution<int> is_damage_distribution(0, 1);
		// this->is_damage = is_damage_distribution(generator) == 0;
		this->is_damage = false;
	}

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
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
	if (this->curr_explore_experiment == NULL) {
		if (this->is_damage) {
			if (this->damage_score_histories.size() < HISTORIES_NUM_SAVE) {
				this->damage_score_histories.push_back(result);
			} else {
				this->damage_score_histories[this->damage_history_index] = result;

				this->damage_history_index++;
				if (this->damage_history_index >= HISTORIES_NUM_SAVE) {
					this->damage_history_index = 0;
				}
			}
		} else {
			if (this->clean_score_histories.size() < HISTORIES_NUM_SAVE) {
				this->clean_score_histories.push_back(result);
			} else {
				this->clean_score_histories[this->clean_history_index] = result;

				this->clean_history_index++;
				if (this->clean_history_index >= HISTORIES_NUM_SAVE) {
					this->clean_history_index = 0;
				}
			}
		}

		set<Scope*> updated_scopes;
		for (map<EvalExperiment*, EvalExperimentHistory*>::iterator it = this->eval_experiment_histories.begin();
				it != this->eval_experiment_histories.end(); it++) {
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
			// switch (this->solution->state) {
			// case SOLUTION_STATE_NON_OUTER:
			// 	if ((int)this->solution->clean_improvement_history.size() >= STUCK_NUM_ITERS) {
			// 		double prev_val = this->solution->clean_improvement_history[this->solution->clean_improvement_history.size() - STUCK_NUM_ITERS];
			// 		bool improved = false;
			// 		for (int h_index = 0; h_index < STUCK_NUM_ITERS-1; h_index++) {
			// 			if (this->solution->clean_improvement_history[this->solution->clean_improvement_history.size() - 1 - h_index] > prev_val) {
			// 				improved = true;
			// 				break;
			// 			}
			// 		}

			// 		if (!improved) {
			// 			this->solution->timestamp = -1;
			// 		}
			// 	}
			// 	break;
			// case SOLUTION_STATE_OUTER:
			// 	if (this->solution->timestamp >= OUTER_ITERS) {
			// 		this->solution->state = SOLUTION_STATE_NON_OUTER;
			// 		this->solution->timestamp = 0;

			// 		this->solution->merge_outer();
			// 	}
			// 	break;
			// }
		} else {
			int node_count = 0;
			int eval_count = 0;
			count_eval_helper(this->scope_histories[0],
							  node_count,
							  eval_count);

			int target_count = (node_count + (TARGET_NODES_PER_EVAL-1)) / TARGET_NODES_PER_EVAL;
			if (eval_count < target_count) {
				// cout << "node_count: " << node_count << endl;
				// cout << "eval_count: " << eval_count << endl;
				// cout << "target_count: " << target_count << endl;

				create_experiment(this->scope_histories[0],
								  this);
			}
		}
	} else {
		this->explore_experiment_history->experiment->backprop(
			result,
			this);

		delete this->explore_experiment_history;
		this->explore_experiment_history = NULL;
	}

	for (map<EvalExperiment*, EvalExperimentHistory*>::iterator it = this->eval_experiment_histories.begin();
			it != this->eval_experiment_histories.end(); it++) {
		delete it->second;
	}
	this->eval_experiment_histories.clear();

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
