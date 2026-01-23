#include "solution_helpers.h"

#include "constants.h"
#include "globals.h"
#include "long_network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int LONG_INIT_NUM_SAMPLES = 100;
#else
const int LONG_INIT_NUM_SAMPLES = 10000;
/**
 * - enough to handle 10% impact
 */
#endif /* MDEBUG */

void update_attribute(ScopeHistory* scope_history,
					  double target_val) {
	Scope* scope = scope_history->scope;

	if (scope->pre_network == NULL) {
		scope->init_pre_obs.push_back(scope_history->pre_obs_history);
		scope->init_pre_targets.push_back(target_val - scope_history->pre_impact);
		scope->init_post_obs.push_back(scope_history->post_obs_history);
		scope->init_post_targets.push_back(target_val - scope_history->post_impact);
	} else {
		scope->pre_network->activate(scope_history->pre_obs_history);
		double pre_target = target_val - scope_history->pre_impact;
		double pre_error = pre_target - scope->pre_network->output->acti_vals[0];
		scope->pre_network->backprop(pre_error);

		scope->post_network->activate(scope_history->post_obs_history);
		double post_target = target_val - scope_history->post_impact;
		double post_error = post_target - scope->post_network->output->acti_vals[0];
		scope->post_network->backprop(post_error);
	}

	// temp
	scope->long_iters++;

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

		if (scope->pre_network == NULL
				&& scope->init_pre_obs.size() >= LONG_INIT_NUM_SAMPLES) {
			uniform_int_distribution<int> input_distribution(0, scope->init_pre_obs.size()-1);

			scope->pre_network = new LongNetwork(scope->init_pre_obs[0].size());
			
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = input_distribution(generator);

				scope->pre_network->activate(scope->init_pre_obs[rand_index]);

				double error = scope->init_pre_targets[rand_index] - scope->pre_network->output->acti_vals[0];

				scope->pre_network->backprop(error);
			}

			scope->init_pre_obs.clear();
			scope->init_pre_obs.shrink_to_fit();

			for (int h_index = 0; h_index < (int)scope->init_pre_obs.size(); h_index++) {
				scope->pre_network->activate(scope->init_pre_obs[h_index]);
				scope->init_post_targets[h_index] -= scope->pre_network->output->acti_vals[0];
			}

			scope->post_network = new LongNetwork(scope->init_post_obs[0].size());

			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = input_distribution(generator);

				scope->post_network->activate(scope->init_post_obs[rand_index]);

				double error = scope->init_post_targets[rand_index] - scope->post_network->output->acti_vals[0];

				scope->post_network->backprop(error);
			}

			scope->init_post_obs.clear();
			scope->init_post_obs.shrink_to_fit();

			/**
			 * - clear other data to take into account new
			 */
			for (int is_index = 0; is_index < (int)wrapper->solution->scopes.size(); is_index++) {
				Scope* o_scope = wrapper->solution->scopes[is_index];
				o_scope->init_pre_obs.clear();
				o_scope->init_pre_targets.clear();
				o_scope->init_post_obs.clear();
				o_scope->init_post_targets.clear();
			}
		}
	}
}
