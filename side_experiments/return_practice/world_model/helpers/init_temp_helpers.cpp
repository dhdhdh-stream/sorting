#include "world_model_helpers.h"

#include <iostream>

#include "globals.h"
#include "network.h"

using namespace std;

const int NUM_TEMP_NETWORKS = 2;

#if defined(MDEBUG) && MDEBUG
const int INIT_TEMP_TRAIN_ITERS = 30;
#else
const int INIT_TEMP_TRAIN_ITERS = 300000;
#endif /* MDEBUG */

void init_temp_helper(vector<vector<double>>& init_obs_inputs,
					  vector<vector<double>>& init_action_inputs,
					  vector<double>& init_target_vals,
					  vector<Network*>& temp_obs_networks,
					  vector<double>& temp_obs_network_means,
					  vector<double>& temp_obs_network_diffs,
					  vector<Network*>& temp_action_networks,
					  vector<double>& temp_action_network_means,
					  vector<double>& temp_action_network_diffs) {
	// temp
	cout << "init_temp_helper" << endl;

	uniform_int_distribution<int> sample_distribution(0, init_obs_inputs.size()-1);
	for (int n_index = 0; n_index < NUM_TEMP_NETWORKS; n_index++) {
		Network* obs_network = new Network(init_obs_inputs[0].size(), 1);
		for (int iter_index = 0; iter_index < INIT_TEMP_TRAIN_ITERS; iter_index++) {
			int index = sample_distribution(generator);
			obs_network->activate(init_obs_inputs[index]);

			vector<double> errors{init_target_vals[index]/2.0 - obs_network->output->acti_vals[0]};
			obs_network->backprop(errors);

			if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
				obs_network->update();
			}
		}
		temp_obs_networks.push_back(obs_network);

		Network* action_network = new Network(init_action_inputs[0].size(), 1);
		for (int iter_index = 0; iter_index < INIT_TEMP_TRAIN_ITERS; iter_index++) {
			int index = sample_distribution(generator);
			action_network->activate(init_action_inputs[index]);

			vector<double> errors{init_target_vals[index]/2.0 - action_network->output->acti_vals[0]};
			action_network->backprop(errors);

			if ((iter_index + 1)%NETWORK_EPOCH_SIZE == 0) {
				action_network->update();
			}
		}
		temp_action_networks.push_back(action_network);

		vector<double> temp_obs_vals(init_obs_inputs.size());
		for (int h_index = 0; h_index < (int)init_obs_inputs.size(); h_index++) {
			obs_network->activate(init_obs_inputs[h_index]);
			temp_obs_vals[h_index] = obs_network->output->acti_vals[0];
		}

		vector<double> temp_action_vals(init_obs_inputs.size());
		for (int h_index = 0; h_index < (int)init_obs_inputs.size(); h_index++) {
			action_network->activate(init_action_inputs[h_index]);
			temp_action_vals[h_index] = action_network->output->acti_vals[0];
		}

		double sum_obs_vals = 0.0;
		for (int h_index = 0; h_index < (int)init_obs_inputs.size(); h_index++) {
			sum_obs_vals += temp_obs_vals[h_index];
		}
		double obs_val_average = sum_obs_vals / (double)init_obs_inputs.size();
		temp_obs_network_means.push_back(obs_val_average);

		double sum_obs_diffs = 0.0;
		for (int h_index = 0; h_index < (int)init_obs_inputs.size(); h_index++) {
			sum_obs_diffs += abs(obs_val_average - temp_obs_vals[h_index]);
		}
		double obs_val_diff = sum_obs_diffs / (double)init_obs_inputs.size();
		temp_obs_network_diffs.push_back(obs_val_diff);

		double sum_action_vals = 0.0;
		for (int h_index = 0; h_index < (int)init_action_inputs.size(); h_index++) {
			sum_action_vals += temp_action_vals[h_index];
		}
		double action_val_average = sum_action_vals / (double)init_action_inputs.size();
		temp_action_network_means.push_back(action_val_average);

		double sum_action_diffs = 0.0;
		for (int h_index = 0; h_index < (int)init_action_inputs.size(); h_index++) {
			sum_action_diffs += abs(action_val_average - temp_action_vals[h_index]);
		}
		double action_val_diff = sum_action_diffs / (double)init_action_inputs.size();
		temp_action_network_diffs.push_back(action_val_diff);

		for (int h_index = 0; h_index < (int)init_obs_inputs.size(); h_index++) {
			init_target_vals[h_index] -= temp_obs_vals[h_index];
			init_target_vals[h_index] -= temp_action_vals[h_index];
		}
	}

	// // temp
	// for (int h_index = 0; h_index < 20; h_index++) {
	// 	cout << h_index << endl;

	// 	temp_obs_networks[0]->activate(init_obs_inputs[h_index]);
	// 	cout << "temp_obs_networks[0]->output->acti_vals[0]: " << temp_obs_networks[0]->output->acti_vals[0] << endl;

	// 	temp_obs_networks[1]->activate(init_obs_inputs[h_index]);
	// 	cout << "temp_obs_networks[1]->output->acti_vals[0]: " << temp_obs_networks[1]->output->acti_vals[0] << endl;

	// 	temp_action_networks[0]->activate(init_action_inputs[h_index]);
	// 	cout << "temp_action_networks[0]->output->acti_vals[0]: " << temp_action_networks[0]->output->acti_vals[0] << endl;

	// 	temp_action_networks[1]->activate(init_action_inputs[h_index]);
	// 	cout << "temp_action_networks[1]->output->acti_vals[0]: " << temp_action_networks[1]->output->acti_vals[0] << endl;

	// 	cout << "init_target_vals[h_index]: " << init_target_vals[h_index] << endl;

	// 	cout << endl;
	// }

	// // temp
	// cout << "temp_obs_network_means[0]: " << temp_obs_network_means[0] << endl;
	// cout << "temp_obs_network_means[1]: " << temp_obs_network_means[1] << endl;
	// cout << "temp_obs_network_diffs[0]: " << temp_obs_network_diffs[0] << endl;
	// cout << "temp_obs_network_diffs[1]: " << temp_obs_network_diffs[1] << endl;
	// cout << "temp_action_network_means[0]: " << temp_action_network_means[0] << endl;
	// cout << "temp_action_network_means[1]: " << temp_action_network_means[1] << endl;
	// cout << "temp_action_network_diffs[0]: " << temp_action_network_diffs[0] << endl;
	// cout << "temp_action_network_diffs[1]: " << temp_action_network_diffs[1] << endl;
}
