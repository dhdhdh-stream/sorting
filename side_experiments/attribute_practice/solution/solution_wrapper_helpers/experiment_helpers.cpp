#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

const int RUN_TIMESTEPS = 100;

const int SNAPSHOT_ITERS = 10;

void SolutionWrapper::experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	if (this->curr_experiment != NULL) {
		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_EXPERIMENT:
			{
				Experiment* experiment = (Experiment*)this->curr_experiment;
				this->experiment_history = new ExperimentHistory(experiment);
			}
			break;
		}
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
			ScopeHistory* scope_history = this->scope_histories.back();
			scope_history->obs_history = obs;

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
	if (this->curr_experiment == NULL) {
		create_experiment(this->scope_histories[0],
						  this);
	} else {
		this->experiment_history->experiment->backprop(
		result,
		this);

		delete this->experiment_history;
		this->experiment_history = NULL;

		switch (this->curr_experiment->type) {
		case EXPERIMENT_TYPE_EXPERIMENT:
			if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
				this->curr_experiment->clean();
				delete this->curr_experiment;

				this->curr_experiment = NULL;
			} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
				this->curr_experiment->clean();

				Scope* last_updated_scope = this->curr_experiment->scope_context;

				this->curr_experiment->add(this);

				this->solution->curr_score = this->curr_experiment->calc_new_score();

				delete this->curr_experiment;
				this->curr_experiment = NULL;

				clean_scope(last_updated_scope);

				this->solution->clean_scopes();

				this->solution->timestamp++;
				// if (this->solution->timestamp >= RUN_TIMESTEPS) {
				// 	this->solution->timestamp = -1;
				// }

				// if (this->solution->timestamp % SNAPSHOT_ITERS == 0) {
				// 	double curr_score = 0.0;
				// 	for (int i_index = 0; i_index < 3; i_index++) {
				// 		curr_score += this->solution->improvement_history[this->solution->improvement_history.size()-1 - i_index];
				// 	}

				// 	double existing_score = 0.0;
				// 	if (this->solution_snapshot->improvement_history.size() > 0) {
				// 		for (int i_index = 0; i_index < 3; i_index++) {
				// 			existing_score += this->solution_snapshot->improvement_history[this->solution_snapshot->improvement_history.size()-1 - i_index];
				// 		}
				// 	}

				// 	if (curr_score > existing_score) {
				// 		delete this->solution_snapshot;
				// 		this->solution_snapshot = new Solution(this->solution);
				// 	} else {
				// 		delete this->solution;
				// 		this->solution = new Solution(this->solution_snapshot);
				// 	}
				// }
			}
			break;
		}
	}

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();
}
