#include "eval_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"

using namespace std;

void EvalPassThroughExperiment::verify_activate(
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

void EvalPassThroughExperiment::verify_back_activate(
		ScopeHistory*& subscope_history,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = subscope_history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != subscope_history->node_histories.end()) {
			switch (this->input_node_contexts[i_index]->type) {
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
		}
	}

	double score = this->new_average_score;
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		score += input_vals[i_index] * this->linear_weights[i_index];
	}
	if (this->network != NULL) {
		vector<vector<double>> network_input_vals(this->network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = input_vals[this->network_input_indexes[i_index][v_index]];
			}
		}
		this->network->activate(network_input_vals);
		score += this->network->output->acti_vals[0];
	}

	history->predicted_scores.push_back(score);
}

void EvalPassThroughExperiment::verify_backprop(
		double target_val,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int p_index = 0; p_index < (int)history->predicted_scores.size(); p_index++) {
		double misguess = (target_val - history->predicted_scores[p_index]) * (target_val - history->predicted_scores[p_index]);
		this->misguess_histories.push_back(misguess);
	}

	this->state_iter++;
	if (this->state == EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
		#if defined(MDEBUG) && MDEBUG
		this->misguess_histories.clear();

		if (rand()%2 == 0) {
		#else
		int num_instances = (int)this->misguess_histories.size();

		double sum_misguesses = 0.0;
		for (int m_index = 0; m_index < num_instances; m_index++) {
			sum_misguesses += this->misguess_histories[m_index];
		}
		double new_average_misguess = sum_misguesses / num_instances;

		this->misguess_histories.clear();

		if (new_average_misguess < this->existing_average_misguess) {
		#endif /* MDEBUG */
			this->misguess_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

			this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
		int num_instances = (int)this->misguess_histories.size();

		double sum_misguesses = 0.0;
		for (int m_index = 0; m_index < num_instances; m_index++) {
			sum_misguesses += this->misguess_histories[m_index];
		}
		double new_average_misguess = sum_misguesses / num_instances;

		this->misguess_histories.clear();

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (new_average_misguess < this->existing_average_misguess) {
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

			cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
			cout << "new_average_misguess: " << new_average_misguess << endl;

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
