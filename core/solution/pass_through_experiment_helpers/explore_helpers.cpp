#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
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
#include "solution_set.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int INITIAL_NUM_TRUTH_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 2;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 2;
const int EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int INITIAL_NUM_TRUTH_PER_ITER = 5;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 25;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 100;
const int EXPLORE_ITERS = 100;
#endif /* MDEBUG */

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
	switch (this->score_type) {
	case SCORE_TYPE_TRUTH:
		history->predicted_scores.push_back(vector<double>());
		break;
	case SCORE_TYPE_ALL:
		history->predicted_scores.push_back(vector<double>(context.size()-1));
		for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
			ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;

			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
		break;
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
		bool is_positive;
		this->curr_info_scope->activate(problem,
										context,
										run_helper,
										is_positive);

		bool is_branch;
		if (this->curr_is_negate) {
			if (is_positive) {
				is_branch = false;
			} else {
				is_branch = true;
			}
		} else {
			if (is_positive) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}

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
		predicted_score = calc_score(scope_history) / (int)scope_history->callback_experiment_indexes.size();
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void PassThroughExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	bool is_fail = false;

	if (run_helper.exceeded_limit) {
		is_fail = true;
	} else {
		PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

		this->state_iter++;
		if (this->state_iter == INITIAL_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= INITIAL_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->curr_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == VERIFY_1ST_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= VERIFY_1ST_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->curr_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == VERIFY_2ND_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->curr_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		}

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double final_score;
			switch (this->score_type) {
			case SCORE_TYPE_TRUTH:
				final_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
				break;
			case SCORE_TYPE_ALL:
				{
					double sum_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
					for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
						sum_score += history->predicted_scores[i_index][l_index];
					}
					final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
				}
				break;
			}

			this->curr_score += final_score - this->existing_average_score;
			this->sub_state_iter++;

			if (this->sub_state_iter == INITIAL_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= INITIAL_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->curr_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == VERIFY_1ST_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= VERIFY_1ST_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->curr_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == VERIFY_2ND_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->curr_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			}
		}
	}

	bool is_next = false;
	if (is_fail) {
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
	} else if (this->sub_state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER
			&& this->state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
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
		this->best_pre_exit_node = this->curr_pre_exit_node;
		this->best_exit_next_node = this->curr_exit_next_node;

		this->curr_score = 0.0;
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		is_next = true;
	}

	if (is_next) {
		this->explore_iter++;
		if (this->explore_iter >= EXPLORE_ITERS) {
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
					exit_node_id = -1;
					exit_node = NULL;
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

				if (this->best_info_scope != NULL) {
					this->info_branch_node = new InfoBranchNode();
					this->info_branch_node->parent = this->scope_context;
					this->info_branch_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
				}

				this->target_val_histories.reserve(NUM_DATAPOINTS);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			Scope* parent_scope = (Scope*)this->scope_context;

			vector<AbstractNode*> possible_pre_exits;
			vector<AbstractNode*> possible_exits;
			parent_scope->random_exit_activate(
				this->node_context,
				this->is_branch,
				possible_pre_exits,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			// int random_index = distribution(generator);
			int random_index = 0;
			this->curr_pre_exit_node = possible_pre_exits[random_index];
			this->curr_exit_next_node = possible_exits[random_index];

			this->curr_info_scope = get_existing_info_scope(parent_scope);
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
