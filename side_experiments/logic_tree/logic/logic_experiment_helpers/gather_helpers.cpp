// TODO: split not good enough
// - then overfits
// - basing on random seeds averages out to nothing

#include "logic_experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_GATHER = 40;
#else
const int NUM_GATHER = 4000;
#endif /* MDEBUG */

const double SEED_RATIO = 0.1;
const double NON_SEED_RATIO = 0.5;
/**
 * - and train at 1 : 5
 */

#if defined(MDEBUG) && MDEBUG
const int MAX_EPOCH_ITERS = 2;
const int ITERS_PER_EPOCH = 10;
#else
const int MAX_EPOCH_ITERS = 10;
const int ITERS_PER_EPOCH = 100000;
#endif /* MDEBUG */

void LogicExperiment::gather_activate(vector<double>& obs,
									  double target_val) {
	this->obs_histories.push_back(obs);
	this->target_val_histories.push_back(target_val);

	if (this->obs_histories.size() >= NUM_GATHER) {
		vector<int> remaining_indexes;
		for (int i_index = 0; i_index < (int)this->obs_histories.size(); i_index++) {
			remaining_indexes.push_back(i_index);
		}

		int num_seeds = SEED_RATIO * (double)this->obs_histories.size();
		int num_non_seeds = NON_SEED_RATIO * (double)this->obs_histories.size();

		vector<vector<double>> seed_obs(num_seeds);
		vector<vector<double>> non_seed_obs(num_non_seeds);

		for (int i_index = 0; i_index < num_seeds; i_index++) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			seed_obs[i_index] = this->obs_histories[index];
			remaining_indexes.erase(remaining_indexes.begin() + index);
		}

		for (int i_index = 0; i_index < num_non_seeds; i_index++) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);
			non_seed_obs[i_index] = this->obs_histories[index];
			remaining_indexes.erase(remaining_indexes.begin() + index);
		}

		this->split_network = new Network(this->obs_histories[0].size(),
										  NETWORK_SIZE_SMALL);

		uniform_int_distribution<int> is_seed_distribution(0, 5);
		uniform_int_distribution<int> seed_distribution(0, num_seeds-1);
		uniform_int_distribution<int> non_seed_distribution(0, num_non_seeds-1);
		for (int epoch_index = 0; epoch_index < MAX_EPOCH_ITERS; epoch_index++) {
			for (int iter_index = 0; iter_index < ITERS_PER_EPOCH; iter_index++) {
				if (is_seed_distribution(generator) == 0) {
					int index = seed_distribution(generator);

					this->split_network->activate(seed_obs[index]);

					double error;
					if (this->split_network->output->acti_vals[0] >= 1.0) {
						error = 0.0;
					} else {
						error = 1.0 - this->split_network->output->acti_vals[0];
					}

					this->split_network->backprop(error);
				} else {
					int index = non_seed_distribution(generator);

					this->split_network->activate(non_seed_obs[index]);

					double error;
					if (this->split_network->output->acti_vals[0] <= -1.0) {
						error = 0.0;
					} else {
						error = -1.0 - this->split_network->output->acti_vals[0];
					}

					this->split_network->backprop(error);
				}
			}

			vector<pair<double,int>> acti_vals;
			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				this->split_network->activate(this->obs_histories[h_index]);

				acti_vals.push_back({this->split_network->output->acti_vals[0], h_index});
			}
			sort(acti_vals.begin(), acti_vals.end());

			if (acti_vals[num_non_seeds-1].first <= -1.0
					&& acti_vals[this->obs_histories.size() - num_seeds].first >= 1.0) {
				break;
			}

			for (int i_index = 0; i_index < num_seeds; i_index++) {
				seed_obs[i_index] = this->obs_histories[
					acti_vals[this->obs_histories.size() - 1 - i_index].second];
			}

			for (int i_index = 0; i_index < num_non_seeds; i_index++) {
				non_seed_obs[i_index] = this->obs_histories[acti_vals[i_index].second];
			}
		}

		this->eval_network = new Network(this->obs_histories[0].size(),
										 NETWORK_SIZE_SMALL);

		vector<vector<double>> match_obs;
		vector<double> match_target_vals;
		for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
			// this->split_network->activate(this->obs_histories[h_index]);
			// #if defined(MDEBUG) && MDEBUG
			// if (this->split_network->output->acti_vals[0] > 0.0 || rand()%2 == 0) {
			// #else
			// if (this->split_network->output->acti_vals[0] > 0.0) {
			// #endif /* MDEBUG */
			// temp
			if (this->obs_histories[h_index][0] == 20.0) {
				match_obs.push_back(this->obs_histories[h_index]);
				match_target_vals.push_back(this->target_val_histories[h_index]);
			}
		}

		cout << "match_obs.size(): " << match_obs.size() << endl;

		uniform_int_distribution<int> match_distribution(0, match_obs.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = match_distribution(generator);

			this->eval_network->activate(match_obs[index]);

			double error = match_target_vals[index] - this->eval_network->output->acti_vals[0];

			this->eval_network->backprop(error);
		}

		this->sum_improvement = 0.0;
		this->count = 0;

		this->state = LOGIC_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
