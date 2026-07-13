#include "explore_experiment.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "negate_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_REUSE_NUM_ITERS = 20;
#else
const int MEASURE_REUSE_NUM_ITERS = 10000;
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

		// history->branch_node_verify_states.push_back(wrapper->state);

		// // temp
		// if (this->state_iter < VERIFY_NUM_ITERS) {
		// 	cout << "this->state_iter: " << this->state_iter << endl;
		// 	#if defined(MDEBUG) && MDEBUG
		// 	cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
		// 	#endif /* MDEBUG */
		// 	wrapper->problem->print();
		// }

		bool is_branch;
		if (new_predicted >= existing_predicted) {
			is_branch = true;
		} else {
			is_branch = false;
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		if (is_branch) {
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
	// ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		// if (this->best_step_types.size() > 0) {
		// 	history->new_node_verify_states[experiment_state->step_index-1].push_back(wrapper->state);
		// }

		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		// if (experiment_state->step_index > 0) {
		// 	history->new_node_verify_states[experiment_state->step_index-1].push_back(wrapper->state);
		// }

		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			ActionNetwork* action_network = wrapper->solution->generic_action_networks[this->best_actions[experiment_state->step_index]];
			action_network->activate(wrapper->state);
			ActionNetworkHistory* action_network_history = new ActionNetworkHistory(action_network);
			action_network->save(action_network_history);
			wrapper->network_histories.push_back(action_network_history);

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			this->best_scopes[experiment_state->step_index]->experiment_start_activate(
				obs,
				wrapper);
		}
	}
}

void ExploreExperiment::measure_reuse_callback(vector<double>& obs,
											   SolutionWrapper* wrapper) {
	ObsNetwork* obs_network = wrapper->solution->generic_obs_network;
	obs_network->activate(wrapper->state,
						  obs);
	ObsNetworkHistory* obs_network_history = new ObsNetworkHistory(obs_network);
	obs_network->save(obs_network_history);
	wrapper->network_histories.push_back(obs_network_history);
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
		// if (this->state_iter < VERIFY_NUM_ITERS) {
		// 	this->verify_problems.push_back(wrapper->problem->copy_and_reset());
		// 	#if defined(MDEBUG) && MDEBUG
		// 	this->verify_starting_run_seeds.push_back(wrapper->starting_run_seed);
		// 	#endif /* MDEBUG */
		// 	this->branch_node_verify_states.insert(this->branch_node_verify_states.end(),
		// 		history->branch_node_verify_states.begin(), history->branch_node_verify_states.end());
		// 	for (int a_index = 0; a_index < (int)this->best_step_types.size(); a_index++) {
		// 		this->new_node_verify_states[a_index].insert(this->new_node_verify_states[a_index].end(),
		// 			history->new_node_verify_states[a_index].begin(), history->new_node_verify_states[a_index].end());
		// 	}
		// }

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