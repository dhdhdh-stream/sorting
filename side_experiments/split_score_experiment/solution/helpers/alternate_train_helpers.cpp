#include "helpers.h"

#include <algorithm>
#include <iostream>

#include "globals.h"
#include "network.h"

using namespace std;

const double INITIAL_MATCH_RATIO = 0.1;

const int FORCE_ITERS = 20;
const int CONTINUE_ITERS = 10;
const int TRAIN_ITERS = 10000;

const double MIN_MATCH_RATIO = 0.05;

bool alternate_train_helper(vector<vector<double>>& vals,
							vector<double>& target_vals,
							vector<double>& existing_predicted_vals,
							Network* match_network,
							Network* score_network) {
	vector<vector<double>> match_vals;
	vector<double> match_target_vals;

	vector<int> remaining_indexes(vals.size());
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}
	int num_initial = INITIAL_MATCH_RATIO * (double)vals.size();
	for (int iter_index = 0; iter_index < num_initial; iter_index++) {
		uniform_int_distribution<int> possible_distribution(0, remaining_indexes.size()-1);
		int random_index = possible_distribution(generator);
		match_vals.push_back(vals[remaining_indexes[random_index]]);
		match_target_vals.push_back(target_vals[remaining_indexes[random_index]]);

		remaining_indexes.erase(remaining_indexes.begin() + random_index);
	}

	uniform_int_distribution<int> val_distribution(0, vals.size()-1);

	for (int epoch_iter = 0; epoch_iter < FORCE_ITERS; epoch_iter++) {
		uniform_int_distribution<int> match_distribution(0, match_vals.size()-1);
		for (int train_iter = 0; train_iter < TRAIN_ITERS; train_iter++) {
			int match_index = match_distribution(generator);

			score_network->activate(match_vals[match_index]);

			double error = match_target_vals[match_index] - score_network->output->acti_vals[0];

			score_network->backprop(error);
		}

		vector<double> new_predicted_vals(vals.size());
		vector<pair<double, int>> new_improvement(vals.size());
		vector<bool> new_is_better(vals.size());
		for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
			score_network->activate(vals[h_index]);
			new_predicted_vals[h_index] = score_network->output->acti_vals[0];

			double existing_misguess = abs(target_vals[h_index] - existing_predicted_vals[h_index]);
			double new_misguess = abs(target_vals[h_index] - new_predicted_vals[h_index]);

			new_improvement[h_index] = {new_misguess - existing_misguess, h_index};

			if (new_misguess < existing_misguess) {
				new_is_better[h_index] = true;
			} else {
				new_is_better[h_index] = false;
			}
		}
		sort(new_improvement.begin(), new_improvement.end());
		for (int i_index = 0; i_index < num_initial; i_index++) {
			int force_index = new_improvement[new_improvement.size() - 1 - num_initial].second;
			new_is_better[force_index] = true;
		}

		for (int train_iter = 0; train_iter < TRAIN_ITERS; train_iter++) {
			int val_index = val_distribution(generator);

			match_network->activate(vals[val_index]);

			double error;
			if (new_is_better[val_index]) {
				if (match_network->output->acti_vals[0] >= 1.0) {
					error = 0.0;
				} else {
					error = 1.0 - match_network->output->acti_vals[0];
				}
			} else {
				if (match_network->output->acti_vals[0] <= -1.0) {
					error = 0.0;
				} else {
					error = -1.0 - match_network->output->acti_vals[0];
				}
			}

			match_network->backprop(error);
		}

		match_vals.clear();
		match_target_vals.clear();
		for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
			if (new_is_better[h_index]) {
				match_vals.push_back(vals[h_index]);
				match_target_vals.push_back(target_vals[h_index]);
			}
		}
	}

	for (int epoch_iter = 0; epoch_iter < FORCE_ITERS; epoch_iter++) {
		uniform_int_distribution<int> match_distribution(0, match_vals.size()-1);
		for (int train_iter = 0; train_iter < TRAIN_ITERS; train_iter++) {
			int match_index = match_distribution(generator);

			score_network->activate(match_vals[match_index]);

			double error = match_target_vals[match_index] - score_network->output->acti_vals[0];

			score_network->backprop(error);
		}

		vector<double> new_predicted_vals(vals.size());
		for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
			score_network->activate(vals[h_index]);
			new_predicted_vals[h_index] = score_network->output->acti_vals[0];
		}
		vector<bool> new_is_better(vals.size());
		for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
			double existing_misguess = abs(target_vals[h_index] - existing_predicted_vals[h_index]);
			double new_misguess = abs(target_vals[h_index] - new_predicted_vals[h_index]);
			if (new_misguess < existing_misguess) {
				new_is_better[h_index] = true;
			} else {
				new_is_better[h_index] = false;
			}
		}

		for (int train_iter = 0; train_iter < CONTINUE_ITERS; train_iter++) {
			int val_index = val_distribution(generator);

			match_network->activate(vals[val_index]);

			double error;
			if (new_is_better[val_index]) {
				if (match_network->output->acti_vals[0] >= 1.0) {
					error = 0.0;
				} else {
					error = 1.0 - match_network->output->acti_vals[0];
				}
			} else {
				if (match_network->output->acti_vals[0] <= -1.0) {
					error = 0.0;
				} else {
					error = -1.0 - match_network->output->acti_vals[0];
				}
			}

			match_network->backprop(error);
		}

		match_vals.clear();
		match_target_vals.clear();
		for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
			match_network->activate(vals[h_index]);
			if (match_network->output->acti_vals[0] > 0.0) {
				match_vals.push_back(vals[h_index]);
				match_target_vals.push_back(target_vals[h_index]);
			}
		}

		if (match_vals.size() < MIN_MATCH_RATIO * (double)vals.size()) {
			return false;
		}
	}

	cout << "match_vals.size(): " << match_vals.size() << endl;

	return true;
}
