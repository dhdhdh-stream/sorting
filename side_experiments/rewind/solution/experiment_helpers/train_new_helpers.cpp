#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
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
const double VALIDATION_RATIO = 0.2;

// const double MIN_POSITIVE_RATIO = 0.05;
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

		this->new_obs_histories.push_back(obs);

		this->existing_true_network->activate(obs);
		history->existing_predicted_trues.push_back(
			this->existing_true_network->output->acti_vals[0]);
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

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
void binarize(vector<vector<double>>& train_obs_histories,
			  vector<double>& train_true_histories,
			  vector<double>& train_true_network_vals,
			  vector<vector<double>>& validation_obs_histories,
			  vector<double>& validation_true_histories,
			  double& best_sum_vals,
			  Network*& best_network,
			  bool& is_binarize) {
	Network* binary_network = new Network(train_obs_histories[0].size(),
										  NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> input_distribution(0, train_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = input_distribution(generator);

		binary_network->activate(train_obs_histories[rand_index]);

		double error;
		if (train_true_network_vals[rand_index] > 0.0) {
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

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
		binary_network->activate(validation_obs_histories[h_index]);
		if (binary_network->output->acti_vals[0] >= 0.0) {
			sum_vals += validation_true_histories[h_index];
		}
	}

	if (sum_vals > best_sum_vals) {
		best_sum_vals = sum_vals;
		delete best_network;
		best_network = binary_network;

		is_binarize = true;
	} else {
		delete binary_network;

		is_binarize = false;
	}
}

void binarize_with_leeway(vector<vector<double>>& train_obs_histories,
						  vector<double>& train_true_histories,
						  vector<double>& train_true_network_vals,
						  vector<vector<double>>& validation_obs_histories,
						  vector<double>& validation_true_histories,
						  double& best_sum_vals,
						  Network*& best_network,
						  bool& is_binarize) {
	vector<pair<double, int>> positive_samples;
	vector<pair<double, int>> negative_samples;
	for (int h_index = 0; h_index < (int)train_true_network_vals.size(); h_index++) {
		if (train_true_network_vals[h_index] >= 0.0) {
			positive_samples.push_back({train_true_network_vals[h_index], h_index});
		} else {
			negative_samples.push_back({train_true_network_vals[h_index], h_index});
		}
	}

	vector<vector<double>> binary_train_obs;
	vector<bool> binary_train_targets;

	sort(positive_samples.begin(), positive_samples.end());
	// for (int h_index = (int)positive_samples.size() / 2; h_index < (int)positive_samples.size(); h_index++) {
	for (int h_index = (int)positive_samples.size() * 3/4; h_index < (int)positive_samples.size(); h_index++) {
		binary_train_obs.push_back(train_obs_histories[positive_samples[h_index].second]);
		binary_train_targets.push_back(true);
	}
	sort(negative_samples.begin(), negative_samples.end());
	for (int h_index = 0; h_index < (int)negative_samples.size() / 4; h_index++) {
		binary_train_obs.push_back(train_obs_histories[negative_samples[h_index].second]);
		binary_train_targets.push_back(false);
	}

	Network* binary_network = new Network(train_obs_histories[0].size(),
										  NETWORK_SIZE_SMALL);
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

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
		binary_network->activate(validation_obs_histories[h_index]);
		if (binary_network->output->acti_vals[0] >= 0.0) {
			sum_vals += validation_true_histories[h_index];
		}
	}

	// if (sum_vals > best_sum_vals) {
	if (true) {
		best_sum_vals = sum_vals;
		delete best_network;
		best_network = binary_network;

		is_binarize = true;
	} else {
		delete binary_network;

		is_binarize = false;
	}
}

void Experiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		// double existing_result = get_existing_result(wrapper);
		// for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
		// 	this->new_true_histories.push_back(target_val - existing_result);
		// }

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->new_true_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_true_histories.begin(), this->new_true_histories.end(), generator_copy);
			}

			int num_train = (1.0 - VALIDATION_RATIO) * TRAIN_NEW_NUM_DATAPOINTS;

			vector<vector<double>> train_obs_histories(this->new_obs_histories.begin(), this->new_obs_histories.begin() + num_train);
			vector<double> train_true_histories(this->new_true_histories.begin(), this->new_true_histories.begin() + num_train);

			vector<vector<double>> validation_obs_histories(this->new_obs_histories.begin() + num_train, this->new_obs_histories.end());
			vector<double> validation_true_histories(this->new_true_histories.begin() + num_train, this->new_true_histories.end());

			this->new_true_network = new Network(train_obs_histories[0].size(),
												 NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> input_distribution(0, train_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = input_distribution(generator);

				this->new_true_network->activate(train_obs_histories[rand_index]);

				double error = train_true_histories[rand_index] - this->new_true_network->output->acti_vals[0];

				this->new_true_network->backprop(error);
			}

			vector<double> train_true_network_vals(train_obs_histories.size());
			int positive_count = 0;
			for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
				this->new_true_network->activate(train_obs_histories[h_index]);
				train_true_network_vals[h_index] = this->new_true_network->output->acti_vals[0];

				if (this->new_true_network->output->acti_vals[0] > 0.0) {
					positive_count++;
				}
			}

			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
				this->new_true_network->activate(validation_obs_histories[h_index]);
				if (this->new_true_network->output->acti_vals[0] >= 0.0) {
					sum_vals += validation_true_histories[h_index];
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if ((positive_count > MIN_POSITIVE_RATIO * (double)train_obs_histories.size() && sum_vals >= 0.0)
					|| rand()%4 != 0) {
			#else
			if (positive_count > MIN_POSITIVE_RATIO * (double)train_obs_histories.size() && sum_vals >= 0.0) {
			#endif /* MDEBUG */
				// binarize(train_obs_histories,
				// 		 train_true_histories,
				// 		 train_true_network_vals,
				// 		 validation_obs_histories,
				// 		 validation_true_histories,
				// 		 sum_vals,
				// 		 this->new_true_network,
				// 		 this->is_binarize);
				binarize_with_leeway(train_obs_histories,
									 train_true_histories,
									 train_true_network_vals,
									 validation_obs_histories,
									 validation_true_histories,
									 sum_vals,
									 this->new_true_network,
									 this->is_binarize);
				// this->is_binarize = false;

				this->sum_true = 0.0;
				this->hit_count = 0;

				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->state = EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
