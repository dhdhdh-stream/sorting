#include "new_info_experiment.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 5;
#else
const int EXPLORE_ITERS = 500;
#endif /* MDEBUG */

void NewInfoExperiment::explore_sequence_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	run_helper.num_decisions++;

	history->instance_count++;

	bool is_target = false;
	if (!history->has_target) {
		double target_probability;
		if (history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		history->has_target = true;

		vector<AbstractNode*> possible_exits;

		if (this->node_context->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context)->next_node == NULL) {
			possible_exits.push_back(NULL);
		}

		AbstractNode* starting_node;
		if (this->node_context->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		} else if (this->node_context->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			starting_node = scope_node->next_node;
		} else {
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->curr_sequence_exit_next_node = possible_exits[random_index];

		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		if (random_index == 0) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		uniform_int_distribution<int> default_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			if (default_distribution(generator) != 0) {
				ScopeNode* new_scope_node = create_existing();
				if (new_scope_node != NULL) {
					this->curr_sequence_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_sequence_actions.push_back(NULL);

					this->curr_sequence_scopes.push_back(new_scope_node);

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_sequence_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_sequence_actions.push_back(new_action_node);

				this->curr_sequence_scopes.push_back(NULL);
			}
		}

		for (int s_index = 0; s_index < (int)this->curr_sequence_step_types.size(); s_index++) {
			if (this->curr_sequence_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->curr_sequence_actions[s_index]->action);
			} else {
				this->curr_sequence_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}
		}

		curr_node = this->curr_sequence_exit_next_node;
	}
}

void NewInfoExperiment::explore_sequence_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	if (history->has_target) {
		double curr_surprise = target_val - this->existing_average_score;

		bool select = false;
		if (this->explore_type == EXPLORE_TYPE_BEST) {
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (curr_surprise > this->best_sequence_surprise) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_sequence_step_types.size(); s_index++) {
					if (this->best_sequence_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_sequence_actions[s_index];
					} else {
						delete this->best_sequence_scopes[s_index];
					}
				}

				this->best_sequence_surprise = curr_surprise;
				this->best_sequence_step_types = this->curr_sequence_step_types;
				this->best_sequence_actions = this->curr_sequence_actions;
				this->best_sequence_scopes = this->curr_sequence_scopes;
				this->best_sequence_exit_next_node = this->curr_sequence_exit_next_node;

				this->curr_sequence_step_types.clear();
				this->curr_sequence_actions.clear();
				this->curr_sequence_scopes.clear();
			} else {
				for (int s_index = 0; s_index < (int)this->curr_sequence_step_types.size(); s_index++) {
					if (this->curr_sequence_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_sequence_actions[s_index];
					} else {
						delete this->curr_sequence_scopes[s_index];
					}
				}

				this->curr_sequence_step_types.clear();
				this->curr_sequence_actions.clear();
				this->curr_sequence_scopes.clear();
			}

			if (this->state_iter == EXPLORE_ITERS-1
					&& this->best_sequence_surprise > 0.0) {
				select = true;
			}
		} else {
			// this->explore_type == EXPLORE_TYPE_GOOD
			#if defined(MDEBUG) && MDEBUG
			if (!run_helper.exceeded_limit) {
			#else
			if (curr_surprise >= this->existing_score_standard_deviation) {
			#endif /* MDEBUG */
				this->best_sequence_surprise = curr_surprise;
				this->best_sequence_step_types = this->curr_sequence_step_types;
				this->best_sequence_actions = this->curr_sequence_actions;
				this->best_sequence_scopes = this->curr_sequence_scopes;
				this->best_sequence_exit_next_node = this->curr_sequence_exit_next_node;

				this->curr_sequence_step_types.clear();
				this->curr_sequence_actions.clear();
				this->curr_sequence_scopes.clear();

				select = true;
			} else {
				for (int s_index = 0; s_index < (int)this->curr_sequence_step_types.size(); s_index++) {
					if (this->curr_sequence_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_sequence_actions[s_index];
					} else {
						delete this->curr_sequence_scopes[s_index];
					}
				}

				this->curr_sequence_step_types.clear();
				this->curr_sequence_actions.clear();
				this->curr_sequence_scopes.clear();
			}
		}

		if (select) {
			this->info_score = 0.0;
			this->new_info_subscope = create_new_info_scope();

			uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
			this->num_instances_until_target = 1 + until_distribution(generator);

			this->i_scope_histories.reserve(NUM_DATAPOINTS);
			this->i_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE_INFO;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
