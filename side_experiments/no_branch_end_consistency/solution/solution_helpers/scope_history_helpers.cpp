#include "solution_helpers.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void add_existing_samples_helper(ScopeHistory* scope_history) {
	Scope* scope = scope_history->scope;

	if ((int)scope->existing_pre_obs.size() < MEASURE_ITERS) {
		scope->existing_pre_obs.push_back(scope_history->pre_obs);
		scope->existing_post_obs.push_back(scope_history->post_obs);
	} else {
		uniform_int_distribution<int> distribution(0, scope->existing_pre_obs.size()-1);
		int index = distribution(generator);
		scope->existing_pre_obs[index] = scope_history->pre_obs;
		scope->existing_post_obs[index] = scope_history->post_obs;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			add_existing_samples_helper(scope_node_history->scope_history);
		}
	}
}

void calc_consistency_helper(ScopeHistory* scope_history,
							 double& sum_val,
							 int& count) {
	Scope* scope = scope_history->scope;

	if (scope->consistency_network != NULL) {
		vector<double> input = scope_history->pre_obs;
		input.insert(input.end(), scope_history->post_obs.begin(),
			scope_history->post_obs.end());

		scope->consistency_network->activate(input);
		double consistency = scope->consistency_network->output->acti_vals[0];
		/**
		 * - allow to go below -1.0 to help distinguish between bad and very bad
		 */
		if (consistency >= 3.0) {
			sum_val += 3.0;
		} else if (consistency <= -3.0) {
			sum_val += -3.0;
		} else {
			sum_val += consistency;
		}
		count++;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			calc_consistency_helper(scope_node_history->scope_history,
									sum_val,
									count);
		}
	}
}

double calc_consistency(vector<ScopeHistory*>& scope_histories) {
	double sum_consistency = 0.0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		double sum_val = 0.0;
		int count = 0;
		calc_consistency_helper(scope_histories[h_index],
								sum_val,
								count);

		double average_consistency;
		if (count == 0) {
			average_consistency = 0.0;
		} else {
			average_consistency = sum_val / count;
		}
		sum_consistency += average_consistency;
	}

	return sum_consistency / (double)scope_histories.size();
}
