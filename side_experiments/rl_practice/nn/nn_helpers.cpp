#include "nn_helpers.h"

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

using namespace std;

void train_rl_helper(vector<int>& actions,
					 vector<vector<double>>& obs_vals,
					 double target_val,
					 DecisionNetwork* decision_network,
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
	for (int s_index = (int)actions.size()-1; s_index >= 1; s_index--) {
		double eval_error = target_val - eval_histories[s_index];
		eval_network->backprop(eval_error,
							   state_errors,
							   eval_network_histories[s_index]);
		delete eval_network_histories[s_index];

		state_network->backprop(state_errors,
								state_network_histories[s_index]);
		delete state_network_histories[s_index];

		bool is_better;
		if (eval_histories[s_index] > eval_histories[s_index-1]) {
			is_better = true;
		} else {
			is_better = false;
		}

		decision_network->backprop(obs_vals[s_index],
								   actions[s_index],
								   state_histories[s_index-1],
								   is_better,
								   state_errors);
	}
}

void train_rl(DecisionNetwork* decision_network,
			  StateNetwork* state_network,
			  EvalNetwork* eval_network,
			  double& average_val) {
	for (int epoch_index = 0; epoch_index < NUM_EPOCHS; epoch_index++) {
		vector<vector<int>> actions;
		vector<vector<vector<double>>> obs_vals;
		vector<double> target_vals;
		for (int gather_index = 0; gather_index < NUM_SAMPLES; gather_index++) {
			vector<int> curr_actions;
			vector<vector<double>> curr_obs_vals;

			Problem* problem = problem_type->get_problem();

			bool exceeded_limit = false;

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
										   action);

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
					exceeded_limit = true;
					break;
				}
			}

			double target_val;
			if (exceeded_limit) {
				target_val = -1.0;
			} else {
				target_val = problem->score_result(iter_index);
			}

			actions.push_back(curr_actions);
			obs_vals.push_back(curr_obs_vals);
			target_vals.push_back(target_val);
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)target_vals.size(); h_index++) {
			sum_vals += target_vals[h_index];
		}
		average_val = sum_vals / (int)target_vals.size();

		uniform_int_distribution<int> sample_distribution(0, NUM_SAMPLES-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = sample_distribution(generator);
			train_rl_helper(actions[rand_index],
							obs_vals[rand_index],
							target_vals[rand_index],
							decision_network,
							state_network,
							eval_network,
							average_val);
		}
	}
}
