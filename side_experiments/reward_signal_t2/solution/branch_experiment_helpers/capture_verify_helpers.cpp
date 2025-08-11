#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::capture_verify_check_activate(SolutionWrapper* wrapper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = wrapper->problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = wrapper->starting_run_seed;

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

			cout << i_index << ": " << val << endl;
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

		cout << "this->new_network->output->acti_vals[0]: " << this->new_network->output->acti_vals[0] << endl;
	}

	this->verify_scores.push_back(sum_vals);

	cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
	cout << "wrapper->curr_run_seed: " << wrapper->curr_run_seed << endl;
	wrapper->problem->print();

	bool decision_is_branch;
	if (wrapper->curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);

	cout << "decision_is_branch: " << decision_is_branch << endl;

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

void BranchExperiment::capture_verify_step(vector<double>& obs,
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

				experiment_state->step_index++;
			}
			break;
		}
	}
}

void BranchExperiment::capture_verify_exit_step(SolutionWrapper* wrapper,
												BranchExperimentState* experiment_state) {
	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::capture_verify_backprop(SolutionWrapper* wrapper) {
	BranchExperimentOverallHistory* overall_history = (BranchExperimentOverallHistory*)wrapper->experiment_overall_history;
	if (overall_history->is_hit) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */