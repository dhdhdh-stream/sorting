#include "signal_experiment.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "signal_network.h"

using namespace std;

const int SPLIT_NUM_INPUTS = 10;

#if defined(MDEBUG) && MDEBUG
const double SEED_RATIO = 0.2;

const int MAX_EPOCHS = 4;
const int ITERS_PER_EPOCH = 20;
#else
const double SEED_RATIO = 0.2;

const int MAX_EPOCHS = 30;
const int ITERS_PER_EPOCH = 10000;
#endif /* MDEBUG */

const double MAX_AVERAGE_ERROR = 0.1;

bool SignalExperiment::split_helper(vector<bool>& positive_has_match,
									vector<bool>& current_has_match,
									vector<bool>& new_match_input_is_pre,
									vector<int>& new_match_input_indexes,
									vector<int>& new_match_input_obs_indexes,
									SignalNetwork*& new_match_network) {
	vector<pair<bool,pair<int,int>>> possible_inputs;
	for (int i_index = 0; i_index < (int)this->explore_pre_obs[0].size(); i_index++) {
		for (int o_index = 0; o_index < (int)this->explore_pre_obs[0][i_index].size(); o_index++) {
			possible_inputs.push_back({true, {i_index, o_index}});
		}
	}
	for (int i_index = 0; i_index < (int)this->explore_post_obs[0].size(); i_index++) {
		for (int o_index = 0; o_index < (int)this->explore_post_obs[0][i_index].size(); o_index++) {
			possible_inputs.push_back({false, {i_index, o_index}});
		}
	}

	while (possible_inputs.size() > 0) {
		uniform_int_distribution<int> input_distribution(0, possible_inputs.size()-1);
		int input_index = input_distribution(generator);

		new_match_input_is_pre.push_back(possible_inputs[input_index].first);
		new_match_input_indexes.push_back(possible_inputs[input_index].second.first);
		new_match_input_obs_indexes.push_back(possible_inputs[input_index].second.second);

		if (new_match_input_is_pre.size() >= SPLIT_NUM_INPUTS) {
			break;
		}
	}

	new_match_network = new SignalNetwork(new_match_input_is_pre.size());

	int num_positive_seeds = SEED_RATIO * (double)this->positive_pre_obs.size();
	vector<int> positive_seeds;
	{
		vector<int> initial_possible_indexes(this->positive_pre_obs.size());
		for (int i_index = 0; i_index < (int)this->positive_pre_obs.size(); i_index++) {
			initial_possible_indexes[i_index] = i_index;
		}
		for (int s_index = 0; s_index < num_positive_seeds; s_index++) {
			uniform_int_distribution<int> possible_distribution(0, initial_possible_indexes.size()-1);
			int random_index = possible_distribution(generator);
			positive_seeds.push_back(initial_possible_indexes[random_index]);
			initial_possible_indexes.erase(initial_possible_indexes.begin() + random_index);
		}
	}

	int num_negative_seeds = SEED_RATIO * (double)this->explore_pre_obs.size();
	vector<int> negative_seeds;
	{
		vector<int> initial_possible_indexes(this->explore_pre_obs.size());
		for (int i_index = 0; i_index < (int)this->explore_pre_obs.size(); i_index++) {
			initial_possible_indexes[i_index] = i_index;
		}
		for (int s_index = 0; s_index < num_negative_seeds; s_index++) {
			uniform_int_distribution<int> possible_distribution(0, initial_possible_indexes.size()-1);
			int random_index = possible_distribution(generator);
			negative_seeds.push_back(initial_possible_indexes[random_index]);
			initial_possible_indexes.erase(initial_possible_indexes.begin() + random_index);
		}
	}

	uniform_int_distribution<int> is_positive_distribution(0, 1);
	uniform_int_distribution<int> positive_distribution(0, num_positive_seeds-1);
	uniform_int_distribution<int> negative_distribution(0, num_negative_seeds-1);
	int e_index = 0;
	while (true) {
		for (int iter_index = 0; iter_index < ITERS_PER_EPOCH; iter_index++) {
			vector<double> inputs(new_match_input_is_pre.size());

			bool is_positive = is_positive_distribution(generator) == 0;

			if (is_positive) {
				int random_index = positive_distribution(generator);
				int h_index = positive_seeds[random_index];
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						inputs[i_index] = this->positive_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						inputs[i_index] = this->positive_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
			} else {
				int random_index = negative_distribution(generator);
				int h_index = negative_seeds[random_index];
				for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
					if (new_match_input_is_pre[i_index]) {
						inputs[i_index] = this->explore_pre_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					} else {
						inputs[i_index] = this->explore_post_obs[h_index][
							new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
					}
				}
			}

			new_match_network->activate(inputs);

			double error;
			if (is_positive) {
				if (new_match_network->output->acti_vals[0] < 1.0) {
					error = 1.0 - new_match_network->output->acti_vals[0];
				}
			} else {
				if (new_match_network->output->acti_vals[0] > -1.0) {
					error = -1.0 - new_match_network->output->acti_vals[0];
				}
			}

			new_match_network->backprop(error);
		}

		vector<pair<double,int>> positive_acti_vals(this->positive_pre_obs.size());
		for (int h_index = 0; h_index < (int)this->positive_pre_obs.size(); h_index++) {
			vector<double> inputs(new_match_input_is_pre.size());
			for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
				if (new_match_input_is_pre[i_index]) {
					inputs[i_index] = this->positive_pre_obs[h_index][
						new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
				} else {
					inputs[i_index] = this->positive_post_obs[h_index][
						new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
				}
			}

			new_match_network->activate(inputs);

			positive_acti_vals[h_index] = {new_match_network->output->acti_vals[0], h_index};
		}
		sort(positive_acti_vals.begin(), positive_acti_vals.end());

		double sum_positive_errors = 0.0;
		for (int s_index = 0; s_index < num_positive_seeds; s_index++) {
			positive_seeds[s_index] = positive_acti_vals[positive_acti_vals.size() - 1 - s_index].second;

			if (positive_acti_vals[positive_acti_vals.size() - 1 - s_index].first < 1.0) {
				sum_positive_errors += abs(1.0 - positive_acti_vals[positive_acti_vals.size() - 1 - s_index].first);
			}
		}
		double average_positive_error = sum_positive_errors / (double)num_positive_seeds;

		vector<pair<double,int>> acti_vals(this->explore_pre_obs.size());
		for (int h_index = 0; h_index < (int)this->explore_pre_obs.size(); h_index++) {
			vector<double> inputs(new_match_input_is_pre.size());
			for (int i_index = 0; i_index < (int)new_match_input_is_pre.size(); i_index++) {
				if (new_match_input_is_pre[i_index]) {
					inputs[i_index] = this->explore_pre_obs[h_index][
						new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
				} else {
					inputs[i_index] = this->explore_post_obs[h_index][
						new_match_input_indexes[i_index]][new_match_input_obs_indexes[i_index]];
				}
			}

			new_match_network->activate(inputs);

			acti_vals[h_index] = {new_match_network->output->acti_vals[0], h_index};
		}
		sort(acti_vals.begin(), acti_vals.end());

		double sum_negative_errors = 0.0;
		for (int s_index = 0; s_index < num_negative_seeds; s_index++) {
			negative_seeds[s_index] = acti_vals[s_index].second;

			if (acti_vals[s_index].first > -1.0) {
				sum_negative_errors += abs(-1.0 - acti_vals[s_index].first);
			}
		}
		double average_negative_error = sum_negative_errors / (double)num_negative_seeds;

		#if defined(MDEBUG) && MDEBUG
		if ((average_positive_error <= MAX_AVERAGE_ERROR && average_negative_error <= MAX_AVERAGE_ERROR)
				|| rand()%4 == 0) {
		#else
		if (average_positive_error <= MAX_AVERAGE_ERROR && average_negative_error <= MAX_AVERAGE_ERROR) {
		#endif /* MDEBUG */
			return true;
		}

		e_index++;
		if (e_index >= MAX_EPOCHS) {
			return false;
		}
	}

	/**
	 * - unreachable
	 */
	return true;
}
