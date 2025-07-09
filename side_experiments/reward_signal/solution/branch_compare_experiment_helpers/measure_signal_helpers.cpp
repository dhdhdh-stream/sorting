#include "branch_compare_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchCompareExperiment::measure_signal_check_activate(
		SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	double sum_vals = this->new_signal_average_score;

	for (int i_index = 0; i_index < (int)this->new_signal_inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   this->new_signal_inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - this->new_signal_input_averages[i_index]) / this->new_signal_input_standard_deviations[i_index];
			sum_vals += this->new_signal_weights[i_index] * normalized_val;
		}
	}

	if (this->new_signal_network != NULL) {
		vector<double> input_vals(this->new_signal_network_inputs.size());
		vector<bool> input_is_on(this->new_signal_network_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_signal_network_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->new_signal_network_inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->new_signal_network->activate(input_vals,
										   input_is_on);
		sum_vals += this->new_signal_network->output->acti_vals[0];
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
		BranchCompareExperimentState* new_experiment_state = new BranchCompareExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchCompareExperiment::measure_signal_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper,
		BranchCompareExperimentState* experiment_state) {
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

				experiment_state->step_index++;
			}
			break;
		}
	}
}

void BranchCompareExperiment::measure_signal_exit_step(
		SolutionWrapper* wrapper,
		BranchCompareExperimentState* experiment_state) {
	ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

	node->scope->back_activate(wrapper);

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchCompareExperiment::measure_signal_backprop(
		double target_val,
		BranchCompareExperimentHistory* history) {
	if (history->is_hit) {
		this->signal_target_val_histories.push_back(target_val);

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			double sum_true_score = 0.0;
			for (int h_index = 0; h_index < (int)this->true_target_val_histories.size(); h_index++) {
				sum_true_score += this->true_target_val_histories[h_index];
			}
			double true_score = sum_true_score / (double)MEASURE_ITERS;

			double sum_signal_score = 0.0;
			for (int h_index = 0; h_index < (int)this->signal_target_val_histories.size(); h_index++) {
				sum_signal_score += this->signal_target_val_histories[h_index];
			}
			double signal_score = sum_signal_score / (double)MEASURE_ITERS;

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

			cout << "BranchCompareExperiment" << endl;
			cout << "true_score: " << true_score << endl;
			cout << "signal_score: " << signal_score << endl;
			cout << "existing_score: " << existing_score << endl;
			cout << endl;

			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
