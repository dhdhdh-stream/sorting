#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::measure_check_activate(SolutionWrapper* wrapper) {
	if (this->select_percentage == 1.0) {
		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		double sum_vals = this->new_average_score;

		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->new_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->new_input_averages[i_index]) / this->new_input_standard_deviations[i_index];
				sum_vals += this->new_weights[i_index] * normalized_val;
			}
		}

		if (this->new_network != NULL) {
			vector<double> input_vals(this->new_network_inputs.size());
			vector<bool> input_is_on(this->new_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->new_network_inputs[i_index],
								   0,
								   val,
								   is_on);
				input_vals[i_index] = val;
				input_is_on[i_index] = is_on;
			}
			this->new_network->activate(input_vals,
										input_is_on);
			sum_vals += this->new_network->output->acti_vals[0];
		}

		bool decision_is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (sum_vals >= 0.0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		#endif /* MDEBUG */

		BranchNodeHistory* branch_node_history = new BranchNodeHistory(this->new_branch_node);
		branch_node_history->index = (int)scope_history->node_histories.size();
		scope_history->node_histories[this->new_branch_node->id] = branch_node_history;

		branch_node_history->is_branch = decision_is_branch;

		if (decision_is_branch) {
			BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void BranchExperiment::measure_step(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper,
									BranchExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->new_nodes.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		switch (this->new_nodes[experiment_state->step_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

				ActionNodeHistory* history = new ActionNodeHistory(node);
				history->index = (int)scope_history->node_histories.size();
				scope_history->node_histories[node->id] = history;

				action = node->action;
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

				ScopeNodeHistory* history = new ScopeNodeHistory(node);
				history->index = (int)scope_history->node_histories.size();
				scope_history->node_histories[node->id] = history;

				ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
				history->scope_history = inner_scope_history;
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(node->scope->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

				ObsNodeHistory* history = new ObsNodeHistory(node);
				history->index = (int)scope_history->node_histories.size();
				scope_history->node_histories[node->id] = history;

				history->obs_history = obs;

				history->factor_initialized = vector<bool>(node->factors.size(), false);
				history->factor_values = vector<double>(node->factors.size());

				experiment_state->step_index++;
			}
			break;
		}
	}
}

void BranchExperiment::measure_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

	if (node->scope->new_scope_experiment != NULL) {
		node->scope->new_scope_experiment->back_activate(wrapper);
	}

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::measure_backprop(double target_val,
										BranchExperimentHistory* history) {
	if (history->is_hit) {
		this->i_target_val_histories.push_back(target_val);

		this->state_iter++;
		if (this->state_iter == EARLY_SUCCESS_S1_ITERS
				|| this->state_iter == EARLY_SUCCESS_S2_ITERS) {
			double sum_score = 0.0;
			for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
				sum_score += this->i_target_val_histories[h_index];
			}
			double new_score = sum_score / (double)this->state_iter;

			double sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
				sum_variance += (this->i_target_val_histories[h_index] - new_score)
					* (this->i_target_val_histories[h_index] - new_score);
			}
			double new_standard_deviation = sqrt(sum_variance / (double)this->i_target_val_histories.size());
			if (new_standard_deviation < MIN_STANDARD_DEVIATION) {
				new_standard_deviation = MIN_STANDARD_DEVIATION;
			}

			double existing_score;
			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					existing_score = action_node->average_score;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					existing_score = scope_node->average_score;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						existing_score = branch_node->branch_average_score;
					} else {
						existing_score = branch_node->original_average_score;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;
					existing_score = obs_node->average_score;
				}
				break;
			}

			double t_score = (new_score - existing_score) / new_standard_deviation;
			if (t_score >= EARLY_SUCCESS_MIN_T_SCORE) {
				this->improvement = new_score - existing_score;

				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index];
					} else {
						cout << " E" << this->best_scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->best_exit_next_node == NULL) {
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "this->select_percentage: " << this->select_percentage << endl;

				cout << "this->improvement: " << this->improvement << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			}
		} else if (this->state_iter >= MEASURE_ITERS) {
			double sum_score = 0.0;
			for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
				sum_score += this->i_target_val_histories[h_index];
			}
			double new_score = sum_score / (double)this->state_iter;

			double existing_score;
			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					existing_score = action_node->average_score;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					existing_score = scope_node->average_score;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						existing_score = branch_node->branch_average_score;
					} else {
						existing_score = branch_node->original_average_score;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;
					existing_score = obs_node->average_score;
				}
				break;
			}
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->improvement = new_score - existing_score;

				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index];
					} else {
						cout << " E" << this->best_scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->best_exit_next_node == NULL) {
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "this->select_percentage: " << this->select_percentage << endl;

				cout << "this->improvement: " << this->improvement << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				cout << "BranchExperiment fail" << endl;
				cout << "new_score: " << new_score << endl;
				cout << "existing_score: " << existing_score << endl;
				cout << endl;

				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
