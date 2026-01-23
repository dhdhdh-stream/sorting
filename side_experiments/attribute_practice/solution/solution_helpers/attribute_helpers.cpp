#include "solution_helpers.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int LONG_NUM_SAMPLES = 10;
#else
const int LONG_NUM_SAMPLES = 1000;
#endif /* MDEBUG */

const int EXISTING_MAX_WEIGHT = 9;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_ITERS = 10;
#else
const int UPDATE_ITERS = 10000;
#endif /* MDEBUG */

void update_attribute(ScopeHistory* scope_history,
					  double target_val) {
	Scope* scope = scope_history->scope;

	scope->pre_obs.push_back(scope_history->pre_obs_history);
	scope->pre_targets.push_back(target_val - scope_history->pre_impact);
	scope->post_obs.push_back(scope_history->post_obs_history);
	scope->post_targets.push_back(target_val - scope_history->post_impact);

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			update_attribute(scope_node_history->scope_history,
							 target_val);
		}
	}
}

void check_attribute_init(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];

		if (scope->pre_obs.size() >= LONG_NUM_SAMPLES) {
			if (scope->pre_network == NULL) {
				uniform_int_distribution<int> input_distribution(0, scope->pre_obs.size()-1);

				Network* pre_network = new Network(scope->pre_obs[0].size(),
												   NETWORK_SIZE_SMALL);
				scope->pre_network = pre_network;

				for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
					int rand_index = input_distribution(generator);

					pre_network->activate(scope->pre_obs[rand_index]);

					double error = scope->pre_targets[rand_index] - pre_network->output->acti_vals[0];

					pre_network->backprop(error);
				}

				for (int h_index = 0; h_index < (int)scope->pre_obs.size(); h_index++) {
					pre_network->activate(scope->pre_obs[h_index]);
					scope->post_targets[h_index] -= pre_network->output->acti_vals[0];
				}

				Network* post_network = new Network(scope->post_obs[0].size(),
													NETWORK_SIZE_SMALL);
				scope->post_network = post_network;

				for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
					int rand_index = input_distribution(generator);

					post_network->activate(scope->post_obs[rand_index]);

					double error = scope->post_targets[rand_index] - post_network->output->acti_vals[0];

					post_network->backprop(error);
				}
			} else {
				uniform_int_distribution<int> input_distribution(0, scope->pre_obs.size()-1);

				Network* pre_network = scope->pre_network;

				vector<double> pre_adjusted_targets(scope->pre_obs.size());
				for (int h_index = 0; h_index < (int)scope->pre_obs.size(); h_index++) {
					pre_network->activate(scope->pre_obs[h_index]);
					if (scope->long_iter > EXISTING_MAX_WEIGHT) {
						pre_adjusted_targets[h_index] = (EXISTING_MAX_WEIGHT * pre_network->output->acti_vals[0] + scope->pre_targets[h_index]) / (1 + EXISTING_MAX_WEIGHT);
					} else {
						pre_adjusted_targets[h_index] = (scope->long_iter * pre_network->output->acti_vals[0] + scope->pre_targets[h_index]) / (1 + scope->long_iter);
					}
				}

				for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
					int rand_index = input_distribution(generator);

					pre_network->activate(scope->pre_obs[rand_index]);

					double error = pre_adjusted_targets[rand_index] - pre_network->output->acti_vals[0];

					pre_network->backprop(error);
				}

				Network* post_network = scope->post_network;

				vector<double> post_adjusted_targets(scope->post_obs.size());
				for (int h_index = 0; h_index < (int)scope->post_obs.size(); h_index++) {
					post_network->activate(scope->post_obs[h_index]);
					if (scope->long_iter > EXISTING_MAX_WEIGHT) {
						post_adjusted_targets[h_index] = (EXISTING_MAX_WEIGHT * post_network->output->acti_vals[0] + scope->post_targets[h_index]) / (1 + EXISTING_MAX_WEIGHT);
					} else {
						post_adjusted_targets[h_index] = (scope->long_iter * post_network->output->acti_vals[0] + scope->post_targets[h_index]) / (1 + scope->long_iter);
					}
				}

				for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
					int rand_index = input_distribution(generator);

					post_network->activate(scope->post_obs[rand_index]);

					double error = post_adjusted_targets[rand_index] - post_network->output->acti_vals[0];

					post_network->backprop(error);
				}
			}

			scope->pre_obs.clear();
			scope->pre_targets.clear();
			scope->post_obs.clear();
			scope->post_targets.clear();
			scope->long_iter++;
		}
	}
}
