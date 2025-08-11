#include "helpers.h"

#include <iostream>

#include "constants.h"
#include "network.h"
#include "signal.h"

using namespace std;

double calc_signal(vector<vector<double>>& pre_obs_histories,
				   vector<vector<double>>& post_obs_histories,
				   vector<Signal*>& potential_signals,
				   double potential_miss_average_guess) {
	for (int s_index = 0; s_index < (int)potential_signals.size(); s_index++) {
		vector<double> input_vals(potential_signals[s_index]->match_input_is_pre.size());
		for (int i_index = 0; i_index < (int)potential_signals[s_index]->match_input_is_pre.size(); i_index++) {
			if (potential_signals[s_index]->match_input_is_pre[i_index]) {
				input_vals[i_index] = pre_obs_histories[
					potential_signals[s_index]->match_input_indexes[i_index]][potential_signals[s_index]->match_input_obs_indexes[i_index]];
			} else {
				input_vals[i_index] = post_obs_histories[
					potential_signals[s_index]->match_input_indexes[i_index]][potential_signals[s_index]->match_input_obs_indexes[i_index]];
			}
		}
		potential_signals[s_index]->match_network->activate(input_vals);
		#if defined(MDEBUG) && MDEBUG
		if (rand()%3 == 0) {
		#else
		if (potential_signals[s_index]->match_network->output->acti_vals[0] >= MATCH_WEIGHT) {
		#endif /* MDEBUG */
			vector<double> input_vals(potential_signals[s_index]->score_input_is_pre.size());
			for (int i_index = 0; i_index < (int)potential_signals[s_index]->score_input_is_pre.size(); i_index++) {
				if (potential_signals[s_index]->score_input_is_pre[i_index]) {
					input_vals[i_index] = pre_obs_histories[
						potential_signals[s_index]->score_input_indexes[i_index]][potential_signals[s_index]->score_input_obs_indexes[i_index]];
				} else {
					input_vals[i_index] = post_obs_histories[
						potential_signals[s_index]->score_input_indexes[i_index]][potential_signals[s_index]->score_input_obs_indexes[i_index]];
				}
			}
			potential_signals[s_index]->score_network->activate(input_vals);
			return potential_signals[s_index]->score_network->output->acti_vals[0];
		}
	}

	return potential_miss_average_guess;
}
