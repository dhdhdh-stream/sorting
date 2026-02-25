#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MIN_TRAIN_NUM_SAMPLES = 20;
#else
const int MIN_TRAIN_NUM_SAMPLES = 200;
#endif /* MDEBUG */

const double VALIDATION_RATIO = 0.2;

void Experiment::train_and_eval_helper(int layer,
									   double& best_val_average,
									   Network*& best_network,
									   int& best_layer,
									   bool& best_is_binarize) {
	int num_train_samples = (1.0 - VALIDATION_RATIO) * (int)this->new_obs_histories.size();

	vector<vector<double>> new_on_obs_histories;
	vector<double> new_on_target_vals;
	for (int h_index = 0; h_index < num_train_samples; h_index++) {
		if (layer < (int)this->new_target_vals[h_index].size()) {
			if (this->new_target_vals_is_on[h_index][layer]) {
				new_on_obs_histories.push_back(this->new_obs_histories[h_index]);
				new_on_target_vals.push_back(this->new_target_vals[h_index][layer]);
			}
		}
	}

	if (new_on_obs_histories.size() < MIN_TRAIN_NUM_SAMPLES) {
		return;
	}

	// for (int h_index = 0; h_index < (int)new_on_obs_histories.size(); h_index++) {
	// 	this->existing_networks[layer]->activate(new_on_obs_histories[h_index]);
	// 	new_on_target_vals[h_index] -= this->existing_networks[layer]->output->acti_vals[0];
	// }

	Network* curr_new_network = new Network(new_on_obs_histories[0].size());
	uniform_int_distribution<int> new_distribution(0, new_on_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = new_distribution(generator);

		curr_new_network->activate(new_on_obs_histories[index]);

		double error = new_on_target_vals[index] - curr_new_network->output->acti_vals[0];

		curr_new_network->backprop(error);
	}

	vector<pair<double, int>> positive_samples;
	vector<pair<double, int>> negative_samples;
	for (int h_index = 0; h_index < (int)new_on_obs_histories.size(); h_index++) {
		curr_new_network->activate(new_on_obs_histories[h_index]);
		if (curr_new_network->output->acti_vals[0] >= 0.0) {
			positive_samples.push_back({curr_new_network->output->acti_vals[0], h_index});
		} else {
			negative_samples.push_back({curr_new_network->output->acti_vals[0], h_index});
		}
	}

	if (positive_samples.size() == 0) {
		delete curr_new_network;
		return;
	}

	{
		double sum_scores = 0.0;
		int count = 0;
		for (int h_index = num_train_samples; h_index < (int)this->new_obs_histories.size(); h_index++) {
			curr_new_network->activate(this->new_obs_histories[h_index]);
			if (curr_new_network->output->acti_vals[0] >= 0.0) {
				sum_scores += this->new_target_vals[h_index][0];
				count++;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if ((count > 0 && sum_scores / count > best_val_average)
				|| true) {
		#else
		if (count > 0 && sum_scores / count > best_val_average) {
		#endif /* MDEBUG */
			best_val_average = sum_scores / count;

			if (best_network != NULL) {
				delete best_network;
			}
			best_network = curr_new_network;
			best_layer = layer;
			best_is_binarize = false;
		} else {
			delete curr_new_network;
		}
	}

	/**
	 * - noise can make it seem like there's a gradient when there isn't
	 */
	vector<vector<double>> binary_train_obs;
	vector<bool> binary_train_targets;

	sort(positive_samples.begin(), positive_samples.end());
	for (int h_index = (int)positive_samples.size() * 3/4; h_index < (int)positive_samples.size(); h_index++) {
		binary_train_obs.push_back(new_on_obs_histories[positive_samples[h_index].second]);
		binary_train_targets.push_back(true);
	}
	sort(negative_samples.begin(), negative_samples.end());
	for (int h_index = 0; h_index < (int)negative_samples.size() / 4; h_index++) {
		binary_train_obs.push_back(new_on_obs_histories[negative_samples[h_index].second]);
		binary_train_targets.push_back(false);
	}

	Network* binary_network = new Network(binary_train_obs[0].size());
	uniform_int_distribution<int> input_distribution(0, binary_train_obs.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = input_distribution(generator);

		binary_network->activate(binary_train_obs[index]);

		double error;
		if (binary_train_targets[index]) {
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

	double sum_scores = 0.0;
	int count = 0;
	for (int h_index = num_train_samples; h_index < (int)this->new_obs_histories.size(); h_index++) {
		binary_network->activate(this->new_obs_histories[h_index]);
		if (binary_network->output->acti_vals[0] >= 0.0) {
			sum_scores += this->new_target_vals[h_index][0];
			count++;
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if ((count > 0 && sum_scores / count > best_val_average)
			|| true) {
	#else
	if (count > 0 && sum_scores / count > best_val_average) {
	#endif /* MDEBUG */
		best_val_average = sum_scores / count;

		if (best_network != NULL) {
			delete best_network;
		}
		best_network = binary_network;
		best_layer = layer;
		best_is_binarize = true;
	} else {
		delete binary_network;
	}
}
