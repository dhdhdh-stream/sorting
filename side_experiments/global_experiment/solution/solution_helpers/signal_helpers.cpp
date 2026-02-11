#include "solution_helpers.h"

#include "build_network.h"
#include "build_network_helpers.h"
#include "globals.h"
#include "scope.h"

using namespace std;

const int SAMPLES_PER_ITER = 10;

void signal_add_sample(ScopeHistory* scope_history,
					   double target_val) {
	Scope* scope = scope_history->scope;

	vector<double> obs;
	obs.insert(obs.end(), scope_history->pre_obs_history.begin(), scope_history->pre_obs_history.end());
	obs.insert(obs.end(), scope_history->post_obs_history.begin(), scope_history->post_obs_history.end());

	if (scope->obs_histories.size() < SIGNAL_NUM_SAMPLES) {
		scope->obs_histories.push_back(obs);
		scope->target_val_histories.push_back(target_val);

		if (scope->obs_histories.size() >= SIGNAL_NUM_SAMPLES) {
			update_network(scope->obs_histories,
						   scope->target_val_histories,
						   scope->signal);
		}
	} else {
		scope->obs_histories[scope->history_index] = obs;
		scope->target_val_histories[scope->history_index] = target_val;
		scope->history_index++;

		uniform_int_distribution<int> sample_distribution(0, scope->obs_histories.size()-1);
		for (int s_index = 0; s_index < SAMPLES_PER_ITER; s_index++) {
			int sample_index = sample_distribution(generator);
			scope->signal->backprop(scope->obs_histories[sample_index],
									scope->target_val_histories[sample_index]);
		}

		if (scope->history_index >= SIGNAL_NUM_SAMPLES) {
			scope->history_index = 0;

			update_network(scope->obs_histories,
						   scope->target_val_histories,
						   scope->signal);
		}
	}
}
