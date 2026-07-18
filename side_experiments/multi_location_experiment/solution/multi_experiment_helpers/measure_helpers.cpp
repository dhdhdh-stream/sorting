#include "multi_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void MultiExperiment::measure_check_activate(vector<double>& obs,
											 MultiExperimentHistory* history,
											 SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->existing_network->activate(obs);
		this->new_network->activate(obs);
		if (this->new_network->output->acti_vals[0] >= this->existing_network->output->acti_vals[0]) {
			MultiExperimentState* new_experiment_state = new MultiExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void MultiExperiment::measure_step(vector<double>& obs,
								   int& action,
								   bool& is_next,
								   SolutionWrapper* wrapper) {
	MultiExperimentState* experiment_state = (MultiExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		/**
		 * - wrapper->node_context.back() unchanged
		 */

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void MultiExperiment::measure_exit_step(SolutionWrapper* wrapper) {
	MultiExperimentState* experiment_state = (MultiExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void MultiExperiment::measure_backprop(
		double target_val,
		MultiExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->sum_vals += target_val;

		this->state_iter++;
		if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
			double new_average = this->sum_vals / this->state_iter;

			cout << "this->existing_average: " << this->existing_average << endl;
			cout << "new_average: " << new_average << endl;

			add(wrapper);

			delete this;

			wrapper->experiment_iter++;
			if (wrapper->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
				for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
					Scope* scope = wrapper->solution->scopes[s_index];
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

				wrapper->experiment_iter = 0;
			}
		}
	}
}
