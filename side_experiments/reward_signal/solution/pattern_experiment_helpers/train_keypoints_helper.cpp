#include "pattern_experiment.h"

#include "globals.h"
#include "network.h"
#include "pattern.h"
#include "solution_helpers.h"

using namespace std;

const int TRAIN_ITERS = 1000;
const double EXISTING_MIN_MATCH_PERCENT = 0.95;

const int MAX_TRAIN_ITERS = 50000;

bool PatternExperiment::train_keypoints_helper(Pattern* potential_pattern) {
	vector<vector<double>> existing_vals(this->existing_scope_histories.size());
	vector<vector<bool>> existing_is_on(this->existing_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		vector<double> curr_vals(potential_pattern->keypoints.size());
		vector<bool> curr_is_on(potential_pattern->keypoints.size());
		for (int k_index = 0; k_index < (int)potential_pattern->keypoints.size(); k_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->existing_scope_histories[h_index],
							   potential_pattern->keypoints[k_index],
							   0,
							   val,
							   is_on);
			curr_vals[k_index] = val;
			curr_is_on[k_index] = is_on;
		}
		existing_vals[h_index] = curr_vals;
		existing_is_on[h_index] = curr_is_on;
	}

	vector<vector<double>> explore_vals(this->explore_scope_histories.size());
	vector<vector<bool>> explore_is_on(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		vector<double> curr_vals(potential_pattern->keypoints.size());
		vector<bool> curr_is_on(potential_pattern->keypoints.size());
		for (int k_index = 0; k_index < (int)potential_pattern->keypoints.size(); k_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->explore_scope_histories[h_index],
							   potential_pattern->keypoints[k_index],
							   0,
							   val,
							   is_on);
			curr_vals[k_index] = val;
			curr_is_on[k_index] = is_on;
		}
		explore_vals[h_index] = curr_vals;
		explore_is_on[h_index] = curr_is_on;
	}

	potential_pattern->keypoint_network = new Network((int)potential_pattern->keypoints.size(),
													  explore_vals,
													  explore_is_on);

	uniform_int_distribution<int> is_existing_distribution(0, 5);
	uniform_int_distribution<int> existing_distribution(0, (int)this->existing_scope_histories.size()-1);
	uniform_int_distribution<int> explore_distribution(0, (int)this->explore_scope_histories.size()-1);
	double curr_explore_target = -0.2;
	int train_iter = 0;
	while (true) {
		train_iter++;
		if (train_iter >= MAX_TRAIN_ITERS) {
			return false;
		}

		int existing_num_match = 0;
		int existing_count = 0;
		int explore_num_match = 0;
		int explore_count = 0;
		for (int t_index = 0; t_index < TRAIN_ITERS; t_index++) {
			if (is_existing_distribution(generator) == 0) {
				existing_count++;

				int random_index = existing_distribution(generator);
				potential_pattern->keypoint_network->activate(existing_vals[random_index],
															  existing_is_on[random_index]);

				if (potential_pattern->keypoint_network->output->acti_vals[0] > 0.0) {
					existing_num_match++;
				}

				double error = 1.0 - potential_pattern->keypoint_network->output->acti_vals[0];
				potential_pattern->keypoint_network->backprop(error);
			} else {
				explore_count++;

				int random_index = explore_distribution(generator);
				potential_pattern->keypoint_network->activate(explore_vals[random_index],
															  explore_is_on[random_index]);

				if (potential_pattern->keypoint_network->output->acti_vals[0] > 0.0) {
					explore_num_match++;
				}

				double error = curr_explore_target - potential_pattern->keypoint_network->output->acti_vals[0];
				potential_pattern->keypoint_network->backprop(error);
			}
		}

		double existing_match_percent = (double)existing_num_match / (double)existing_count;
		double explore_match_percent = (double)explore_num_match / (double)explore_count;
		if (existing_match_percent >= EXISTING_MIN_MATCH_PERCENT
				&& explore_match_percent >= PATTERN_MIN_MATCH_PERCENT
				&& explore_match_percent <= PATTERN_MAX_MATCH_PERCENT) {
			break;
		} else {
			if (explore_match_percent > PATTERN_MAX_MATCH_PERCENT) {
				curr_explore_target *= 1.25;
			} else if (explore_match_percent < PATTERN_MIN_MATCH_PERCENT) {
				curr_explore_target *= 0.8;
			}
		}
	}

	return true;
}
