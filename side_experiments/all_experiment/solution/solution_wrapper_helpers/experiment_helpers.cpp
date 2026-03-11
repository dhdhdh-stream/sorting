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

const int NON_OUTER_ITERS = 15;
const int OUTER_ITERS = 8;

void SolutionWrapper::experiment_init() {
	this->iter++;

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

	if (this->solution->score_histories.size() < HISTORIES_NUM_SAVE) {
		this->solution->score_histories.push_back(result);
	} else {
		this->solution->score_histories[this->solution->score_index] = result;
		this->solution->score_index++;
		if (this->solution->score_index >= HISTORIES_NUM_SAVE) {
			this->solution->score_index = 0;
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

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

	for (set<Scope*>::iterator it = updated_scopes.begin();
			it != updated_scopes.end(); it++) {
		clean_scope(*it);
	}

	switch (this->solution->state) {
	case SOLUTION_STATE_NON_OUTER:
		if (this->solution->timestamp >= NON_OUTER_ITERS) {
			this->solution->timestamp = -1;
		}
		break;
	case SOLUTION_STATE_OUTER:
		if (this->solution->timestamp >= OUTER_ITERS) {
			this->solution->state = SOLUTION_STATE_NON_OUTER;
			this->solution->timestamp = 0;

			this->solution->merge_outer();
			create_experiments(this);
		}
		break;
	}
}
