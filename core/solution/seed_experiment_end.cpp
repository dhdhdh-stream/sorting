#include "seed_experiment_end.h"

using namespace std;



void SeedExperimentEnd::activate(vector<ContextLayer>& context) {
	if (this->parent->state == SEED_EXPERIMENT_STATE_FIND_FILTER
			&& this->parent->sub_state_iter == -1) {
		/**
		 * - instance_count already updated in parent
		 */
		bool is_target = false;
		SeedExperimentCurrFilter* overall_history = (SeedExperimentCurrFilter*)run_helper.experiment_history;
		if (!overall_history->has_target) {
			double target_probability;
			if (overall_history->instance_count > this->average_instances_per_run) {
				target_probability = 0.5;
			} else {
				target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
			}
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < target_probability) {
				is_target = true;
			}
		}

		if (is_target) {
			overall_history->has_target = true;

			select_filter(this->parent->curr_filter_scope_context,
						  this->parent->curr_filter_node_context,
						  this->parent->curr_filter_is_branch,
						  this->parent,
						  context[context.size() - this->parent->scope_context.size()].scope_history);

			vector<pair<int,AbstractNode*>> possible_exits;
			gather_possible_exits(possible_exits,
								  this->parent->scope_context,
								  this->parent->node_context);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->parent->curr_filter_exit_depth = possible_exits[random_index].first;
			this->parent->curr_filter_exit_node = possible_exits[random_index].second;

			uniform_int_distribution<int> uniform_distribution(0, 2);
			geometric_distribution<int> geometric_distribution(0.5);
			int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

			uniform_int_distribution<int> new_scope_distribution(0, 3);
			uniform_int_distribution<int> random_scope_distribution(0, 3);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				ScopeNode* new_scope_node = NULL;
				if (new_scope_distribution(generator) == 0) {
					if (random_scope_distribution(generator) == 0) {
						uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
						Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
						new_scope_node = create_scope(scope,
													  run_helper);
					} else {
						new_scope_node = create_scope(this->parent->scope_context[0],
													  run_helper);
					}
				}
				if (new_scope_node != NULL) {
					this->parent->curr_filter_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
					this->parent->curr_filter_actions.push_back(NULL);
					this->parent->curr_filter_existing_scopes.push_back(NULL);

					this->parent->curr_filter_potential_scopes.push_back(new_scope_node);
				} else {
					ScopeNode* new_existing_scope_node = reuse_existing(problem);
					if (new_existing_scope_node != NULL) {
						this->parent->curr_filter_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
						this->parent->curr_filter_actions.push_back(NULL);

						this->parent->curr_filter_existing_scopes.push_back(new_existing_scope_node);

						this->parent->curr_filter_potential_scopes.push_back(NULL);
					} else {
						this->parent->curr_filter_step_types.push_back(STEP_TYPE_ACTION);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->action = problem->random_action();
						this->parent->curr_filter_actions.push_back(new_action_node);

						this->parent->curr_filter_existing_scopes.push_back(NULL);
						this->parent->curr_filter_potential_scopes.push_back(NULL);
					}
				}
			}
		}
	}
}
