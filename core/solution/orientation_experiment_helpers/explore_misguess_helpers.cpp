#include "orientation_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void OrientationExperiment::explore_misguess_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		OrientationExperimentHistory* history) {
	run_helper.num_actions_limit = MAX_NUM_ACTIONS_LIMIT_MULTIPLIER * solution->explore_scope_max_num_actions;

	run_helper.num_decisions++;

	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_SCOPE:
						{
							InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
							if (info_scope_node_history->is_positive) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								input_vals[i_index] = 1.0;
							} else {
								input_vals[i_index] = -1.0;
							}
						}
						break;
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}

	double predicted_score = this->existing_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
	}
	if (this->existing_network != NULL) {
		vector<vector<double>> network_input_vals(this->existing_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
				network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
			}
		}
		this->existing_network->activate(network_input_vals);
		predicted_score += this->existing_network->output->acti_vals[0];
	}

	history->existing_predicted_score = predicted_score;

	vector<AbstractNode*> possible_exits;

	if (this->node_context->type == NODE_TYPE_ACTION
			&& ((ActionNode*)this->node_context)->next_node == NULL) {
		possible_exits.push_back(NULL);
	}

	AbstractNode* starting_node;
	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_INFO_BRANCH:
		{
			InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = info_branch_node->branch_next_node;
			} else {
				starting_node = info_branch_node->original_next_node;
			}
		}
		break;
	}

	this->scope_context->random_exit_activate(
		starting_node,
		possible_exits);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	this->exit_next_node = possible_exits[random_index];

	int new_num_steps;
	uniform_int_distribution<int> uniform_distribution(0, 1);
	geometric_distribution<int> geometric_distribution(0.5);
	if (random_index == 0) {
		new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
	} else {
		new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
	}

	uniform_int_distribution<int> default_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		// TODO: reuse from a separate pool of scopes?
		// bool default_to_action = true;
		// if (default_distribution(generator) != 0) {
		// 	ScopeNode* new_scope_node = create_existing();
		// 	if (new_scope_node != NULL) {
		// 		this->step_types.push_back(STEP_TYPE_SCOPE);
		// 		this->actions.push_back(NULL);

		// 		this->scopes.push_back(new_scope_node);

		// 		default_to_action = false;
		// 	}
		// }

		// if (default_to_action) {
			this->step_types.push_back(STEP_TYPE_ACTION);

			ActionNode* new_action_node = new ActionNode();
			new_action_node->action = problem_type->random_action();
			this->actions.push_back(new_action_node);

			this->scopes.push_back(NULL);
		// }
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->actions[s_index]->action);
		} else {
			this->scopes[s_index]->explore_activate(
				problem,
				context,
				run_helper);
		}
	}

	curr_node = this->exit_next_node;
}

void OrientationExperiment::explore_misguess_backprop(
		EvalHistory* outer_eval_history,
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.num_actions_limit > 0) {
	#else
	OrientationExperimentHistory* history = (OrientationExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	double curr_surprise;
	if (run_helper.num_actions_limit > 0) {
		double target_impact;
		if (context.size() == 1) {
			target_impact = problem->score_result(run_helper.num_decisions);
		} else {
			target_impact = context[context.size()-2].scope->eval->calc_impact(outer_eval_history);
		}

		double new_predicted_impact = this->eval_context->calc_impact(eval_history);

		double new_misguess = (target_impact - new_predicted_impact) * (target_impact - new_predicted_impact);
		curr_surprise = history->existing_predicted_score - new_misguess;
	}

	if (run_helper.num_actions_limit > 0
			&& curr_surprise >= solution->explore_scope_misguess_standard_deviation) {
	#endif /* MDEBUG */
		this->target_val_histories.reserve(NUM_DATAPOINTS);
		this->scope_histories.reserve(NUM_DATAPOINTS);

		this->new_score = 0.0;

		cout << "ORIENTATION_EXPERIMENT_STATE_EXPLORE_IMPACT" << endl;
		this->state = ORIENTATION_EXPERIMENT_STATE_EXPLORE_IMPACT;
		this->state_iter = 0;
	} else {
		for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
			if (this->step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->actions[s_index];
			} else {
				delete this->scopes[s_index];
			}
		}

		this->step_types.clear();
		this->actions.clear();
		this->scopes.clear();

		this->explore_iter++;
		if (this->explore_iter >= ORIENTATION_EXPERIMENT_EXPLORE_ITERS) {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}

	run_helper.num_actions_limit = -1;
}
