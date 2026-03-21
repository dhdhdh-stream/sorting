#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

const int NON_OUTER_ITERS = 20;
const int OUTER_ITERS = 8;

const int TARGET_NODES_PER_EVAL = 5;

void SolutionWrapper::experiment_init() {
	this->num_experiments = 0;
	this->run_is_fail = false;

	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);

	existing_clean_result_helper(this,
								 this->prev_clean_result,
								 this->prev_signal);
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
													   is_done,
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
	while (true) {
		if (this->node_context.back() == NULL
				&& this->experiment_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				break;
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
			delete this->experiment_context.back();
			this->experiment_context.back() = NULL;
		} else {
			this->node_context.back() = NULL;
		}
	}

	if (!this->run_is_fail) {
		if (this->solution->score_histories.size() < HISTORIES_NUM_SAVE) {
			this->solution->score_histories.push_back(result);
			this->solution->num_experiment_histories.push_back(this->num_experiments);
		} else {
			this->solution->score_histories[this->solution->score_index] = result;
			this->solution->num_experiment_histories[this->solution->score_index] = this->num_experiments;
			this->solution->score_index++;
			if (this->solution->score_index >= HISTORIES_NUM_SAVE) {
				this->solution->score_index = 0;
			}
		}

		this->eval_iter++;
	}

	set<Scope*> updated_scopes;
	for (map<Experiment*, ExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		it->first->backprop(result,
							it->second,
							this,
							updated_scopes);
		delete it->second;
	}
	this->experiment_histories.clear();

	if (updated_scopes.size() > 0) {
		for (set<Scope*>::iterator it = updated_scopes.begin();
				it != updated_scopes.end(); it++) {
			clean_scope(*it);
		}

		// switch (this->solution->state) {
		// case SOLUTION_STATE_NON_OUTER:
		// 	if (this->solution->timestamp >= NON_OUTER_ITERS) {
		// 		this->solution->timestamp = -1;
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

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
