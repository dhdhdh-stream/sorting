#include "eval_pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void EvalPassThroughExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	if (this->info_scope == NULL) {
		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}
	} else {
		ScopeHistory* inner_scope_history;
		bool inner_is_positive;
		this->info_scope->activate(problem,
								   run_helper,
								   inner_scope_history,
								   inner_is_positive);

		delete inner_scope_history;

		if ((this->is_negate && !inner_is_positive)
				|| (!this->is_negate && inner_is_positive)) {
			if (this->step_types.size() == 0) {
				curr_node = this->exit_next_node;
			} else {
				if (this->step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->actions[0];
				} else {
					curr_node = this->scopes[0];
				}
			}
		}
	}
}

void EvalPassThroughExperiment::measure_backprop(
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	double starting_target_val;
	if (context.size() == 1) {
		starting_target_val = 1.46;
	} else {
		starting_target_val = context[context.size()-2].scope->eval->calc_score(
			run_helper,
			history->outer_eval_history->start_scope_history);
	}

	double starting_predicted_score;
	{
		vector<double> input_vals(this->score_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = eval_history->start_scope_history->node_histories.find(
				this->score_input_node_contexts[i_index]);
			if (it != eval_history->start_scope_history->node_histories.end()) {
				switch (this->score_input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->score_input_obs_indexes[i_index]];
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
			}
		}

		starting_predicted_score = this->score_average_score;
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			starting_predicted_score += input_vals[i_index] * this->score_linear_weights[i_index];
		}
		if (this->score_network != NULL) {
			vector<vector<double>> network_input_vals(this->score_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->score_network_input_indexes.size(); i_index++) {
				network_input_vals[i_index] = vector<double>(this->score_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->score_network_input_indexes[i_index].size(); v_index++) {
					network_input_vals[i_index][v_index] = input_vals[this->score_network_input_indexes[i_index][v_index]];
				}
			}
			this->score_network->activate(network_input_vals);
			starting_predicted_score += this->score_network->output->acti_vals[0];
		}
	}

	this->eval_context->activate(problem,
								 run_helper,
								 eval_history->end_scope_history);

	double ending_target_val;
	if (context.size() == 1) {
		ending_target_val = problem->score_result(run_helper.num_decisions);
	} else {
		context[context.size()-2].scope->eval->activate(
			problem,
			run_helper,
			history->outer_eval_history->end_scope_history);
		double ending_target_vs = context[context.size()-2].scope->eval->calc_vs(
			run_helper,
			history->outer_eval_history);
		ending_target_val = starting_target_val + ending_target_vs;
	}

	double ending_predicted_score;
	{
		vector<double> input_vals(this->score_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = eval_history->end_scope_history->node_histories.find(
				this->score_input_node_contexts[i_index]);
			if (it != eval_history->end_scope_history->node_histories.end()) {
				switch (this->score_input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->score_input_obs_indexes[i_index]];
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
			}
		}

		ending_predicted_score = this->score_average_score;
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			ending_predicted_score += input_vals[i_index] * this->score_linear_weights[i_index];
		}
		if (this->score_network != NULL) {
			vector<vector<double>> network_input_vals(this->score_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->score_network_input_indexes.size(); i_index++) {
				network_input_vals[i_index] = vector<double>(this->score_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->score_network_input_indexes[i_index].size(); v_index++) {
					network_input_vals[i_index][v_index] = input_vals[this->score_network_input_indexes[i_index][v_index]];
				}
			}
			this->score_network->activate(network_input_vals);
			ending_predicted_score += this->score_network->output->acti_vals[0];
		}
	}

	double ending_misguess = (ending_target_val - ending_predicted_score) * (ending_target_val - ending_predicted_score);
	this->score_misguess_histories.push_back(ending_misguess);

	double ending_vs_predicted_score;
	{
		vector<double> input_vals(this->vs_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it;
			bool does_contain;
			if (this->vs_input_is_start[i_index]) {
				it = eval_history->start_scope_history->node_histories.find(this->vs_input_node_contexts[i_index]);
				if (it != eval_history->start_scope_history->node_histories.end()) {
					does_contain = true;
				} else {
					does_contain = false;
				}
			} else {
				it = eval_history->end_scope_history->node_histories.find(this->vs_input_node_contexts[i_index]);
				if (it != eval_history->end_scope_history->node_histories.end()) {
					does_contain = true;
				} else {
					does_contain = false;
				}
			}
			if (does_contain) {
				switch (this->vs_input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->vs_input_obs_indexes[i_index]];
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
			}
		}

		double vs_score = this->vs_average_score;
		for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
			vs_score += input_vals[i_index] * this->vs_linear_weights[i_index];
		}
		if (this->vs_network != NULL) {
			vector<vector<double>> network_input_vals(this->vs_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->vs_network_input_indexes.size(); i_index++) {
				network_input_vals[i_index] = vector<double>(this->vs_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->vs_network_input_indexes[i_index].size(); v_index++) {
					network_input_vals[i_index][v_index] = input_vals[this->vs_network_input_indexes[i_index][v_index]];
				}
			}
			this->vs_network->activate(network_input_vals);
			vs_score += this->vs_network->output->acti_vals[0];
		}

		ending_vs_predicted_score = starting_predicted_score + vs_score;
	}

	double ending_vs_misguess = (ending_target_val - ending_vs_predicted_score) * (ending_target_val - ending_vs_predicted_score);
	this->vs_misguess_histories.push_back(ending_vs_misguess);

	if ((int)this->vs_misguess_histories.size() >= NUM_DATAPOINTS) {
		double sum_score_misguesses = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_score_misguesses += this->score_misguess_histories[m_index];
		}
		double new_score_average_misguess = sum_score_misguesses / NUM_DATAPOINTS;

		double sum_score_misguess_variance = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_score_misguess_variance += (this->score_misguess_histories[m_index] - this->existing_score_average_misguess) * (this->score_misguess_histories[m_index] - this->existing_score_average_misguess);
		}
		double new_score_misguess_standard_deviation = sqrt(sum_score_misguess_variance / NUM_DATAPOINTS);
		if (new_score_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			new_score_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->score_misguess_histories.clear();

		double sum_vs_misguesses = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_vs_misguesses += this->vs_misguess_histories[m_index];
		}
		double new_vs_average_misguess = sum_vs_misguesses / NUM_DATAPOINTS;

		double sum_vs_misguess_variance = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_vs_misguess_variance += (this->vs_misguess_histories[m_index] - this->existing_vs_average_misguess) * (this->vs_misguess_histories[m_index] - this->existing_vs_average_misguess);
		}
		double new_vs_misguess_standard_deviation = sqrt(sum_vs_misguess_variance / NUM_DATAPOINTS);
		if (new_vs_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			new_vs_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->vs_misguess_histories.clear();

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_improvement = this->existing_score_average_misguess - new_score_average_misguess;
		double score_standard_deviation = min(this->existing_score_misguess_standard_deviation, new_score_misguess_standard_deviation);
		double score_t_score = score_improvement / (score_standard_deviation / sqrt(NUM_DATAPOINTS));

		double vs_improvement = this->existing_vs_average_misguess - new_vs_average_misguess;
		double vs_standard_deviation = min(this->existing_vs_misguess_standard_deviation, new_vs_misguess_standard_deviation);
		double vs_t_score = vs_improvement / (vs_standard_deviation / sqrt(NUM_DATAPOINTS));

		if ((score_t_score > 1.645 && vs_t_score >= 0.0)
				|| (score_t_score >= 0.0 && vs_t_score > 1.645)) {
		#endif /* MDEBUG */
			cout << "EvalPassThrough" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			if (this->info_scope != NULL) {
				cout << "this->info_scope->id: " << this->info_scope->id << endl;
			}
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->actions[s_index]->action.move;
				} else if (this->step_types[s_index] == STEP_TYPE_SCOPE) {
					cout << " E";
				}
			}
			cout << endl;

			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}

			cout << "this->existing_score_average_misguess: " << this->existing_score_average_misguess << endl;
			cout << "new_score_average_misguess: " << new_score_average_misguess << endl;
			cout << "this->existing_vs_average_misguess: " << this->existing_vs_average_misguess << endl;
			cout << "new_vs_average_misguess: " << new_vs_average_misguess << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
