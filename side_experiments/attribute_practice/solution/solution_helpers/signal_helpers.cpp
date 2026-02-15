#include "solution_helpers.h"

#include "build_network.h"
#include "build_network_helpers.h"
#include "globals.h"
#include "scope.h"

using namespace std;

const int SAMPLES_PER_ITER = 10;

void signal_add_existing_sample(ScopeHistory* scope_history,
								double target_val) {
	Scope* scope = scope_history->scope;

	if (scope->existing_pre_obs_histories.size() < SIGNAL_NUM_SAMPLES) {
		scope->existing_pre_obs_histories.push_back(scope_history->pre_obs_history);
		scope->existing_post_obs_histories.push_back(scope_history->post_obs_history);
		scope->existing_target_val_histories.push_back(target_val);
	} else {
		scope->existing_pre_obs_histories[scope->existing_history_index] = scope_history->pre_obs_history;
		scope->existing_post_obs_histories[scope->existing_history_index] = scope_history->post_obs_history;
		scope->existing_target_val_histories[scope->existing_history_index] = target_val;
		scope->existing_history_index++;

		if (scope->existing_history_index >= SIGNAL_NUM_SAMPLES) {
			scope->existing_history_index = 0;
		}
	}
}

void signal_add_explore_sample(ScopeHistory* scope_history,
							   double target_val) {
	Scope* scope = scope_history->scope;

	if (scope->explore_pre_obs_histories.size() < SIGNAL_NUM_SAMPLES) {
		scope->explore_pre_obs_histories.push_back(scope_history->pre_obs_history);
		scope->explore_post_obs_histories.push_back(scope_history->post_obs_history);
		scope->explore_target_val_histories.push_back(target_val);

		if (scope->explore_pre_obs_histories.size() >= SIGNAL_NUM_SAMPLES) {
			update_network(scope->existing_pre_obs_histories,
						   scope->existing_target_val_histories,
						   scope->explore_pre_obs_histories,
						   scope->explore_target_val_histories,
						   scope->pre_signal);

			vector<vector<double>> existing_combined_obs_histories(scope->existing_pre_obs_histories.size());
			for (int h_index = 0; h_index < (int)scope->existing_pre_obs_histories.size(); h_index++) {
				vector<double> obs;
				obs.insert(obs.end(), scope->existing_pre_obs_histories[h_index].begin(), scope->existing_pre_obs_histories[h_index].end());
				obs.insert(obs.end(), scope->existing_post_obs_histories[h_index].begin(), scope->existing_post_obs_histories[h_index].end());
				existing_combined_obs_histories[h_index] = obs;
			}

			vector<vector<double>> explore_combined_obs_histories(scope->explore_pre_obs_histories.size());
			for (int h_index = 0; h_index < (int)scope->explore_pre_obs_histories.size(); h_index++) {
				vector<double> obs;
				obs.insert(obs.end(), scope->explore_pre_obs_histories[h_index].begin(), scope->explore_pre_obs_histories[h_index].end());
				obs.insert(obs.end(), scope->explore_post_obs_histories[h_index].begin(), scope->explore_post_obs_histories[h_index].end());
				explore_combined_obs_histories[h_index] = obs;
			}

			update_network(existing_combined_obs_histories,
						   scope->existing_target_val_histories,
						   explore_combined_obs_histories,
						   scope->explore_target_val_histories,
						   scope->post_signal);
		}
	} else {
		scope->explore_pre_obs_histories[scope->explore_history_index] = scope_history->pre_obs_history;
		scope->explore_post_obs_histories[scope->explore_history_index] = scope_history->post_obs_history;
		scope->explore_target_val_histories[scope->explore_history_index] = target_val;
		scope->explore_history_index++;

		uniform_int_distribution<int> sample_distribution(0, scope->explore_pre_obs_histories.size()-1);
		for (int s_index = 0; s_index < SAMPLES_PER_ITER; s_index++) {
			int sample_index = sample_distribution(generator);

			scope->pre_signal->backprop(scope->explore_pre_obs_histories[sample_index],
										scope->explore_target_val_histories[sample_index]);

			vector<double> obs;
			obs.insert(obs.end(), scope->explore_pre_obs_histories[sample_index].begin(), scope->explore_pre_obs_histories[sample_index].end());
			obs.insert(obs.end(), scope->explore_post_obs_histories[sample_index].begin(), scope->explore_post_obs_histories[sample_index].end());

			scope->post_signal->backprop(obs,
										 scope->explore_target_val_histories[sample_index]);
		}

		if (scope->explore_history_index >= SIGNAL_NUM_SAMPLES) {
			scope->explore_history_index = 0;

			update_network(scope->existing_pre_obs_histories,
						   scope->existing_target_val_histories,
						   scope->explore_pre_obs_histories,
						   scope->explore_target_val_histories,
						   scope->pre_signal);

			vector<vector<double>> existing_combined_obs_histories(scope->existing_pre_obs_histories.size());
			for (int h_index = 0; h_index < (int)scope->existing_pre_obs_histories.size(); h_index++) {
				vector<double> obs;
				obs.insert(obs.end(), scope->existing_pre_obs_histories[h_index].begin(), scope->existing_pre_obs_histories[h_index].end());
				obs.insert(obs.end(), scope->existing_post_obs_histories[h_index].begin(), scope->existing_post_obs_histories[h_index].end());
				existing_combined_obs_histories[h_index] = obs;
			}

			vector<vector<double>> explore_combined_obs_histories(scope->explore_pre_obs_histories.size());
			for (int h_index = 0; h_index < (int)scope->explore_pre_obs_histories.size(); h_index++) {
				vector<double> obs;
				obs.insert(obs.end(), scope->explore_pre_obs_histories[h_index].begin(), scope->explore_pre_obs_histories[h_index].end());
				obs.insert(obs.end(), scope->explore_post_obs_histories[h_index].begin(), scope->explore_post_obs_histories[h_index].end());
				explore_combined_obs_histories[h_index] = obs;
			}

			update_network(existing_combined_obs_histories,
						   scope->existing_target_val_histories,
						   explore_combined_obs_histories,
						   scope->explore_target_val_histories,
						   scope->post_signal);
		}
	}
}
