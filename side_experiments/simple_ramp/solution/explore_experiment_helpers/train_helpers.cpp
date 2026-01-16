#include "explore_experiment.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

bool train_existing_helper(ObsNode* node_context,
						   SolutionWrapper* wrapper,
						   Network*& network) {
	vector<vector<double>> obs_histories;
	vector<double> target_val_histories;
	int num_hits = 0;
	for (list<pair<ScopeHistory*,double>>::iterator it = wrapper->solution->existing_scope_histories.begin();
			it != wrapper->solution->existing_scope_histories.end(); it++) {
		int start_size = (int)obs_histories.size();

		fetch_histories_helper(it->first,
							   node_context,
							   obs_histories);

		if ((int)obs_histories.size() > start_size) {
			num_hits++;

			while (target_val_histories.size() < obs_histories.size()) {
				target_val_histories.push_back(it->second);
			}
		}
	}

	if (num_hits >= EXPERIMENT_MIN_HIT) {
		network = new Network(obs_histories[0].size());

		uniform_int_distribution<int> input_distribution(0, obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = input_distribution(generator);

			network->activate(obs_histories[rand_index]);

			double error = target_val_histories[rand_index] - network->output->acti_vals[0];

			network->backprop(error);
		}

		return true;
	} else {
		return false;
	}
}
