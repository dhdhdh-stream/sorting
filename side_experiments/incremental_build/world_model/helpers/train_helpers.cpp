#include "world_model_helpers.h"

#include <algorithm>
#include <iostream>

#include "globals.h"
#include "network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_ITERS = 40;
#else
const int UPDATE_ITERS = 300000;
#endif /* MDEBUG */

const double VERIFICATION_RATIO = 0.2;
const double SEED_RATIO = 0.1;

double eval_helper(vector<vector<double>>& sample_obs,
				   vector<int>& sample_actions,
				   double sample_target_val,
				   int include_obs_index,
				   WorldModel* world_model,
				   Wrapper* wrapper) {
	vector<double> state(world_model->num_states, 0.0);

	for (int step_index = 0; step_index < (int)sample_obs.size(); step_index++) {
		if (step_index <= include_obs_index) {
			vector<double> starting_state = state;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->obs_network_inputs[n_index].size(); i_index++) {
					inputs.push_back(starting_state[world_model->obs_network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), sample_obs[step_index].begin(),
					sample_obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->obs_network_outputs[n_index].size(); o_index++) {
					state[world_model->obs_network_outputs[n_index][o_index]]
						+= world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}
		}

		if (step_index < (int)sample_actions.size()) {
			int action = sample_actions[step_index];

			vector<double> starting_state = state;

			vector<double> partial_inputs;
			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
				if (action == a_index) {
					partial_inputs.push_back(1.0);
				} else {
					partial_inputs.push_back(0.0);
				}
			}

			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->action_network_inputs[n_index].size(); i_index++) {
					inputs.push_back(starting_state[world_model->action_network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				world_model->action_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->action_network_outputs[n_index].size(); o_index++) {
					state[world_model->action_network_outputs[n_index][o_index]]
						+= world_model->action_networks[n_index]->output->acti_vals[o_index];
				}
			}
		}
	}

	world_model->score_network->activate(state);

	return world_model->score_network->output->acti_vals[0];
}

void train_helper(Wrapper* wrapper) {
	// temp
	cout << "train_helper" << endl;

	vector<vector<vector<double>>> train_obs;
	vector<vector<int>> train_actions;
	vector<double> train_target_vals;
	vector<vector<vector<double>>> verification_obs;
	vector<vector<int>> verification_actions;
	vector<double> verification_target_vals;

	int num_verification = VERIFICATION_RATIO * (double)wrapper->sample_obs.size();
	int num_train = (int)wrapper->sample_obs.size() - num_verification;
	{
		vector<int> remaining_indexes(wrapper->sample_obs.size());
		for (int i_index = 0; i_index < (int)wrapper->sample_obs.size(); i_index++) {
			remaining_indexes[i_index] = i_index;
		}

		for (int i_index = 0; i_index < num_train; i_index++) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			train_obs.push_back(wrapper->sample_obs[remaining_indexes[index]]);
			train_actions.push_back(wrapper->sample_actions[remaining_indexes[index]]);
			train_target_vals.push_back(wrapper->sample_target_vals[remaining_indexes[index]]);

			remaining_indexes.erase(remaining_indexes.begin() + index);
		}
		for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
			verification_obs.push_back(wrapper->sample_obs[remaining_indexes[i_index]]);
			verification_actions.push_back(wrapper->sample_actions[remaining_indexes[i_index]]);
			verification_target_vals.push_back(wrapper->sample_target_vals[remaining_indexes[i_index]]);
		}
	}

	WorldModel* potential_world_model = new WorldModel(wrapper->world_model);

	int num_seeds = SEED_RATIO * num_train;
	vector<int> remaining_indexes(num_train);
	vector<int> seed_indexes;
	for (int i_index = 0; i_index < num_train; i_index++) {
		remaining_indexes[i_index] = i_index;
	}
	for (int i_index = 0; i_index < num_seeds; i_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		seed_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	{
		vector<vector<vector<double>>> train_recognition_obs;
		vector<vector<int>> train_recognition_actions;
		vector<double> train_recognition_target_vals;
		for (int i_index = 0; i_index < (int)seed_indexes.size(); i_index++) {
			train_recognition_obs.push_back(train_obs[seed_indexes[i_index]]);
			train_recognition_actions.push_back(train_actions[seed_indexes[i_index]]);
			train_recognition_target_vals.push_back(1.0);
		}
		for (int i_index = 0; i_index < (int)remaining_indexes.size(); i_index++) {
			train_recognition_obs.push_back(train_obs[remaining_indexes[i_index]]);
			train_recognition_actions.push_back(train_actions[remaining_indexes[i_index]]);
			train_recognition_target_vals.push_back(-1.0);
		}

		temp_train_helper(train_recognition_obs,
						  train_recognition_actions,
						  train_recognition_target_vals,
						  potential_world_model,
						  wrapper);
	}

	{
		vector<vector<vector<double>>> train_seed_obs;
		vector<vector<int>> train_seed_actions;
		vector<double> train_seed_target_vals;
		for (int i_index = 0; i_index < (int)seed_indexes.size(); i_index++) {
			train_seed_obs.push_back(train_obs[seed_indexes[i_index]]);
			train_seed_actions.push_back(train_actions[seed_indexes[i_index]]);
			train_seed_target_vals.push_back(train_target_vals[seed_indexes[i_index]]);
		}

		temp_train_helper(train_seed_obs,
						  train_seed_actions,
						  train_seed_target_vals,
						  potential_world_model,
						  wrapper);
	}

	uniform_int_distribution<int> train_sample_distribution(0, train_obs.size()-1);
	for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
		int train_sample_index = train_sample_distribution(generator);

		update_world_model_helper(train_obs[train_sample_index],
								  train_actions[train_sample_index],
								  train_target_vals[train_sample_index],
								  potential_world_model,
								  wrapper);
	}

	vector<double> existing_misguess(verification_obs.size());
	vector<double> new_misguess(verification_obs.size());
	uniform_int_distribution<int> verification_sample_distribution(0, verification_obs.size()-1);
	for (int h_index = 0; h_index < (int)verification_obs.size(); h_index++) {
		vector<vector<double>> sample_obs = verification_obs[h_index];
		vector<int> sample_actions = verification_actions[h_index];
		double sample_target_val = verification_target_vals[h_index];

		uniform_int_distribution<int> include_obs_distribution(
			0, sample_obs.size()-1);
		// int include_obs_index = include_obs_distribution(generator);
		int include_obs_index = sample_obs.size()-1;

		double existing_predicted = eval_helper(sample_obs,
												sample_actions,
												sample_target_val,
												include_obs_index,
												wrapper->world_model,
												wrapper);
		existing_misguess[h_index] = (sample_target_val - existing_predicted)
			* (sample_target_val - existing_predicted);

		double new_predicted = eval_helper(sample_obs,
										   sample_actions,
										   sample_target_val,
										   include_obs_index,
										   potential_world_model,
										   wrapper);
		new_misguess[h_index] = (sample_target_val - new_predicted)
			* (sample_target_val - new_predicted);
	}

	double existing_sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)verification_obs.size(); h_index++) {
		existing_sum_misguess += existing_misguess[h_index];
	}
	double existing_misguess_average = existing_sum_misguess / (double)verification_obs.size();

	double existing_misguess_sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)verification_obs.size(); h_index++) {
		existing_misguess_sum_variance += (existing_misguess[h_index] - existing_misguess_average)
			* (existing_misguess[h_index] - existing_misguess_average);
	}
	double existing_misguess_variance = existing_misguess_sum_variance / (double)verification_obs.size();

	double new_sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)verification_obs.size(); h_index++) {
		new_sum_misguess += new_misguess[h_index];
	}
	double new_misguess_average = new_sum_misguess / (double)verification_obs.size();

	double new_misguess_sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)verification_obs.size(); h_index++) {
		new_misguess_sum_variance += (new_misguess[h_index] - new_misguess_average)
			* (new_misguess[h_index] - new_misguess_average);
	}
	double new_misguess_variance = new_misguess_sum_variance / (double)verification_obs.size();

	double denom = sqrt((existing_misguess_variance + new_misguess_variance) / (double)verification_obs.size());
	double t_score = (existing_misguess_average - new_misguess_average) / denom;

	// temp
	cout << "existing_misguess_average: " << existing_misguess_average << endl;
	cout << "existing_misguess_variance: " << existing_misguess_variance << endl;
	cout << "new_misguess_average: " << new_misguess_average << endl;
	cout << "new_misguess_variance: " << new_misguess_variance << endl;
	cout << "t_score: " << t_score << endl;

	#if defined(MDEBUG) && MDEBUG
	if (t_score >= 1.645 || rand()%2 == 0) {
	#else
	if (t_score >= 1.645) {
	#endif /* MDEBUG */
		cout << "add state" << endl;

		delete wrapper->world_model;
		wrapper->world_model = potential_world_model;
	} else {
		delete potential_world_model;
	}

	wrapper->sample_obs.clear();
	wrapper->sample_actions.clear();
	wrapper->sample_target_vals.clear();

	measure_test(wrapper);

	// temp
	cout << "train_helper done" << endl;
}
