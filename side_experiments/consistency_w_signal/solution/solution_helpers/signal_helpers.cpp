#include "solution_helpers.h"

#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

double calc_signal(vector<ScopeHistory*>& post_scope_histories,
				   double target_val,
				   SolutionWrapper* wrapper) {
	double sum_vals = target_val - wrapper->solution->curr_score;
	for (int h_index = 0; h_index < (int)post_scope_histories.size(); h_index++) {
		if (!post_scope_histories[h_index]->signal_initialized) {
			post_scope_histories[h_index]->signal_initialized = true;

			Scope* scope = post_scope_histories[h_index]->scope;

			vector<double> inputs = post_scope_histories[h_index]->pre_obs;
			inputs.insert(inputs.end(), post_scope_histories[h_index]->post_obs.begin(), post_scope_histories[h_index]->post_obs.end());

			scope->consistency_network->activate(inputs);
			double consistency = scope->consistency_network->output->acti_vals[0];
			if (consistency <= 0.0) {
				post_scope_histories[h_index]->signal_val = 0.0;
			} else {
				if (consistency > 1.0) {
					consistency = 1.0;
				}

				scope->signal_network->activate(inputs);
				double diff = scope->signal_network->output->acti_vals[0];

				post_scope_histories[h_index]->signal_val = consistency * diff;
			}
		}
		sum_vals += post_scope_histories[h_index]->signal_val;
	}

	return sum_vals;
}
