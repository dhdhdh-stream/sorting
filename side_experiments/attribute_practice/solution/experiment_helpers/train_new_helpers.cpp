#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "build_network.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

const double MIN_POSITIVE_RATIO = 0.1;

void Experiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void Experiment::train_new_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

		if (this->signal_depth != -1) {
			history->stack_traces.push_back(wrapper->scope_histories);
		}

		this->new_obs_histories.push_back(obs);

		this->existing_network->activate(obs);
		history->existing_predicted.push_back(
			this->existing_network->output->acti_vals[0]);
	}

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
			ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[experiment_state->step_index];

			ScopeHistory* scope_history = wrapper->scope_histories.back();

			ScopeNodeHistory* history = new ScopeNodeHistory(scope_node);
			history->index = (int)scope_history->node_histories.size();
			scope_history->node_histories[scope_node->id] = history;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

/**
 * - noise can make it seem like there's a gradient when there isn't
 */
void binarize_with_leeway(vector<vector<double>>& train_obs_histories,
						  vector<double>& train_network_vals,
						  Network*& best_network) {
	vector<pair<double, int>> positive_samples;
	vector<pair<double, int>> negative_samples;
	for (int h_index = 0; h_index < (int)train_network_vals.size(); h_index++) {
		if (train_network_vals[h_index] >= 0.0) {
			positive_samples.push_back({train_network_vals[h_index], h_index});
		} else {
			negative_samples.push_back({train_network_vals[h_index], h_index});
		}
	}

	vector<vector<double>> binary_train_obs;
	vector<bool> binary_train_targets;

	sort(positive_samples.begin(), positive_samples.end());
	for (int h_index = (int)positive_samples.size() * 3/4; h_index < (int)positive_samples.size(); h_index++) {
		binary_train_obs.push_back(train_obs_histories[positive_samples[h_index].second]);
		binary_train_targets.push_back(true);
	}
	sort(negative_samples.begin(), negative_samples.end());
	for (int h_index = 0; h_index < (int)negative_samples.size() / 4; h_index++) {
		binary_train_obs.push_back(train_obs_histories[negative_samples[h_index].second]);
		binary_train_targets.push_back(false);
	}

	Network* binary_network = new Network(train_obs_histories[0].size());
	uniform_int_distribution<int> input_distribution(0, binary_train_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = input_distribution(generator);

		binary_network->activate(binary_train_obs[rand_index]);

		double error;
		if (binary_train_targets[rand_index]) {
			if (binary_network->output->acti_vals[0] > 1.0) {
				error = 0.0;
			} else {
				error = 1.0 - binary_network->output->acti_vals[0];
			}
		} else {
			if (binary_network->output->acti_vals[0] < -1.0) {
				error = 0.0;
			} else {
				error = -1.0 - binary_network->output->acti_vals[0];
			}
		}

		binary_network->backprop(error);
	}

	delete best_network;
	best_network = binary_network;
}

void Experiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		if (this->signal_depth == -1) {
			for (int i_index = 0; i_index < (int)history->existing_predicted.size(); i_index++) {
				this->new_target_vals.push_back(target_val - history->existing_predicted[i_index]);
			}
		} else {
			for (int i_index = 0; i_index < (int)history->existing_predicted.size(); i_index++) {
				ScopeHistory* scope_history;
				if (this->signal_depth >= (int)history->stack_traces[i_index].size()) {
					scope_history = history->stack_traces[i_index][0];
				} else {
					int index = history->stack_traces[i_index].size()-1 - this->signal_depth;
					scope_history = history->stack_traces[i_index][index];
				}

				Scope* scope = scope_history->scope;

				double pre_signal = scope->pre_signal->activate(scope_history->pre_obs_history);

				vector<double> input;
				input.insert(input.end(), scope_history->pre_obs_history.begin(), scope_history->pre_obs_history.end());
				input.insert(input.end(), scope_history->post_obs_history.begin(), scope_history->post_obs_history.end());

				double post_signal = scope->post_signal->activate(input);

				double new_signal = post_signal - pre_signal;

				this->new_target_vals.push_back(new_signal - history->existing_predicted[i_index]);
			}
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->new_target_vals.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			this->new_network = new Network(this->new_obs_histories[0].size());
			uniform_int_distribution<int> input_distribution(0, this->new_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = input_distribution(generator);

				this->new_network->activate(this->new_obs_histories[rand_index]);

				double error = this->new_target_vals[rand_index] - this->new_network->output->acti_vals[0];

				this->new_network->backprop(error);
			}

			vector<double> train_network_vals(this->new_obs_histories.size());
			int positive_count = 0;
			for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
				this->new_network->activate(this->new_obs_histories[h_index]);
				train_network_vals[h_index] = this->new_network->output->acti_vals[0];

				if (this->new_network->output->acti_vals[0] > 0.0) {
					positive_count++;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if ((positive_count > MIN_POSITIVE_RATIO * (double)this->new_obs_histories.size())
					|| rand()%4 != 0) {
			#else
			if (positive_count > MIN_POSITIVE_RATIO * (double)this->new_obs_histories.size()) {
			#endif /* MDEBUG */
				binarize_with_leeway(this->new_obs_histories,
									 train_network_vals,
									 this->new_network);

				this->new_branch_node = new BranchNode();
				this->new_branch_node->parent = this->scope_context;
				this->new_branch_node->id = this->scope_context->node_counter + (int)this->best_step_types.size();

				this->sum_true = 0.0;
				this->hit_count = 0;

				this->total_count = 0;
				this->total_sum_true = 0.0;

				this->state = EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
