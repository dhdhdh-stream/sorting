#include "decision_tree_helpers.h"

#include "globals.h"

using namespace std;

const double MAX_RATIO = 0.8;
const double MIN_RATIO = 0.2;

const int NUM_PRE_FILTER = 20;

void split_try(vector<vector<double>>& train_obs_histories,
			   vector<double>& train_target_val_histories,
			   int& obs_index,
			   int& rel_obs_index,
			   int& split_type,
			   double& split_target,
			   double& split_range) {
	double best_diff = 0.0;
	int count = 0;
	uniform_int_distribution<int> obs_distribution(0, train_obs_histories[0].size()-1);
	uniform_int_distribution<int> train_distribution(0, train_obs_histories.size()-1);
	while (true) {
		int potential_split_type;
		if (train_obs_histories[0].size() <= 1) {
			uniform_int_distribution<int> type_distribution(0, 7);
			potential_split_type = type_distribution(generator);
		} else {
			uniform_int_distribution<int> type_distribution(0, 13);
			potential_split_type = type_distribution(generator);
		}

		int potential_obs_index;
		int potential_rel_obs_index;
		double potential_split_target;
		double potential_split_range;
		switch (potential_split_type) {
		case SPLIT_TYPE_GREATER:
		case SPLIT_TYPE_GREATER_EQUAL:
		case SPLIT_TYPE_LESSER:
		case SPLIT_TYPE_LESSER_EQUAL:
			{
				potential_obs_index = obs_distribution(generator);
				potential_rel_obs_index = -1;
				int potential_index = train_distribution(generator);
				potential_split_target = train_obs_histories[potential_index][potential_obs_index];
				potential_split_range = 0.0;
			}
			break;
		case SPLIT_TYPE_WITHIN:
		case SPLIT_TYPE_WITHIN_EQUAL:
		case SPLIT_TYPE_WITHOUT:
		case SPLIT_TYPE_WITHOUT_EQUAL:
			{
				potential_obs_index = obs_distribution(generator);
				potential_rel_obs_index = -1;
				int potential_index = train_distribution(generator);
				potential_split_target = train_obs_histories[potential_index][potential_obs_index];
				int potential_rel_index = train_distribution(generator);
				potential_split_range = abs(potential_split_target - train_obs_histories[potential_rel_index][potential_obs_index]);
			}
			break;
		case SPLIT_TYPE_REL_GREATER:
		case SPLIT_TYPE_REL_GREATER_EQUAL:
			{
				potential_obs_index = obs_distribution(generator);
				while (true) {
					potential_rel_obs_index = obs_distribution(generator);
					if (potential_rel_obs_index != potential_obs_index) {
						break;
					}
				}
				int potential_index = train_distribution(generator);
				potential_split_target = train_obs_histories[potential_index][potential_obs_index] - train_obs_histories[potential_index][potential_rel_obs_index];
				potential_split_range = 0.0;
			}
			break;
		case SPLIT_TYPE_REL_WITHIN:
		case SPLIT_TYPE_REL_WITHIN_EQUAL:
		case SPLIT_TYPE_REL_WITHOUT:
		case SPLIT_TYPE_REL_WITHOUT_EQUAL:
			{
				potential_obs_index = obs_distribution(generator);
				while (true) {
					potential_rel_obs_index = obs_distribution(generator);
					if (potential_rel_obs_index != potential_obs_index) {
						break;
					}
				}
				int potential_index = train_distribution(generator);
				potential_split_target = train_obs_histories[potential_index][potential_obs_index] - train_obs_histories[potential_index][potential_rel_obs_index];
				int potential_rel_index = train_distribution(generator);
				potential_split_range = abs(potential_split_target - (train_obs_histories[potential_rel_index][potential_obs_index] - train_obs_histories[potential_rel_index][potential_rel_obs_index]));
			}
			break;
		}

		vector<vector<double>> match_obs;
		vector<double> match_target_vals;
		vector<vector<double>> non_match_obs;
		vector<double> non_match_target_vals;
		for (int h_index = 0; h_index < (int)train_obs_histories.size(); h_index++) {
			bool is_match = is_match_helper(train_obs_histories[h_index],
											potential_obs_index,
											potential_rel_obs_index,
											potential_split_type,
											potential_split_target,
											potential_split_range);

			if (is_match) {
				match_obs.push_back(train_obs_histories[h_index]);
				match_target_vals.push_back(train_target_val_histories[h_index]);
			} else {
				non_match_obs.push_back(train_obs_histories[h_index]);
				non_match_target_vals.push_back(train_target_val_histories[h_index]);
			}
		}

		double max_num_samples = MAX_RATIO * (double)train_obs_histories.size();
		double min_num_samples = MIN_RATIO * (double)train_obs_histories.size();
		if (match_obs.size() < min_num_samples || match_obs.size() > max_num_samples) {
			continue;
		}

		double match_sum_vals = 0.0;
		for (int i_index = 0; i_index < (int)match_target_vals.size(); i_index++) {
			match_sum_vals += match_target_vals[i_index];
		}
		double match_val_average = match_sum_vals / (double)match_target_vals.size();

		double non_match_sum_vals = 0.0;
		for (int i_index = 0; i_index < (int)non_match_target_vals.size(); i_index++) {
			non_match_sum_vals += non_match_target_vals[i_index];
		}
		double non_match_val_average = non_match_sum_vals / (double)non_match_target_vals.size();

		double curr_diff = abs(match_val_average - non_match_val_average);
		if (curr_diff > best_diff) {
			split_type = potential_split_type;
			obs_index = potential_obs_index;
			rel_obs_index = potential_rel_obs_index;
			split_target = potential_split_target;
			split_range = potential_split_range;

			best_diff = curr_diff;
		}

		count++;
		if (count >= NUM_PRE_FILTER) {
			break;
		}
	}
}
