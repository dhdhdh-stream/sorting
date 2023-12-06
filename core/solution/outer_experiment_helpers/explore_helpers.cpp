#include "outer_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXPLORE_ITERS = 2;
const int NUM_SAMPLES_PER_ITER = 2;
#else
const int EXPLORE_ITERS = 100;
const int NUM_SAMPLES_PER_ITER = 100;
#endif /* MDEBUG */

void OuterExperiment::explore_initial_activate(Problem* problem,
											   RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = NULL;
	context.back().node = NULL;

	// unused
	AbstractNode* curr_node = NULL;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.5);
	int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

	uniform_int_distribution<int> type_distribution(0, 1);
	uniform_int_distribution<int> root_distribution(0, 1);
	uniform_int_distribution<int> next_distribution(0, 1);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		if (type_distribution(generator) == 0) {
			this->curr_step_types.push_back(STEP_TYPE_ACTION);

			ActionNode* new_action_node = new ActionNode();
			new_action_node->action = problem->random_action();
			this->curr_actions.push_back(new_action_node);

			this->curr_potential_scopes.push_back(NULL);
			this->curr_root_scope_nodes.push_back(NULL);

			ActionNodeHistory* action_node_history = new ActionNodeHistory(new_action_node);
			new_action_node->activate(curr_node,
									  problem,
									  context,
									  exit_depth,
									  exit_node,
									  run_helper,
									  action_node_history);
			delete action_node_history;
		} else {
			if (root_distribution(generator) == 0) {
				this->curr_step_types.push_back(STEP_TYPE_ROOT);
				this->curr_actions.push_back(NULL);
				this->curr_potential_scopes.push_back(NULL);

				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->inner_scope = solution->root;
				this->curr_root_scope_nodes.push_back(new_scope_node);

				new_scope_node->is_loop = false;
				new_scope_node->continue_score_mod = 0.0;
				new_scope_node->halt_score_mod = 0.0;
				new_scope_node->max_iters = 0;

				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(new_scope_node);
				new_scope_node->activate(curr_node,
										 problem,
										 context,
										 exit_depth,
										 exit_node,
										 run_helper,
										 scope_node_history);
				delete scope_node_history;
			} else {
				this->curr_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
				this->curr_actions.push_back(NULL);

				PotentialScopeNode* new_potential_scope_node = create_scope(
					context,
					1,
					solution->root);
				this->curr_potential_scopes.push_back(new_potential_scope_node);

				this->curr_root_scope_nodes.push_back(NULL);

				PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(new_potential_scope_node);
				new_potential_scope_node->activate(problem,
												   context,
												   run_helper,
												   potential_scope_node_history);
				delete potential_scope_node_history;
			}
		}
	}
}

void OuterExperiment::explore_activate(Problem* problem,
									   RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = NULL;
	context.back().node = NULL;

	// unused
	AbstractNode* curr_node = NULL;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
		if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->curr_actions[s_index]);
			this->curr_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else if (this->curr_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->curr_potential_scopes[s_index]);
			this->curr_potential_scopes[s_index]->activate(problem,
														   context,
														   run_helper,
														   potential_scope_node_history);
			delete potential_scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_root_scope_nodes[s_index]);
			this->curr_root_scope_nodes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}
	}
}

void OuterExperiment::explore_backprop(double target_val) {
	this->curr_score += target_val - this->existing_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
		this->curr_score /= NUM_SAMPLES_PER_ITER;
		#if defined(MDEBUG) && MDEBUG
		if (true) {
		#else
		if (this->curr_score > this->best_score) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					delete this->best_potential_scopes[s_index];
				} else {
					delete this->best_root_scope_nodes[s_index];
				}
			}

			this->best_score = curr_score;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_potential_scopes = this->curr_potential_scopes;
			this->best_root_scope_nodes = this->curr_root_scope_nodes;

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_potential_scopes.clear();
			this->curr_root_scope_nodes.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else if (this->curr_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					delete this->curr_potential_scopes[s_index];
				} else {
					delete this->curr_root_scope_nodes[s_index];
				}
			}

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_potential_scopes.clear();
			this->curr_root_scope_nodes.clear();
		}

		this->state_iter++;
		this->sub_state_iter = 0;
		if (this->state_iter >= EXPLORE_ITERS) {
			double score_standard_deviation = sqrt(this->existing_score_variance);
			double score_improvement_t_score = this->best_score
				/ (score_standard_deviation / sqrt(NUM_SAMPLES_PER_ITER));

			// cout << "Outer" << endl;
			// cout << "this->best_surprise: " << this->best_score << endl;
			// cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (score_improvement_t_score > 1.645) {	// >95%
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->id = 1 + s_index;
					} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
						this->best_potential_scopes[s_index]->scope_node_placeholder = new ScopeNode();
						this->best_potential_scopes[s_index]->scope_node_placeholder->id = 1 + s_index;

						this->best_potential_scopes[s_index]->scope->id = solution->scope_counter;
						solution->scope_counter++;
					} else {
						this->best_root_scope_nodes[s_index]->id = 1 + s_index;
					}
				}

				// cout << "new explore path:";
				// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				// 		cout << " " << this->best_actions[s_index]->action.move;
				// 	} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
				// 		cout << " S";
				// 	} else {
				// 		cout << " R";
				// 	}
				// }
				// cout << endl;

				this->target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = OUTER_EXPERIMENT_STATE_MEASURE_NEW_SCORE;
				this->state_iter = 0;
			} else {
				this->best_score = 0.0;
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
						delete this->best_potential_scopes[s_index];
					} else {
						delete this->best_root_scope_nodes[s_index];
					}
				}
				this->best_step_types.clear();
				this->best_actions.clear();
				this->best_potential_scopes.clear();
				this->best_root_scope_nodes.clear();

				// leave this->state as OUTER_EXPERIMENT_STATE_EXPLORE
				this->state_iter = 0;
				this->sub_state_iter = 0;
			}

			// cout << endl;
		}
	}
}
