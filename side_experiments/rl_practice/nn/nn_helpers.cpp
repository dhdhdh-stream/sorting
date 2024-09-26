// for SimpleRLPractice, correctness based on previous obs
// - so would need to learn state
// - but can state be learned with a low number of train_iters and samples?

#include "nn_helpers.h"

#include <iostream>
#include <vector>

#include "constants.h"
#include "decision_network.h"
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
const int TRAIN_ITERS = 20000;
#endif /* MDEBUG */

const double STARTING_TEMPERATURE = 100.0;
const double ENDING_TEMPERATURE = 0.01;

using namespace std;

void train_eval_helper(vector<int>& actions,
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

void calc_eval_helper(vector<vector<int>>& actions,
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

			// temp
			if (gather_index%100 == 0) {
				cout << s_index << endl;
				cout << "obs_vals[gather_index][s_index][0]: " << obs_vals[gather_index][s_index][0] << endl;
				cout << "actions[gather_index][s_index]: " << actions[gather_index][s_index] << endl;
				cout << "evaluation: " << evaluation << endl;
			}
		}
	}
}

void train_decision_helper(vector<int>& actions,
						   vector<vector<double>>& obs_vals,
						   vector<vector<double>>& state_vals,
						   vector<double>& eval_vals,
						   DecisionNetwork* decision_network) {
	for (int s_index = 1; s_index < (int)actions.size(); s_index++) {
		decision_network->backprop(obs_vals[s_index],
								   actions[s_index],
								   state_vals[s_index],
								   eval_vals[s_index]);
	}
}

void train_rl(DecisionNetwork* decision_network,
			  StateNetwork* state_network,
			  EvalNetwork* eval_network,
			  double& average_val) {
	double temperature_diff = STARTING_TEMPERATURE / ENDING_TEMPERATURE;
	double temperature_exp = log(temperature_diff) / (NUM_EPOCHS-1);

	uniform_int_distribution<int> sample_distribution(0, NUM_SAMPLES-1);
	for (int epoch_index = 0; epoch_index < NUM_EPOCHS; epoch_index++) {
		double temperature = ENDING_TEMPERATURE * exp(temperature_exp * (NUM_EPOCHS-1 - epoch_index));

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
				decision_network->activate(obs,
										   state_vals,
										   temperature,
										   action);

				// // temp
				// if (gather_index%100 == 0) {
				// 	cout << iter_index << endl;
				// 	cout << "obs[0]: " << obs[0] << endl;
				// 	cout << "decision_network->output->acti_vals:" << endl;
				// 	for (int a_index = 0; a_index < (int)decision_network->output->acti_vals.size(); a_index++) {
				// 		cout << a_index << ": " << decision_network->output->acti_vals[a_index] << endl;
				// 	}
				// }

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

		// for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		for (int iter_index = 0; iter_index < 4 * TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			train_eval_helper(actions[rand_index],
							  obs_vals[rand_index],
							  target_vals[rand_index],
							  state_network,
							  eval_network,
							  average_val);
		}

		vector<vector<vector<double>>> state_vals(NUM_SAMPLES);
		vector<vector<double>> eval_vals(NUM_SAMPLES);
		calc_eval_helper(actions,
						 obs_vals,
						 state_vals,
						 eval_vals,
						 state_network,
						 eval_network);

		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			train_decision_helper(actions[rand_index],
								  obs_vals[rand_index],
								  state_vals[rand_index],
								  eval_vals[rand_index],
								  decision_network);
		}
	}
}
