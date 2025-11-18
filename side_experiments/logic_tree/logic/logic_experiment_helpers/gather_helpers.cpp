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

#if defined(MDEBUG) && MDEBUG
const int TRAIN_MIN_SAMPLES = 4;
#else
const int TRAIN_MIN_SAMPLES = 100;
#endif /* MDEBUG */

void LogicExperiment::gather_activate(vector<double>& obs,
									  double target_val) {
	this->obs_histories.push_back(obs);
	this->target_val_histories.push_back(target_val);

	if (this->obs_histories.size() >= NUM_GATHER) {
		vector<vector<double>> match_obs;
		vector<double> match_target_vals;

		uniform_int_distribution<int> obs_distribution(0, this->obs_histories[0].size()-1);
		uniform_int_distribution<int> type_distribution(0, 13);
		while (true) {
			if (this->obs_histories[0].size() > 1) {
				uniform_int_distribution<int> type_distribution(0, 7);
				this->split_type = type_distribution(generator);
				switch (this->split_type) {
				case SPLIT_TYPE_GREATER:
				case SPLIT_TYPE_GREATER_EQUAL:
				case SPLIT_TYPE_LESSER:
				case SPLIT_TYPE_LESSER_EQUAL:
					this->obs_index = obs_distribution(generator);
					this->rel_obs_index = -1;
					this->split_target = this->obs_histories[0][this->obs_index];
					this->split_range = 0.0;
					break;
				case SPLIT_TYPE_WITHIN:
				case SPLIT_TYPE_WITHIN_EQUAL:
				case SPLIT_TYPE_WITHOUT:
				case SPLIT_TYPE_WITHOUT_EQUAL:
					this->obs_index = obs_distribution(generator);
					this->rel_obs_index = -1;
					this->split_target = this->obs_histories[0][this->obs_index];
					this->split_range = abs(this->split_target - this->obs_histories[1][this->obs_index]);
					break;
				}
			} else {
				uniform_int_distribution<int> type_distribution(0, 13);
				this->split_type = type_distribution(generator);
				switch (this->split_type) {
				case SPLIT_TYPE_GREATER:
				case SPLIT_TYPE_GREATER_EQUAL:
				case SPLIT_TYPE_LESSER:
				case SPLIT_TYPE_LESSER_EQUAL:
					this->obs_index = obs_distribution(generator);
					this->rel_obs_index = -1;
					this->split_target = this->obs_histories[0][this->obs_index];
					this->split_range = 0.0;
					break;
				case SPLIT_TYPE_WITHIN:
				case SPLIT_TYPE_WITHIN_EQUAL:
				case SPLIT_TYPE_WITHOUT:
				case SPLIT_TYPE_WITHOUT_EQUAL:
					this->obs_index = obs_distribution(generator);
					this->rel_obs_index = -1;
					this->split_target = this->obs_histories[0][this->obs_index];
					this->split_range = abs(this->split_target - this->obs_histories[1][this->obs_index]);
					break;
				case SPLIT_TYPE_REL_GREATER:
				case SPLIT_TYPE_REL_GREATER_EQUAL:
					this->obs_index = obs_distribution(generator);
					while (true) {
						this->rel_obs_index = obs_distribution(generator);
						if (this->rel_obs_index != this->obs_index) {
							break;
						}
					}
					this->split_target = this->obs_histories[0][this->obs_index] - this->obs_histories[0][this->rel_obs_index];
					this->split_range = 0.0;
					break;
				case SPLIT_TYPE_REL_WITHIN:
				case SPLIT_TYPE_REL_WITHIN_EQUAL:
				case SPLIT_TYPE_REL_WITHOUT:
				case SPLIT_TYPE_REL_WITHOUT_EQUAL:
					this->obs_index = obs_distribution(generator);
					while (true) {
						this->rel_obs_index = obs_distribution(generator);
						if (this->rel_obs_index != this->obs_index) {
							break;
						}
					}
					this->split_target = this->obs_histories[0][this->obs_index] - this->obs_histories[0][this->rel_obs_index];
					this->split_range = abs(this->split_target - (this->obs_histories[1][this->obs_index] - this->obs_histories[1][this->rel_obs_index]));
					break;
				}
			}

			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				switch (this->split_type) {
				case SPLIT_TYPE_GREATER:
					if (this->obs_histories[h_index][this->obs_index] > this->split_target) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_GREATER_EQUAL:
					if (this->obs_histories[h_index][this->obs_index] >= this->split_target) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_LESSER:
					if (this->obs_histories[h_index][this->obs_index] < this->split_target) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_LESSER_EQUAL:
					if (this->obs_histories[h_index][this->obs_index] <= this->split_target) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_WITHIN:
					if (abs(this->obs_histories[h_index][this->obs_index] - this->split_target) < this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_WITHIN_EQUAL:
					if (abs(this->obs_histories[h_index][this->obs_index] - this->split_target) <= this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_WITHOUT:
					if (abs(this->obs_histories[h_index][this->obs_index] - this->split_target) > this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_WITHOUT_EQUAL:
					if (abs(this->obs_histories[h_index][this->obs_index] - this->split_target) >= this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_REL_GREATER:
					if (this->obs_histories[h_index][this->obs_index] - this->obs_histories[h_index][this->rel_obs_index] > this->split_target) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_REL_GREATER_EQUAL:
					if (this->obs_histories[h_index][this->obs_index] - this->obs_histories[h_index][this->rel_obs_index] >= this->split_target) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_REL_WITHIN:
					if (abs((this->obs_histories[h_index][this->obs_index] - this->obs_histories[h_index][this->rel_obs_index]) - this->split_target) < this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_REL_WITHIN_EQUAL:
					if (abs((this->obs_histories[h_index][this->obs_index] - this->obs_histories[h_index][this->rel_obs_index]) - this->split_target) <= this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_REL_WITHOUT:
					if (abs((this->obs_histories[h_index][this->obs_index] - this->obs_histories[h_index][this->rel_obs_index]) - this->split_target) > this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				case SPLIT_TYPE_REL_WITHOUT_EQUAL:
					if (abs((this->obs_histories[h_index][this->obs_index] - this->obs_histories[h_index][this->rel_obs_index]) - this->split_target) >= this->split_range) {
						match_obs.push_back(this->obs_histories[h_index]);
						match_target_vals.push_back(this->target_val_histories[h_index]);
					}
					break;
				}
			}

			if (match_obs.size() >= TRAIN_MIN_SAMPLES) {
				break;
			} else {
				match_obs.clear();
				match_target_vals.clear();
			}
		}

		this->eval_network = new Network(this->obs_histories[0].size(),
										 NETWORK_SIZE_SMALL);

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
