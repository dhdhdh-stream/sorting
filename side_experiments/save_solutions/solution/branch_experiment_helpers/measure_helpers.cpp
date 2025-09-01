#include "branch_experiment.h"

#include <cmath>
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
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EARLY_FAIL_1_NUM_ITERS = 2;
const int EARLY_FAIL_2_NUM_ITERS = 2;
const int EARLY_FAIL_3_NUM_ITERS = 2;
#else
const int EARLY_FAIL_1_NUM_ITERS = 20;
const int EARLY_FAIL_2_NUM_ITERS = 60;
const int EARLY_FAIL_3_NUM_ITERS = 200;
#endif /* MDEBUG */

void BranchExperiment::measure_check_activate(SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
	wrapper->experiment_instance_histories.push_back(instance_history);
	for (int l_index = (int)wrapper->scope_histories.size()-1; l_index >= 0; l_index--) {
		map<int, Signal*>::iterator it = wrapper->signals.find(wrapper->scope_histories[l_index]->scope->id);
		if (it != wrapper->signals.end()) {
			instance_history->signal_needed_from = wrapper->scope_histories[l_index];
			break;
		}
	}

	if (this->select_percentage == 1.0) {
		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	} else {
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

				experiment_state->step_index++;
			}
			break;
		}
	}
}

void BranchExperiment::measure_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::measure_backprop(double target_val,
										SolutionWrapper* wrapper) {
	BranchExperimentOverallHistory* overall_history = (BranchExperimentOverallHistory*)wrapper->experiment_overall_history;

	if (overall_history->is_hit) {
		for (int i_index = 0; i_index < (int)wrapper->experiment_instance_histories.size(); i_index++) {
			BranchExperimentInstanceHistory* instance_history =
				(BranchExperimentInstanceHistory*)wrapper->experiment_instance_histories[i_index];

			double inner_targel_val;
			if (instance_history->signal_needed_from == NULL) {
				inner_targel_val = target_val;
			} else {
				inner_targel_val = calc_signal(instance_history->signal_needed_from,
											   wrapper);
			}

			this->new_scores.push_back(inner_targel_val);
		}

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			double existing_sum_score = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_score += this->existing_scores[h_index];
			}
			double existing_score = existing_sum_score / (double)this->existing_scores.size();

			double new_sum_score = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_score += this->new_scores[h_index];
			}
			double new_score = new_sum_score / (double)this->new_scores.size();

			// temp
			map<int, Signal*>::iterator it = wrapper->signals.find(this->scope_context->id);
			if (it != wrapper->signals.end()) {
				cout << "existing_score: " << existing_score << endl;
				cout << "new_score: " << new_score << endl;
				cout << endl;
			}

			#if defined(MDEBUG) && MDEBUG
			if (new_score <= existing_score && rand()%2 == 0) {
			#else
			if (new_score <= existing_score) {
			#endif /* MDEBUG */
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			}

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
			Solution* solution_copy = new Solution(wrapper->solution);
			/**
			 * - TODO: move scope_histories over
			 */

			add(wrapper);

			for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
				delete wrapper->solution->existing_scope_histories[h_index];
			}
			wrapper->solution->existing_scope_histories.clear();
			wrapper->solution->existing_target_val_histories.clear();

			wrapper->solution->existing_scope_histories = this->new_scope_histories;
			this->new_scope_histories.clear();
			wrapper->solution->existing_target_val_histories = this->new_target_val_histories;
			clean_scope(this->scope_context,
						wrapper);

			wrapper->solution->clean();

			wrapper->solution->measure_update();

			this->resulting_solution = wrapper->solution;
			wrapper->solutions.push_back(this->resulting_solution);
			wrapper->solution = solution_copy;

			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		}
	}
}
