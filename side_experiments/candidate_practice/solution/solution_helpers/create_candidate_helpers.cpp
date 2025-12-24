#include "solution_helpers.h"

#include "globals.h"
#include "network.h"
#include "tunnel.h"

using namespace std;

Tunnel* create_obs_candidate(vector<vector<double>>& obs_histories,
							 vector<double>& target_val_histories,
							 SolutionWrapper* wrapper) {
	geometric_distribution<int> num_obs_distribution(0.3);
	int num_obs;
	while (true) {
		num_obs = 1 + num_obs_distribution(generator);
		if (num_obs <= (int)obs_histories[0].size()) {
			break;
		}
	}

	vector<int> remaining_indexes(obs_histories[0].size());
	for (int i_index = 0; i_index < (int)obs_histories[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	vector<int> obs_indexes;
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		obs_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	vector<vector<double>> inputs(obs_histories.size());
	for (int h_index = 0; h_index < (int)obs_histories.size(); h_index++) {
		inputs[h_index] = vector<double>(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			inputs[h_index][o_index] = obs_histories[h_index][obs_indexes[o_index]];
		}
	}

	Network* signal_network = new Network(num_obs,
										  NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> sample_distribution(0, obs_histories.size()-1);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 30; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
	#endif /* MDEBUG */
		int index = sample_distribution(generator);

		signal_network->activate(inputs[index]);

		double error = target_val_histories[index] - signal_network->output->acti_vals[0];

		signal_network->backprop(error);
	}

	Tunnel* new_tunnel = new Tunnel(obs_indexes,
									false,
									NULL,
									signal_network,
									wrapper);

	return new_tunnel;
}

// Tunnel* create_pattern_candidate(vector<vector<double>>& obs_histories,
// 								 vector<double>& target_val_histories) {
// 	vector<double> selected_obs = obs_histories[0];
// 	double selected_target_val = target_val_histories[0];
// 	uniform_int_distribution<int> select_distribution(0, 1);
// 	for (int h_index = 1; h_index < (int)obs_histories.size(); h_index++) {
// 		if (target_val_histories[h_index] >= selected_target_val) {
// 			if (select_distribution(generator) == 0) {
// 				selected_obs = obs_histories[h_index];
// 				selected_target_val = target_val_histories[h_index];
// 			}
// 		}
// 	}

// 	geometric_distribution<int> num_obs_distribution(0.3);
// 	int num_obs;
// 	while (true) {
// 		num_obs = 1 + num_obs_distribution(generator);
// 		if (num_obs <= (int)obs_histories[0].size()) {
// 			break;
// 		}
// 	}

// 	vector<int> remaining_indexes(obs_histories[0].size());
// 	for (int i_index = 0; i_index < (int)obs_histories[0].size(); i_index++) {
// 		remaining_indexes[i_index] = i_index;
// 	}

// 	vector<int> obs_indexes;
// 	for (int o_index = 0; o_index < num_obs; o_index++) {
// 		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
// 		int index = distribution(generator);
// 		obs_indexes.push_back(remaining_indexes[index]);
// 		remaining_indexes.erase(remaining_indexes.begin() + index);
// 	}

// 	vector<vector<double>> inputs(obs_histories.size());


// 	/**
// 	 * - initial
// 	 */
// 	uniform_int_distribution<int> seed_distribution(0, 9);
// 	uniform_int_distribution<int> non_seed_positive_distribution(0, 9);
// 	for (int iter_index = 0; iter_index < 40000; iter_index++) {
// 		if (seed_distribution(generator) == 0) {

// 		} else {
// 			if () {

// 			}

// 		}
// 	}
// }
