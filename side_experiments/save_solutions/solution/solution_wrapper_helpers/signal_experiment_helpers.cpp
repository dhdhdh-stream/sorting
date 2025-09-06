#include "solution_wrapper.h"

#include <iostream>

#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_experiment.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SIGNAL_IMPROVEMENTS_PER_ITER = 2;
#else
const int SIGNAL_IMPROVEMENTS_PER_ITER = 10;
#endif /* MDEBUG */

void SolutionWrapper::signal_experiment_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	Solution* current_solution;
	switch (this->curr_signal_experiment->solution_type) {
	case SIGNAL_EXPERIMENT_SOLUTION_TYPE_POSITIVE:
		current_solution = this->positive_solutions[
			this->curr_signal_experiment->solution_index];
		break;
	case SIGNAL_EXPERIMENT_SOLUTION_TYPE_TRAP:
		current_solution = this->trap_solutions[
			this->curr_signal_experiment->solution_index];
		break;
	case SIGNAL_EXPERIMENT_SOLUTION_TYPE_CURRENT:
		current_solution = this->solution;
		break;
	}

	ScopeHistory* scope_history = new ScopeHistory(current_solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(current_solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);
}

tuple<bool,bool,int> SolutionWrapper::signal_experiment_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;
	
	while (!is_next) {
		bool is_signal = signal_experiment_check_signal(
			obs,
			action,
			is_next,
			this);
		if (!is_signal) {
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
	}

	return tuple<bool,bool,int>{is_done, fetch_action, action};
}

void SolutionWrapper::signal_experiment_end(double result) {
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

	this->curr_signal_experiment->backprop(
		result,
		this);

	for (int i_index = 0; i_index < (int)this->signal_experiment_instance_histories.size(); i_index++) {
		delete this->signal_experiment_instance_histories[i_index];
	}
	this->signal_experiment_instance_histories.clear();

	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	if (this->curr_signal_experiment->state == SIGNAL_EXPERIMENT_STATE_DONE) {
		if (this->best_signal_experiment == NULL) {
			this->best_signal_experiment = this->curr_signal_experiment;
		} else {
			if (this->curr_signal_experiment->misguess_average < this->best_signal_experiment->misguess_average) {
				delete this->best_signal_experiment;
				this->best_signal_experiment = this->curr_signal_experiment;
			} else {
				delete this->curr_signal_experiment;
			}
		}
		this->curr_signal_experiment = NULL;

		this->improvement_iter++;
		if (this->improvement_iter >= SIGNAL_IMPROVEMENTS_PER_ITER) {
			this->best_signal_experiment->add(this);

			delete this->best_signal_experiment;
			this->best_signal_experiment = NULL;

			for (int s_index = 0; s_index < (int)this->trap_solutions.size(); s_index++) {
				delete this->trap_solutions[s_index];
			}
			this->trap_solutions.clear();

			this->improvement_iter = 0;
		} else {
			this->curr_signal_experiment = new SignalExperiment(0,
																this);
		}
	}
}
