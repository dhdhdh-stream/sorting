#include "nn_helpers.h"

#include <iostream>
#include <vector>

#include "combined_decision_network.h"
#include "constants.h"
#include "eval_network.h"
#include "globals.h"
#include "problem.h"
#include "state_network.h"

#if defined(MDEBUG) && MDEBUG
const int NUM_EPOCHS = 2;
const int NUM_SAMPLES = 10;
const int TRAIN_ITERS = 200;
#else
const int NUM_EPOCHS = 20;
const int NUM_SAMPLES = 1000;
const int TRAIN_ITERS = 100000;
#endif /* MDEBUG */

using namespace std;

void train_eval_combined_helper(vector<int>& actions,
								vector<vector<double>>& obs_vals,
								double target_val,
								StateNetwork* state_network,
								EvalNetwork* eval_network,
								double average_val) {
	vector<double> state_vals(RL_STATE_SIZE, 0.0);
	double current_eval = average_val;

	vector<StateNetworkHistory*> state_network_histories;
	vector<EvalNetworkHistory*> eval_network_histories;
	vector<vector<double>> state_histories;
	vector<double> eval_histories;

	for (int s_index = 0; s_index < (int)actions.size(); s_index++) {
		StateNetworkHistory* state_network_history = new StateNetworkHistory();
		state_network->activate(obs_vals[s_index],
								actions[s_index],
								state_vals,
								state_network_history);

		EvalNetworkHistory* eval_network_history = new EvalNetworkHistory();
		double evaluation;
		eval_network->activate(obs_vals[s_index],
							   actions[s_index],
							   state_vals,
							   eval_network_history,
							   evaluation);

		current_eval += evaluation;

		state_network_histories.push_back(state_network_history);
		eval_network_histories.push_back(eval_network_history);
		state_histories.push_back(state_vals);
		eval_histories.push_back(current_eval);
	}

	vector<double> state_errors(RL_STATE_SIZE, 0.0);
	for (int s_index = (int)actions.size()-1; s_index >= 0; s_index--) {
		double eval_error = target_val - eval_histories[s_index];
		eval_network->backprop(eval_error,
							   state_errors,
							   eval_network_histories[s_index]);
		delete eval_network_histories[s_index];

		state_network->backprop(state_errors,
								state_network_histories[s_index]);
		delete state_network_histories[s_index];
	}
}

void calc_eval_combined_helper(vector<vector<int>>& actions,
							   vector<vector<vector<double>>>& obs_vals,
							   vector<vector<vector<double>>>& state_vals,
							   vector<vector<double>>& eval_vals,
							   StateNetwork* state_network,
							   EvalNetwork* eval_network) {
	for (int gather_index = 0; gather_index < NUM_SAMPLES; gather_index++) {
		state_vals[gather_index] = vector<vector<double>>(actions[gather_index].size());
		eval_vals[gather_index] = vector<double>(actions[gather_index].size());

		vector<double> curr_state_vals(RL_STATE_SIZE, 0.0);

		for (int s_index = 0; s_index < (int)actions[gather_index].size(); s_index++) {
			state_network->activate(obs_vals[gather_index][s_index],
									actions[gather_index][s_index],
									curr_state_vals);

			double evaluation;
			eval_network->activate(obs_vals[gather_index][s_index],
								   actions[gather_index][s_index],
								   curr_state_vals,
								   evaluation);

			state_vals[gather_index][s_index] = curr_state_vals;
			eval_vals[gather_index][s_index] = evaluation;
		}
	}
}

void train_decision_combined_helper(vector<int>& actions,
									vector<vector<double>>& obs_vals,
									vector<vector<double>>& state_vals,
									vector<double>& eval_vals,
									CombinedDecisionNetwork* decision_network) {
	for (int s_index = 1; s_index < (int)actions.size(); s_index++) {
		decision_network->backprop(
			obs_vals[s_index-1],
			actions[s_index],
			state_vals[s_index-1],
			eval_vals[s_index]);
	}
}

void train_rl_combined(CombinedDecisionNetwork* decision_network,
					   StateNetwork* state_network,
					   EvalNetwork* eval_network,
					   double& average_val) {
	/**
	 * - occasional random is better than temperature
	 *   - explore should be evaluated given other actions are correct
	 *     - vs. the random to random from temperature
	 */
	uniform_int_distribution<int> random_distribution(0, 9);
	uniform_int_distribution<int> random_action_distribution(-1, problem_type->num_possible_actions()-1);
	uniform_int_distribution<int> sample_distribution(0, NUM_SAMPLES-1);
	for (int epoch_index = 0; epoch_index < NUM_EPOCHS; epoch_index++) {
		vector<vector<int>> actions;
		vector<vector<vector<double>>> obs_vals;
		vector<double> target_vals;
		for (int gather_index = 0; gather_index < NUM_SAMPLES; gather_index++) {
			vector<int> curr_actions;
			vector<vector<double>> curr_obs_vals;

			Problem* problem = problem_type->get_problem();

			vector<double> state_vals(RL_STATE_SIZE, 0.0);

			vector<double> obs;
			vector<vector<int>> locations;

			{
				problem->get_observations(obs,
										  locations);

				curr_actions.push_back(ACTION_NOOP);
				curr_obs_vals.push_back(obs);

				state_network->activate(obs,
										ACTION_NOOP,
										state_vals);
			}

			int iter_index = 0;
			while (true) {
				int action;
				if (epoch_index == 0
						|| random_distribution(generator) == 0) {
					action = random_action_distribution(generator);
				} else {
					decision_network->activate(
						obs,
						state_vals,
						action);
				}

				if (action == ACTION_TERMINATE) {
					curr_actions.push_back(action);
					/**
					 * - simply reuse previous obs
					 */
					curr_obs_vals.push_back(obs);
					break;
				}

				problem->perform_action(Action(action));
				problem->get_observations(obs,
										  locations);

				curr_actions.push_back(action);
				curr_obs_vals.push_back(obs);

				state_network->activate(obs,
										action,
										state_vals);

				iter_index++;
				if (iter_index >= RL_MAX_ITERS) {
					break;
				}
			}

			double target_val = problem->score_result(iter_index);

			actions.push_back(curr_actions);
			obs_vals.push_back(curr_obs_vals);
			target_vals.push_back(target_val);

			delete problem;
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)target_vals.size(); h_index++) {
			sum_vals += target_vals[h_index];
		}
		average_val = sum_vals / (int)target_vals.size();
		cout << "average_val: " << average_val << endl;

		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			train_eval_combined_helper(actions[rand_index],
									   obs_vals[rand_index],
									   target_vals[rand_index],
									   state_network,
									   eval_network,
									   average_val);
		}

		vector<vector<vector<double>>> state_vals(NUM_SAMPLES);
		vector<vector<double>> eval_vals(NUM_SAMPLES);
		calc_eval_combined_helper(actions,
								  obs_vals,
								  state_vals,
								  eval_vals,
								  state_network,
								  eval_network);

		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			train_decision_combined_helper(actions[rand_index],
										   obs_vals[rand_index],
										   state_vals[rand_index],
										   eval_vals[rand_index],
										   decision_network);
		}
	}
}
