#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "return_node.h"
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
const int INITIAL_NUM_SAMPLES_PER_ITER = 50;
const int INITIAL_NUM_TRUTH_PER_ITER = 20;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 250;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 100;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 1000;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 400;
const int EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void PassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	if (this->state_iter == -1) {
		uniform_int_distribution<int> use_previous_location_distribution(0, 3);
		// if (use_previous_location_distribution(generator) == 0) {
		if (false) {
			uniform_int_distribution<int> location_distribution(0, context.back().location_history.size()-1);
			AbstractNode* previous_location = (*next(context.back().location_history.begin(), location_distribution(generator))).first;
			this->curr_previous_location = previous_location;
		} else {
			this->curr_previous_location = NULL;
		}

		int new_num_steps;
		uniform_int_distribution<int> loop_distribution(0, 4);
		// if (this->scope_context->id != 0
		// 		&& loop_distribution(generator) == 0) {
		if (false) {
			this->curr_is_loop = true;

			uniform_int_distribution<int> past_distribution(0, context.back().location_history.size()-1);
			this->curr_exit_next_node = (*next(context.back().location_history.begin(), past_distribution(generator))).first;

			uniform_int_distribution<int> uniform_distribution(0, 1);
			geometric_distribution<int> geometric_distribution(0.5);
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			this->curr_is_loop = false;

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
			case NODE_TYPE_RETURN:
				{
					ReturnNode* return_node = (ReturnNode*)this->node_context;
					starting_node = return_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->curr_exit_next_node = possible_exits[random_index];

			uniform_int_distribution<int> uniform_distribution(0, 1);
			geometric_distribution<int> geometric_distribution(0.5);
			if (random_index == 0) {
				new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
			} else {
				new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
			}
		}

		uniform_int_distribution<int> default_distribution(0, 3);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool default_to_action = true;
			if (default_distribution(generator) != 0) {
				ScopeNode* new_scope_node = create_existing();
				if (new_scope_node != NULL) {
					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
					this->curr_actions.push_back(NULL);

					this->curr_scopes.push_back(new_scope_node);

					this->curr_returns.push_back(NULL);

					default_to_action = false;
				}
			}

			if (default_to_action) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->curr_actions.push_back(new_action_node);

				this->curr_scopes.push_back(NULL);
				this->curr_returns.push_back(NULL);
			}
		}

		geometric_distribution<int> return_distribution(0.75);
		int num_returns = return_distribution(generator);
		for (int r_index = 0; r_index < num_returns; r_index++) {
			ReturnNode* new_return_node = new ReturnNode();
			uniform_int_distribution<int> location_distribution(0, context.back().location_history.size()-1);
			AbstractNode* previous_location = (*next(context.back().location_history.begin(), location_distribution(generator))).first;
			new_return_node->previous_location_id = previous_location->id;
			new_return_node->previous_location = previous_location;

			uniform_int_distribution<int> step_distribution(0, new_num_steps);
			int step_index = step_distribution(generator);
			this->curr_step_types.insert(this->curr_step_types.begin() + step_index, STEP_TYPE_RETURN);
			this->curr_actions.insert(this->curr_actions.begin() + step_index, NULL);
			this->curr_scopes.insert(this->curr_scopes.begin() + step_index, NULL);
			this->curr_returns.insert(this->curr_returns.begin() + step_index, new_return_node);
		}

		this->state_iter = 0;
		this->sub_state_iter = 0;
	}

	bool can_loop = true;
	if (this->curr_is_loop) {
		set<AbstractNode*>::iterator loop_start_it = context.back().loop_nodes_seen.find(this->branch_node);
		if (loop_start_it != context.back().loop_nodes_seen.end()) {
			can_loop = false;

			context.back().loop_nodes_seen.erase(loop_start_it);
		}
	}

	bool location_match = true;
	map<AbstractNode*, pair<int,int>>::iterator location_it;
	if (this->curr_previous_location != NULL) {
		location_it = context.back().location_history.find(this->curr_previous_location);
		if (location_it == context.back().location_history.end()) {
			location_match = false;
		}
	}

	if (location_match && can_loop) {
		history->instance_count++;

		if (this->curr_previous_location != NULL) {
			Minesweeper* minesweeper = (Minesweeper*)problem;
			minesweeper->current_x = location_it->second.first;
			minesweeper->current_y = location_it->second.second;
		}

		if (this->curr_is_loop) {
			context.back().loop_nodes_seen.insert(this->branch_node);
		}

		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				this->curr_actions[s_index]->explore_activate(
					problem,
					run_helper);
			} else if (this->curr_step_types[s_index] == STEP_TYPE_SCOPE) {
				this->curr_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			} else {
				this->curr_returns[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}
		}
	}

	curr_node = this->curr_exit_next_node;
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
			if (this->curr_score <= 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == VERIFY_1ST_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= VERIFY_1ST_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->curr_score <= 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == VERIFY_2ND_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->curr_score <= 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		}

		double final_score = (target_val - run_helper.result) / history->instance_count;
		for (int i_index = 0; i_index < history->instance_count; i_index++) {
			this->curr_score += final_score;
			this->sub_state_iter++;

			if (this->sub_state_iter == INITIAL_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= INITIAL_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->curr_score <= 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == VERIFY_1ST_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= VERIFY_1ST_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->curr_score <= 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == VERIFY_2ND_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->curr_score <= 0.0) {
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
			} else if (this->curr_step_types[s_index] == STEP_TYPE_SCOPE) {
				delete this->curr_scopes[s_index];
			} else {
				delete this->curr_returns[s_index];
			}
		}

		this->curr_score = 0.0;
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();
		this->curr_returns.clear();

		is_next = true;
	} else if (this->sub_state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER
			&& this->state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->best_actions[s_index];
			} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
				delete this->best_scopes[s_index];
			} else {
				delete this->best_returns[s_index];
			}
		}

		this->best_score = curr_score;
		this->best_previous_location = this->curr_previous_location;
		this->best_step_types = this->curr_step_types;
		this->best_actions = this->curr_actions;
		this->best_scopes = this->curr_scopes;
		this->best_returns = this->curr_returns;
		this->best_is_loop = this->curr_is_loop;
		this->best_exit_next_node = this->curr_exit_next_node;

		this->curr_score = 0.0;
		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();
		this->curr_returns.clear();

		is_next = true;
	}

	if (is_next) {
		this->explore_iter++;
		if (this->explore_iter >= EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_score != numeric_limits<double>::lowest()) {
			#else
			if (this->best_score > 0.0) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = this->scope_context;
						this->best_actions[s_index]->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						this->best_scopes[s_index]->parent = this->scope_context;
						this->best_scopes[s_index]->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
					} else {
						this->best_returns[s_index]->parent = this->scope_context;
						this->best_returns[s_index]->id = this->scope_context->node_counter;
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

					new_ending_node->action = Action(ACTION_NOOP, 0);

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
						} else if (this->best_step_types[s_index+1] == STEP_TYPE_SCOPE) {
							next_node_id = this->best_scopes[s_index+1]->id;
							next_node = this->best_scopes[s_index+1];
						} else {
							next_node_id = this->best_returns[s_index+1]->id;
							next_node = this->best_returns[s_index+1];
						}
					}

					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->next_node_id = next_node_id;
						this->best_actions[s_index]->next_node = next_node;
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						this->best_scopes[s_index]->next_node_id = next_node_id;
						this->best_scopes[s_index]->next_node = next_node;
					} else {
						this->best_returns[s_index]->next_node_id = next_node_id;
						this->best_returns[s_index]->next_node = next_node;
					}
				}

				cout << "PassThrough" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index]->action.move;
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						cout << " E";
					} else {
						cout << " R";
					}
				}
				cout << endl;

				cout << "this->best_score: " << this->best_score << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			this->state_iter = -1;
			this->sub_state_iter = -1;
		}
	}
}
