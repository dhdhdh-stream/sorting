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
#include "tunnel.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */
const double VALIDATION_RATIO = 0.2;

#if defined(MDEBUG) && MDEBUG
const int MIN_TRAIN_NUM_DATAPOINTS = 4;
#else
const int MIN_TRAIN_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

void Experiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
		history->stack_traces.push_back(wrapper->scope_histories);

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

	vector<double> obs = wrapper->problem->get_observations();
	wrapper->scope_histories.back()->obs_history = obs;

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void split_above(std::vector<double>& vals,
				 double& split) {
	vector<double> above_vals;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		if (vals[h_index] >= 0.0) {
			above_vals.push_back(vals[h_index]);
		}
	}

	sort(above_vals.begin(), above_vals.end());

	double best_sum_variance = numeric_limits<double>::max();
	double best_zero_standard_deviation;
	double best_above_val_average;
	double best_above_standard_deviation;
	for (int i_index = 1; i_index < (int)above_vals.size(); i_index++) {
		double zero_sum_variance = 0.0;
		for (int zero_index = 0; zero_index < i_index; zero_index++) {
			zero_sum_variance += above_vals[zero_index] * above_vals[zero_index];
		}

		double above_sum_vals = 0.0;
		for (int above_index = i_index; above_index < (int)above_vals.size(); above_index++) {
			above_sum_vals += above_vals[above_index];
		}
		double above_val_average = above_sum_vals / ((double)above_vals.size() - i_index);

		double above_sum_variance = 0.0;
		for (int above_index = i_index; above_index < (int)above_vals.size(); above_index++) {
			above_sum_variance += (above_vals[above_index] - above_val_average) * (above_vals[above_index] - above_val_average);
		}

		double sum_variance = zero_sum_variance + above_sum_variance;

		if (sum_variance < best_sum_variance) {
			best_sum_variance = sum_variance;
			best_zero_standard_deviation = sqrt(zero_sum_variance / (double)i_index);
			best_above_val_average = above_val_average;
			best_above_standard_deviation = sqrt(above_sum_variance / ((double)above_vals.size() - i_index));
		}
	}

	split = best_above_val_average * best_zero_standard_deviation / (best_zero_standard_deviation + best_above_standard_deviation);
}

void split_below(std::vector<double>& vals,
				 double& split) {
	vector<double> below_vals;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		if (vals[h_index] <= 0.0) {
			below_vals.push_back(vals[h_index]);
		}
	}

	sort(below_vals.begin(), below_vals.end());

	double best_sum_variance = numeric_limits<double>::max();
	double best_below_val_average;
	double best_below_standard_deviation;
	double best_zero_standard_deviation;
	for (int i_index = 1; i_index < (int)below_vals.size(); i_index++) {
		double below_sum_vals = 0.0;
		for (int below_index = 0; below_index < i_index; below_index++) {
			below_sum_vals += below_vals[below_index];
		}
		double below_val_average = below_sum_vals / (double)i_index;

		double below_sum_variance = 0.0;
		for (int below_index = 0; below_index < i_index; below_index++) {
			below_sum_variance += (below_vals[below_index] - below_val_average) * (below_vals[below_index] - below_val_average);
		}

		double zero_sum_variance = 0.0;
		for (int zero_index = i_index; zero_index < (int)below_vals.size(); zero_index++) {
			zero_sum_variance += below_vals[zero_index] * below_vals[zero_index];
		}

		double sum_variance = below_sum_variance + zero_sum_variance;

		if (sum_variance < best_sum_variance) {
			best_sum_variance = sum_variance;
			best_below_val_average = below_val_average;
			best_below_standard_deviation = sqrt(below_sum_variance / (double)i_index);
			best_zero_standard_deviation = sqrt(zero_sum_variance / ((double)below_vals.size() - i_index));
		}
	}

	split = best_below_val_average * best_zero_standard_deviation / (best_below_standard_deviation + best_zero_standard_deviation);
}

void eval_tunnel(int tunnel_scope_context_id,
				 Tunnel* tunnel,
				 vector<vector<double>>& existing_obs_histories,
				 vector<vector<ScopeHistory*>>& existing_stack_traces,
				 vector<double>& existing_true_histories,
				 vector<vector<double>>& train_obs_histories,
				 vector<vector<ScopeHistory*>>& train_stack_traces,
				 vector<double>& train_true_histories,
				 vector<double>& train_true_network_vals,
				 double true_above_min,
				 double true_below_max,
				 vector<vector<double>>& validation_obs_histories,
				 vector<vector<ScopeHistory*>>& validation_stack_traces,
				 vector<double>& validation_true_histories,
				 vector<double>& validation_true_network_vals,
				 double true_score,
				 double& best_score,
				 vector<Network*>& networks,
				 vector<double>& above_min,
				 vector<double>& below_max) {
	vector<vector<double>> existing_signal_obs_histories;
	vector<double> existing_signal_target_vals;
	for (int h_index = 0; h_index < (int)existing_obs_histories.size(); h_index++) {
		for (int l_index = 0; l_index < (int)existing_stack_traces[h_index].size(); l_index++) {
			if (tunnel_scope_context_id == existing_stack_traces[h_index][l_index]->scope->id) {
				existing_signal_obs_histories.push_back(existing_obs_histories[h_index]);

				double signal = tunnel->get_signal(existing_stack_traces[h_index][l_index]->obs_history);
				existing_signal_target_vals.push_back(signal);

				break;
			}
		}
	}

	vector<vector<double>> new_signal_obs_histories;
	vector<double> new_signal_target_vals;
	for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
		for (int l_index = 0; l_index < (int)train_stack_traces[h_index].size(); l_index++) {
			if (tunnel_scope_context_id == train_stack_traces[h_index][l_index]->scope->id) {
				new_signal_obs_histories.push_back(train_obs_histories[h_index]);

				double signal = tunnel->get_signal(train_stack_traces[h_index][l_index]->obs_history);
				new_signal_target_vals.push_back(signal);

				break;
			}
		}
	}

	if (existing_signal_obs_histories.size() < MIN_TRAIN_NUM_DATAPOINTS
			|| new_signal_obs_histories.size() < MIN_TRAIN_NUM_DATAPOINTS) {
		return;
	}

	tunnel->latest_existing_obs.clear();
	for (int i_index = 0; i_index < 10; i_index++) {
		tunnel->latest_existing_obs.push_back(existing_signal_obs_histories[i_index]);
	}
	tunnel->latest_new_obs.clear();
	for (int i_index = 0; i_index < 10; i_index++) {
		tunnel->latest_new_obs.push_back(new_signal_obs_histories[i_index]);
	}

	tunnel->num_tries++;

	Network* tunnel_existing_network = new Network(existing_signal_obs_histories[0].size(),
												   NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> tunnel_existing_input_distribution(0, existing_signal_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = tunnel_existing_input_distribution(generator);

		tunnel_existing_network->activate(existing_signal_obs_histories[rand_index]);

		double error = existing_signal_target_vals[rand_index] - tunnel_existing_network->output->acti_vals[0];

		tunnel_existing_network->backprop(error);
	}

	for (int h_index = 0; h_index < (int)new_signal_obs_histories.size(); h_index++) {
		tunnel_existing_network->activate(new_signal_obs_histories[h_index]);
		new_signal_target_vals[h_index] -= tunnel_existing_network->output->acti_vals[0];
	}

	Network* tunnel_new_network = new Network(new_signal_obs_histories[0].size(),
											  NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> tunnel_new_input_distribution(0, new_signal_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = tunnel_new_input_distribution(generator);

		tunnel_new_network->activate(new_signal_obs_histories[rand_index]);

		double error = new_signal_target_vals[rand_index] - tunnel_new_network->output->acti_vals[0];

		tunnel_new_network->backprop(error);
	}

	/**
	 * - simply always use true_above_min and true_below_max
	 */

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
		if (validation_true_network_vals[h_index] >= 0.0) {
			if (validation_true_network_vals[h_index] >= true_above_min) {
				sum_vals += validation_true_histories[h_index];
			} else {
				tunnel_new_network->activate(validation_obs_histories[h_index]);
				if (tunnel_new_network->output->acti_vals[0] >= 0.0) {
					sum_vals += validation_true_histories[h_index];
				}
			}
		} else {
			if (validation_true_network_vals[h_index] > true_below_max) {
				tunnel_new_network->activate(validation_obs_histories[h_index]);
				if (tunnel_new_network->output->acti_vals[0] >= 0.0) {
					sum_vals += validation_true_histories[h_index];
				}
			}
		}
	}

	if (sum_vals > true_score) {
		tunnel->num_improve++;
	}

	#if defined(MDEBUG) && MDEBUG
	if (sum_vals > best_score || rand()%4 == 0) {
	#else
	if (sum_vals > best_score) {
	#endif /* MDEBUG */
		best_score = sum_vals;
		for (int n_index = 1; n_index < (int)networks.size(); n_index++) {
			delete networks[n_index];
		}
		networks = {networks[0], tunnel_new_network};
		above_min = {true_above_min, 0.0};
		below_max = {true_below_max, 0.0};
	} else {
		delete tunnel_new_network;
	}

	delete tunnel_existing_network;
}

void Experiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			vector<ScopeHistory*> stack_trace_copy(history->stack_traces[i_index].size());
			for (int l_index = 0; l_index < (int)history->stack_traces[i_index].size(); l_index++) {
				stack_trace_copy[l_index] = history->stack_traces[i_index][l_index]->copy_obs_history();
			}
			this->new_stack_traces.push_back(stack_trace_copy);

			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->new_true_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_stack_traces.begin(), this->new_stack_traces.end(), generator_copy);
			}
			{
				default_random_engine generator_copy = generator;
				shuffle(this->new_true_histories.begin(), this->new_true_histories.end(), generator_copy);
			}

			int num_train = (1.0 - VALIDATION_RATIO) * TRAIN_NEW_NUM_DATAPOINTS;

			vector<vector<double>> train_obs_histories(this->new_obs_histories.begin(), this->new_obs_histories.begin() + num_train);
			vector<vector<ScopeHistory*>> train_stack_traces(this->new_stack_traces.begin(), this->new_stack_traces.begin() + num_train);
			vector<double> train_true_histories(this->new_true_histories.begin(), this->new_true_histories.begin() + num_train);

			vector<vector<double>> validation_obs_histories(this->new_obs_histories.begin() + num_train, this->new_obs_histories.end());
			vector<vector<ScopeHistory*>> validation_stack_traces(this->new_stack_traces.begin() + num_train, this->new_stack_traces.end());
			vector<double> validation_true_histories(this->new_true_histories.begin() + num_train, this->new_true_histories.end());

			Network* new_true_network = new Network(train_obs_histories[0].size(),
													NETWORK_SIZE_SMALL);
			uniform_int_distribution<int> val_input_distribution(0, train_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				new_true_network->activate(train_obs_histories[rand_index]);

				double error = train_true_histories[rand_index] - new_true_network->output->acti_vals[0];

				new_true_network->backprop(error);
			}

			vector<double> train_true_network_vals(train_obs_histories.size());
			for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
				new_true_network->activate(train_obs_histories[h_index]);
				train_true_network_vals[h_index] = new_true_network->output->acti_vals[0];
			}

			double above_min;
			split_above(train_true_network_vals,
						above_min);

			double below_max;
			split_below(train_true_network_vals,
						below_max);

			vector<double> validation_true_network_vals(validation_obs_histories.size());
			for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
				new_true_network->activate(validation_obs_histories[h_index]);
				validation_true_network_vals[h_index] = new_true_network->output->acti_vals[0];
			}

			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)validation_obs_histories.size(); h_index++) {
				if (validation_true_network_vals[h_index] >= 0.0) {
					sum_vals += validation_true_histories[h_index];
				}
			}

			double best_score = sum_vals;
			this->networks = vector<Network*>{new_true_network};
			this->above_min = vector<double>{0.0};
			this->below_max = vector<double>{0.0};

			for (int c_index = 0; c_index < (int)wrapper->candidates.size(); c_index++) {
				eval_tunnel(wrapper->candidates[c_index].first,
							wrapper->candidates[c_index].second,
							this->existing_obs_histories,
							this->existing_stack_traces,
							this->existing_true_histories,
							train_obs_histories,
							train_stack_traces,
							train_true_histories,
							train_true_network_vals,
							above_min,
							below_max,
							validation_obs_histories,
							validation_stack_traces,
							validation_true_histories,
							validation_true_network_vals,
							sum_vals,
							best_score,
							this->networks,
							this->above_min,
							this->below_max);
			}

			#if defined(MDEBUG) && MDEBUG
			if (best_score > 0.0 || rand()%4 != 0) {
			#else
			if (best_score > 0.0) {
			#endif /* MDEBUG */
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
