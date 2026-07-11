#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "negate_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_REUSE_NUM_ITERS = 20;
#else
const int MEASURE_REUSE_NUM_ITERS = 2000;
#endif /* MDEBUG */

void ExploreExperiment::measure_reuse_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		double existing_predicted;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				noop_node->score_network->activate(wrapper->state);
				existing_predicted = noop_node->score_network->output->acti_vals(0);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				action_node->score_network->activate(wrapper->state);
				existing_predicted = action_node->score_network->output->acti_vals(0);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				scope_node->score_network->activate(wrapper->state);
				existing_predicted = scope_node->score_network->output->acti_vals(0);
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					branch_node->branch_network->activate(wrapper->state);
					existing_predicted = branch_node->branch_network->output->acti_vals(0);
				} else {
					branch_node->original_network->activate(wrapper->state);
					existing_predicted = branch_node->original_network->output->acti_vals(0);
				}
			}
			break;
		}

		this->measure_new_network->activate(wrapper->state);
		double new_predicted = this->measure_new_network->output->acti_vals(0);

		if (new_predicted >= existing_predicted) {
			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::measure_reuse_step(vector<double>& obs,
										   int& action,
										   bool& is_next,
										   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

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

void ExploreExperiment::measure_reuse_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::measure_reuse_backprop(double target_val,
											   ExploreExperimentHistory* history,
											   SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->new_sum_scores += target_val;
		this->new_count++;

		this->state_iter++;
		if (this->state_iter >= MEASURE_REUSE_NUM_ITERS) {
			cout << "measure_reuse" << endl;
			double existing_average = this->existing_sum_scores / (double)this->existing_count;
			cout << "existing_average: " << existing_average << endl;
			double new_average = this->new_sum_scores / (double)this->new_count;
			cout << "new_average: " << new_average << endl;

			double average_instances_per_run;
			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					average_instances_per_run = noop_node->average_instances_per_run;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					average_instances_per_run = action_node->average_instances_per_run;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					average_instances_per_run = scope_node->average_instances_per_run;
				}
				break;
			default:
			// case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						average_instances_per_run = branch_node->branch_average_instances_per_run;
					} else {
						average_instances_per_run = branch_node->original_average_instances_per_run;
					}
				}
				break;
			}
			cout << "average_instances_per_run: " << average_instances_per_run << endl;

			add(this->measure_new_network,
				wrapper);
			this->measure_new_network = NULL;

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
								}
							}
							break;
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)it->second;
								if (action_node->experiment != NULL) {
									delete action_node->experiment;
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)it->second;
								if (scope_node->experiment != NULL) {
									delete scope_node->experiment;
								}
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)it->second;
								if (branch_node->original_experiment != NULL) {
									delete branch_node->original_experiment;
								}
								if (branch_node->branch_experiment != NULL) {
									delete branch_node->branch_experiment;
								}
							}
							break;
						}
					}
				}

				wrapper->experiment_iter = 0;
			}
		}
	} else {
		this->existing_sum_scores += target_val;
		this->existing_count++;
	}
}