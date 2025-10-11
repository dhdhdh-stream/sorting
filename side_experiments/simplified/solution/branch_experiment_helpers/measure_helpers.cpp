#include "branch_experiment.h"

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
#include "start_node.h"
#include "utilities.h"

using namespace std;

const int RETRAIN_NUM_CHANCES = 3;

void BranchExperiment::measure_check_activate(SolutionWrapper* wrapper,
											  BranchExperimentHistory* history) {
	double sum_vals = this->existing_constant;
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(wrapper->scope_histories.back(),
						   this->existing_inputs[i_index],
						   0,
						   val,
						   is_on);
		if (is_on) {
			double normalized_val = (val - this->existing_input_averages[i_index]) / this->existing_input_standard_deviations[i_index];
			sum_vals += this->existing_weights[i_index] * normalized_val;
		}
	}
	if (this->existing_network != NULL) {
		vector<double> input_vals(this->existing_network_inputs.size());
		vector<bool> input_is_on(this->existing_network_inputs.size());
		for (int i_index = 0; i_index < (int)this->existing_network_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(wrapper->scope_histories.back(),
							   this->existing_network_inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->existing_network->activate(input_vals,
									input_is_on);
		sum_vals += this->existing_network->output->acti_vals[0];
	}
	history->existing_predicted_scores.push_back(sum_vals);

	ScopeHistory* scope_history_copy = new ScopeHistory(wrapper->scope_histories.back());
	scope_history_copy->num_actions_snapshot = wrapper->num_actions;
	this->scope_histories.push_back(scope_history_copy);

	if (this->select_percentage == 1.0) {
		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		double sum_vals = this->new_constant;

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
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void BranchExperiment::measure_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::measure_backprop(double target_val,
										BranchExperimentHistory* history) {
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_scores.size(); i_index++) {
			this->target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
		}

		this->new_sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			double new_score = this->new_sum_scores / (double)this->state_iter;

			double existing_score;
			switch (this->node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)this->node_context;
					existing_score = start_node->average_score;
				}
				break;
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

			// temp
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "new_score: " << new_score << endl;
			cout << "existing_score: " << existing_score << endl;

			#if defined(MDEBUG) && MDEBUG
			if (true) {
			#else
			if (new_score - existing_score >= 0.0
					&& new_score > this->best_new_score) {
			#endif /* MDEBUG */
				this->best_new_score = new_score;

				this->best_constant = this->new_constant;
				this->best_inputs = this->new_inputs;
				this->best_input_averages = this->new_input_averages;
				this->best_input_standard_deviations = this->new_input_standard_deviations;
				this->best_weights = this->new_weights;
				this->best_network_inputs = this->new_network_inputs;
				if (this->best_network != NULL) {
					delete this->best_network;
				}
				this->best_network = this->new_network;
				this->new_network = NULL;

				this->best_select_percentage = this->select_percentage;
			}

			double target = max(0.0, this->best_new_score - existing_score);

			/**
			 * - if predicted good, but actual bad, then training samples must not be representative
			 *   - which may be corrected with additional samples from measure
			 *     - so retry until success or predicted becomes bad
			 */
			double constant;
			vector<Input> factor_inputs;
			vector<double> factor_input_averages;
			vector<double> factor_input_standard_deviations;
			vector<double> factor_weights;
			vector<Input> network_inputs;
			Network* network = NULL;
			double select_percentage;
			bool is_success = train_new_helper(this->scope_histories,
											   this->target_val_histories,
											   constant,
											   factor_inputs,
											   factor_input_averages,
											   factor_input_standard_deviations,
											   factor_weights,
											   network_inputs,
											   network,
											   select_percentage,
											   target);

			if (is_success && select_percentage > 0.0) {
				this->new_constant = constant;
				this->new_inputs = factor_inputs;
				this->new_input_averages = factor_input_averages;
				this->new_input_standard_deviations = factor_input_standard_deviations;
				this->new_weights = factor_weights;
				this->new_network_inputs = network_inputs;
				if (this->new_network != NULL) {
					delete this->new_network;
				}
				this->new_network = network;

				this->select_percentage = select_percentage;

				this->new_sum_scores = 0.0;

				this->state_iter = 0;
			} else {
				if (network != NULL) {
					delete network;
				}

				#if defined(MDEBUG) && MDEBUG
				if (this->best_new_score - existing_score >= 0.0 || rand()%2 == 0) {
				#else
				if (this->best_new_score - existing_score >= 0.0) {
				#endif /* MDEBUG */
					this->new_score = this->best_new_score;

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

					double improvement = this->new_score - existing_score;
					cout << "improvement: " << improvement << endl;

					cout << endl;

					#if defined(MDEBUG) && MDEBUG
					if (this->best_select_percentage == 1.0) {
						this->result = EXPERIMENT_RESULT_SUCCESS;
					} else {
						this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
						this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

						this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
						this->state_iter = 0;
					}
					#else
					this->result = EXPERIMENT_RESULT_SUCCESS;
					#endif /* MDEBUG */
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		}
	}
}
