#include "eval_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void EvalExperiment::capture_existing_activate(
		vector<ContextLayer>& context,
		EvalExperimentHistory* history) {
	this->existing_decision_scope_histories.push_back(new ScopeHistory(context.back().scope_history));

	context.back().scope_history->experiment_history = history;
}

void EvalExperiment::capture_existing_back_activate(
		vector<ContextLayer>& context) {
	this->existing_final_scope_histories.push_back(new ScopeHistory(context.back().scope_history));
}

void EvalExperiment::capture_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	while (this->existing_target_val_histories.size() < this->existing_decision_scope_histories.size()) {
		this->existing_target_val_histories.push_back(target_val);
	}

	if (this->existing_decision_scope_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_scores += this->existing_target_val_histories[d_index];
		}
		this->original_average_score = sum_scores / NUM_DATAPOINTS;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_score_variance += (this->existing_target_val_histories[d_index] - this->original_average_score) * (this->existing_target_val_histories[d_index] - this->original_average_score);
		}
		this->original_score_standard_deviation = sqrt(sum_score_variance / NUM_DATAPOINTS);
		if (this->original_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->original_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		vector<double> misguesses(NUM_DATAPOINTS);
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			vector<double> input_vals(solution->eval->input_node_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)solution->eval->input_node_contexts.size(); i_index++) {
				int curr_layer = 0;
				ScopeHistory* curr_scope_history = this->existing_final_scope_histories[d_index];
				while (true) {
					map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
						solution->eval->input_node_contexts[i_index][curr_layer]);
					if (it == curr_scope_history->node_histories.end()) {
						break;
					} else {
						if (curr_layer == (int)solution->eval->input_node_contexts[i_index].size()-1) {
							if (it->first->type == NODE_TYPE_ACTION) {
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								input_vals[i_index] = action_node_history->obs_snapshot[solution->eval->input_obs_indexes[i_index]];
							} else {
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								if (branch_node_history->is_branch) {
									input_vals[i_index] = 1.0;
								} else {
									input_vals[i_index] = -1.0;
								}
							}
							break;
						} else {
							curr_layer++;
							curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
						}
					}
				}
			}

			double score = solution->eval->average_score;
			for (int i_index = 0; i_index < (int)solution->eval->linear_input_indexes.size(); i_index++) {
				score += input_vals[solution->eval->linear_input_indexes[i_index]] * solution->eval->linear_weights[i_index];
			}
			if (solution->eval->network != NULL) {
				vector<vector<double>> network_input_vals(solution->eval->network_input_indexes.size());
				for (int i_index = 0; i_index < (int)solution->eval->network_input_indexes.size(); i_index++) {
					network_input_vals[i_index] = vector<double>(solution->eval->network_input_indexes[i_index].size());
					for (int v_index = 0; v_index < (int)solution->eval->network_input_indexes[i_index].size(); v_index++) {
						network_input_vals[i_index][v_index] = input_vals[solution->eval->network_input_indexes[i_index][v_index]];
					}
				}
				solution->eval->network->activate(network_input_vals);
				score += solution->eval->network->output->acti_vals[0];
			}

			misguesses[d_index] = (this->existing_target_val_histories[d_index] - score) * (this->existing_target_val_histories[d_index] - score);
		}

		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_misguesses += misguesses[d_index];
		}
		this->original_average_misguess = sum_misguesses / NUM_DATAPOINTS;

		double sum_misguess_variance = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_misguess_variance += (misguesses[d_index] - this->original_average_misguess) * (misguesses[d_index] - this->original_average_misguess);
		}
		this->original_misguess_standard_deviation = sqrt(sum_misguess_variance / NUM_DATAPOINTS);
		if (this->original_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->original_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		vector<AbstractNode*> possible_exits;

		if (this->node_context->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context)->next_node == NULL) {
			possible_exits.push_back(NULL);
		}

		AbstractNode* starting_node;
		if (this->node_context->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		} else {
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}

		solution->eval->subscope->random_exit_activate(
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

		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->action = problem_type->random_action();
			this->actions.push_back(new_action_node);
		}

		this->new_score = 0.0;

		this->state = EVAL_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
