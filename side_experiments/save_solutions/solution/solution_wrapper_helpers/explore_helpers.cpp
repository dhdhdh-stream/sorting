#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void SolutionWrapper::explore_init() {
	while (this->curr_experiment == NULL) {
		create_experiment(this);
	}

	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	this->experiment_overall_history = new BranchExperimentOverallHistory(this->curr_experiment);

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->experiment_context.push_back(NULL);
}

tuple<bool,bool,int> SolutionWrapper::explore_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;

	while (!is_next) {
		bool is_signal = check_signal(obs,
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

void SolutionWrapper::explore_end(double result) {
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

	this->experiment_overall_history->experiment->backprop(
		result,
		this);

	delete this->experiment_overall_history;
	this->experiment_overall_history = NULL;
	for (int i_index = 0; i_index < (int)this->experiment_instance_histories.size(); i_index++) {
		delete this->experiment_instance_histories[i_index];
	}
	this->experiment_instance_histories.clear();

	this->scope_histories.clear();
	this->node_context.clear();
	this->experiment_context.clear();

	if (this->curr_experiment != NULL) {
		if (this->curr_experiment->result == EXPERIMENT_RESULT_FAIL) {
			this->curr_experiment->clean();
			delete this->curr_experiment;

			this->curr_experiment = NULL;
		} else if (this->curr_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			this->curr_experiment->clean();

			if (this->best_experiment == NULL) {
				this->best_experiment = this->curr_experiment;
			} else {
				if (this->curr_experiment->improvement > this->best_experiment->improvement) {
					delete this->best_experiment;
					this->best_experiment = this->curr_experiment;
				} else {
					delete this->curr_experiment;
				}
			}

			this->curr_experiment = NULL;

			this->improvement_iter++;
			if (this->improvement_iter >= IMPROVEMENTS_PER_ITER) {
				cout << "update" << endl;

				delete this->solution;
				for (int s_index = 0; s_index < (int)this->positive_solutions.size(); s_index++) {
					if (this->positive_solutions[s_index] == this->best_experiment->resulting_solution) {
						this->positive_solutions.erase(this->positive_solutions.begin() + s_index);
						break;
					}
				}
				this->positive_solutions.push_back(new Solution(this->best_experiment->resulting_solution));
				this->solution = this->best_experiment->resulting_solution;

				cout << "this->best_experiment->resulting_solution->curr_val_average: " << this->best_experiment->resulting_solution->curr_val_average << endl;

				delete this->best_experiment;
				this->best_experiment = NULL;

				this->solution->timestamp++;

				this->improvement_iter = 0;
			}
		}
	}
}
