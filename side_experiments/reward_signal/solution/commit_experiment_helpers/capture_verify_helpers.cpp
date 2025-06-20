#if defined(MDEBUG) && MDEBUG

#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void CommitExperiment::capture_verify_check_activate(
		SolutionWrapper* wrapper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = wrapper->problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = wrapper->starting_run_seed;

	CommitExperimentState* new_experiment_state = new CommitExperimentState(this);
	new_experiment_state->is_save = false;
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void CommitExperiment::capture_verify_step(vector<double>& obs,
										   int& action,
										   bool& is_next,
										   SolutionWrapper* wrapper,
										   CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (experiment_state->step_index >= (int)this->save_step_types.size()) {
			wrapper->node_context.back() = this->save_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
		} else {
			if (this->save_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
				action = this->save_actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[experiment_state->step_index]);
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(this->save_scopes[experiment_state->step_index]->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
				wrapper->confusion_context.push_back(NULL);

				if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
					this->save_scopes[experiment_state->step_index]->new_scope_experiment->pre_activate(wrapper);
				}
			}
		}
	} else {
		if (experiment_state->step_index == this->step_iter) {
			double sum_vals = this->commit_new_average_score;
			for (int i_index = 0; i_index < (int)this->commit_new_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(wrapper->scope_histories.back(),
								   this->commit_new_inputs[i_index],
								   0,
								   val,
								   is_on);
				if (is_on) {
					double normalized_val = (val - this->commit_new_input_averages[i_index]) / this->commit_new_input_standard_deviations[i_index];
					sum_vals += this->commit_new_weights[i_index] * normalized_val;
				}
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

			if (decision_is_branch) {
				experiment_state->is_save = true;
				experiment_state->step_index = 0;
				return;
			}
		}

		if (experiment_state->step_index >= (int)this->new_nodes.size()) {
			wrapper->node_context.back() = this->best_exit_next_node;

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
		} else {
			switch (this->new_nodes[experiment_state->step_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

					action = node->action;
					is_next = true;

					wrapper->num_actions++;

					experiment_state->step_index++;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

					ScopeNodeHistory* history = new ScopeNodeHistory(node);
					history->index = (int)scope_history->node_histories.size();
					scope_history->node_histories[node->id] = history;

					ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
					history->scope_history = inner_scope_history;
					wrapper->scope_histories.push_back(inner_scope_history);
					wrapper->node_context.push_back(node->scope->nodes[0]);
					wrapper->experiment_context.push_back(NULL);
					wrapper->confusion_context.push_back(NULL);

					if (node->scope->new_scope_experiment != NULL) {
						node->scope->new_scope_experiment->pre_activate(wrapper);
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

					ScopeHistory* scope_history = wrapper->scope_histories.back();

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
}

void CommitExperiment::capture_verify_exit_step(SolutionWrapper* wrapper,
												CommitExperimentState* experiment_state) {
	if (experiment_state->is_save) {
		if (this->save_scopes[experiment_state->step_index]->new_scope_experiment != NULL) {
			this->save_scopes[experiment_state->step_index]->new_scope_experiment->back_activate(wrapper);
		}

		delete wrapper->scope_histories.back();

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	} else {
		ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

		if (node->scope->new_scope_experiment != NULL) {
			node->scope->new_scope_experiment->back_activate(wrapper);
		}

		wrapper->scope_histories.pop_back();
		wrapper->node_context.pop_back();
		wrapper->experiment_context.pop_back();
		wrapper->confusion_context.pop_back();

		experiment_state->step_index++;
	}
}

void CommitExperiment::capture_verify_backprop() {
	if (this->verify_problems[this->state_iter] != NULL) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */