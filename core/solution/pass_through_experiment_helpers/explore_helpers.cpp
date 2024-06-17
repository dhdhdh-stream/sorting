#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SAMPLES_PER_ITER = 2;
#else
const int NUM_SAMPLES_PER_ITER = 40;
#endif /* MDEBUG */

void PassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
	for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
		Scope* scope = (Scope*)context[l_index].scope;
		ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
		if (scope->eval_network != NULL) {
			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
	}

	if (this->curr_info_scope == NULL) {
		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->curr_actions[s_index]->action);
			} else {
				this->curr_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}
		}

		curr_node = this->curr_exit_next_node;
	} else {
		double inner_score;
		this->curr_info_scope->activate(problem,
										run_helper,
										inner_score);

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (this->curr_is_negate) {
			if (inner_score >= 0.0) {
				is_branch = false;
			} else {
				is_branch = true;
			}
		} else {
			if (inner_score >= 0.0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
		#endif /* MDEBUG */

		if (is_branch) {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->curr_actions[s_index]->action);
				} else {
					this->curr_scopes[s_index]->explore_activate(
						problem,
						context,
						run_helper);
				}
			}

			curr_node = this->curr_exit_next_node;
		}
	}
}

void PassThroughExperiment::explore_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history);
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void PassThroughExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	bool is_next = false;
	if (run_helper.exceeded_limit) {
		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->curr_actions[s_index];
			} else {
				delete this->curr_scopes[s_index];
			}
		}

		this->curr_score = 0.0;
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		is_next = true;
	} else {
		PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double sum_score = 0.0;
			for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
				sum_score += history->predicted_scores[i_index][l_index];
			}
			sum_score += target_val - solution->average_score;
			double final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);

			this->curr_score += final_score - this->existing_average_score;
			this->sub_state_iter++;
		}

		this->state_iter++;
		if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->curr_score /= this->sub_state_iter;
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->curr_score > this->best_score) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_score = curr_score;
				this->best_info_scope = this->curr_info_scope;
				this->best_is_negate = this->curr_is_negate;
				this->best_step_types = this->curr_step_types;
				this->best_actions = this->curr_actions;
				this->best_scopes = this->curr_scopes;
				this->best_exit_next_node = this->curr_exit_next_node;

				this->curr_score = 0.0;
				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			} else {
				for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
					if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->curr_actions[s_index];
					} else {
						delete this->curr_scopes[s_index];
					}
				}

				this->curr_score = 0.0;
				this->curr_step_types.clear();
				this->curr_actions.clear();
				this->curr_scopes.clear();
			}

			is_next = true;
		}
	}

	if (is_next) {
		this->explore_iter++;
		if (this->explore_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_score != numeric_limits<double>::lowest()) {
			#else
			if (this->best_score >= 0.0) {
			#endif /* MDEBUG */
				// cout << "PassThrough" << endl;
				// cout << "this->scope_context->id: " << this->scope_context->id << endl;
				// cout << "this->node_context->id: " << this->node_context->id << endl;
				// cout << "this->is_branch: " << this->is_branch << endl;
				// cout << "new explore path:";
				// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				// 		cout << " " << this->best_actions[s_index]->action.move;
				// 	} else {
				// 		cout << " E";
				// 	}
				// }
				// cout << endl;

				// if (this->best_exit_next_node == NULL) {
				// 	cout << "this->best_exit_next_node->id: " << -1 << endl;
				// } else {
				// 	cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				// }

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = this->scope_context;
						this->best_actions[s_index]->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
					} else {
						this->best_scopes[s_index]->parent = this->scope_context;
						this->best_scopes[s_index]->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
					}
				}

				int exit_node_id;
				AbstractNode* exit_node;
				if (this->best_exit_next_node == NULL) {
					ActionNode* new_ending_node = new ActionNode();
					new_ending_node->parent = this->scope_context;
					new_ending_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_ending_node->action = Action(ACTION_NOOP);

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					this->ending_node = new_ending_node;

					exit_node_id = new_ending_node->id;
					exit_node = new_ending_node;
				} else {
					exit_node_id = this->best_exit_next_node->id;
					exit_node = this->best_exit_next_node;
				}

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					int next_node_id;
					AbstractNode* next_node;
					if (s_index == (int)this->best_step_types.size()-1) {
						next_node_id = exit_node_id;
						next_node = exit_node;
					} else {
						if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
							next_node_id = this->best_actions[s_index+1]->id;
							next_node = this->best_actions[s_index+1];
						} else {
							next_node_id = this->best_scopes[s_index+1]->id;
							next_node = this->best_scopes[s_index+1];
						}
					}

					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->next_node_id = next_node_id;
						this->best_actions[s_index]->next_node = next_node;
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						this->best_scopes[s_index]->next_node_id = next_node_id;
						this->best_scopes[s_index]->next_node = next_node;
					}
				}

				this->target_val_histories.reserve(NUM_DATAPOINTS);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			Scope* parent_scope = (Scope*)this->scope_context;

			vector<AbstractNode*> possible_exits;

			if (this->node_context->type == NODE_TYPE_ACTION
					&& ((ActionNode*)this->node_context)->next_node == NULL) {
				possible_exits.push_back(NULL);
			}

			AbstractNode* starting_node;
			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = info_branch_node->branch_next_node;
					} else {
						starting_node = info_branch_node->original_next_node;
					}
				}
				break;
			}

			parent_scope->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->curr_exit_next_node = possible_exits[random_index];

			this->curr_info_scope = get_existing_info_scope();
			uniform_int_distribution<int> negate_distribution(0, 1);
			this->curr_is_negate = negate_distribution(generator) == 0;

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
					ScopeNode* new_scope_node = create_existing(parent_scope);
					if (new_scope_node != NULL) {
						this->curr_step_types.push_back(STEP_TYPE_SCOPE);
						this->curr_actions.push_back(NULL);

						this->curr_scopes.push_back(new_scope_node);

						default_to_action = false;
					}
				}

				if (default_to_action) {
					this->curr_step_types.push_back(STEP_TYPE_ACTION);

					ActionNode* new_action_node = new ActionNode();
					new_action_node->action = problem_type->random_action();
					this->curr_actions.push_back(new_action_node);

					this->curr_scopes.push_back(NULL);
				}
			}

			this->state_iter = 0;
			this->sub_state_iter = 0;
		}
	}
}
