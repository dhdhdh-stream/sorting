#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "helpers.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

const int EXPLORE_ITERS = 1000;

const int EXPERIMENT_SURPRISE_THRESHOLD = 1.0;
/**
 * - if surprise isn't better than what can be expected from random fluctuation, don't bother
 */

void BranchExperiment::explore_activate(int& curr_node_id,
										Problem& problem,
										vector<ContextLayer>& context,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	double predicted_score = parent_scope->average_score;
	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			predicted_score += normalized * it->first->scale->weight;
		} else {
			predicted_score += it->second.val * it->first->scale->weight;
		}
	}
	history->existing_predicted_score = predicted_score;

	{
		// exit
		int new_exit_depth;
		int new_exit_node_id;
		random_exit(this->scope_context,
					this->node_context,
					new_exit_depth,
					new_exit_node_id);

		this->curr_exit_depth = new_exit_depth;
		this->curr_exit_node_id = new_exit_node_id;
	}

	context.push_back(ContextLayer());

	context.back().scope_id = this->new_scope_id;
	context.back().node_id = -1;

	{
		// new path
		geometric_distribution<int> geometric_distribution(0.3);
		uniform_int_distribution<int> allow_zero_distribution(0, 5);
		int new_num_steps;
		if (this->curr_exit_depth == 0
				&& this->curr_exit_node_id == curr_node_id) {
			new_num_steps = 1 + geometric_distribution(generator);
		} else {
			if (allow_zero_distribution(generator) == 0) {
				new_num_steps = geometric_distribution(generator);
			} else {
				new_num_steps = 1 + geometric_distribution(generator);
			}
		}
		
		uniform_int_distribution<int> type_distribution(0, 1);
		uniform_int_distribution<int> action_distribution(0, 2);
		uniform_int_distribution<int> direction_distribution(0, 1);
		uniform_int_distribution<int> next_distribution(0, 1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (type_distribution(generator) == 0
					|| (this->scope_context.back() == 0
						&& solution->scopes[0]->nodes.size() == 1)) {
				this->curr_step_types.push_back(STEP_TYPE_ACTION);
				this->curr_actions.push_back(new ActionNode());
				this->curr_actions.back()->action = Action(action_distribution(generator));
				this->curr_sequences.push_back(NULL);

				problem.perform_action(this->curr_actions.back()->action);
			} else {
				this->curr_step_types.push_back(STEP_TYPE_SEQUENCE);
				this->curr_actions.push_back(NULL);

				Sequence* new_sequence;
				while (true) {
					Scope* containing_scope;
					if (direction_distribution(generator) == 0) {
						// higher
						int context_index = (int)context.size() - (int)this->scope_context.size() - 1;
						/**
						 * - start from 1 above
						 *   - current scope comes from lower
						 */
						while (true) {
							if (context_index >= 0 && next_distribution(generator) == 0) {
								context_index--;
							} else {
								break;
							}
						}
						if (context_index == -1) {
							containing_scope = NULL;
						} else {
							containing_scope = solution->scopes[context[context_index].scope_id];
						}
					} else {
						// lower
						containing_scope = solution->scopes[this->scope_context[0]];
						while (true) {
							if (containing_scope->child_scopes.size() > 0 && next_distribution(generator) == 0) {
								uniform_int_distribution<int> child_distribution(0, (int)containing_scope->child_scopes.size()-1);
								containing_scope = containing_scope->child_scopes[child_distribution(generator)];
							} else {
								break;
							}
						}
					}

					Sequence* sequence;
					if (containing_scope == NULL) {
						sequence = create_root_sequence(problem,
														context,
														(int)this->scope_context.size()+1,
														run_helper);
					} else {
						sequence = create_sequence(problem,
												   context,
												   (int)this->scope_context.size()+1,
												   containing_scope,
												   run_helper);
					}

					bool should_retry = false;
					if (sequence->scope->nodes.size() == 0) {
						should_retry = true;
						/**
						 * - can be empty sequence if, e.g., start from branch node into exit
						 *   - in which case retry
						 */
					}
					bool has_non_stub_node = false;
					for (int n_index = 0; n_index < (int)sequence->scope->nodes.size(); n_index++) {
						bool is_non_stub = true;
						if (sequence->scope->nodes[n_index]->type == NODE_TYPE_BRANCH_STUB) {
							is_non_stub = false;
						} else if (sequence->scope->nodes[n_index]->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)sequence->scope->nodes[n_index];
							if (action_node->action.move == ACTION_NOOP) {
								is_non_stub = false;
							}
						}
						if (is_non_stub) {
							has_non_stub_node = true;
							break;
						}
					}
					if (!has_non_stub_node) {
						should_retry = true;
					}
					bool ends_with_non_stub_node = true;
					if (sequence->scope->nodes.size() > 0) {
						if (sequence->scope->nodes.back()->type == NODE_TYPE_BRANCH_STUB) {
							ends_with_non_stub_node = false;
						} else if (sequence->scope->nodes.back()->type == NODE_TYPE_ACTION) {
							ActionNode* action_node = (ActionNode*)sequence->scope->nodes.back();
							if (action_node->action.move == ACTION_NOOP) {
								ends_with_non_stub_node = false;
							}
						}
					}
					if (!ends_with_non_stub_node) {
						should_retry = true;
					}
					if (should_retry) {
						delete sequence;
					} else {
						new_sequence = sequence;
						break;
					}
				}
				this->curr_sequences.push_back(new_sequence);
			}
		}
	}

	context.pop_back();

	{
		if (this->curr_exit_depth == 0) {
			curr_node_id = this->curr_exit_node_id;
		} else {
			exit_depth = this->curr_exit_depth-1;
			exit_node_id = this->curr_exit_node_id;
		}
	}
}

void BranchExperiment::explore_backprop(double target_val,
										BranchExperimentHistory* history) {
	Scope* parent_scope = solution->scopes[this->scope_context[0]];
	double curr_surprise = (target_val - history->existing_predicted_score)
		/ parent_scope->average_misguess;
	if (curr_surprise > this->best_surprise) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->best_actions[s_index];
			} else {
				delete this->best_sequences[s_index];
			}
		}

		this->best_surprise = curr_surprise;
		this->best_step_types = this->curr_step_types;
		this->best_actions = this->curr_actions;
		this->best_sequences = this->curr_sequences;
		this->best_exit_depth = this->curr_exit_depth;
		this->best_exit_node_id = this->curr_exit_node_id;

		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_sequences.clear();
	} else {
		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->curr_actions[s_index];
			} else {
				delete this->curr_sequences[s_index];
			}
		}

		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_sequences.clear();
	}

	this->state_iter++;
	if (this->state_iter >= EXPLORE_ITERS) {
		cout << "this->best_surprise: " << this->best_surprise << endl;
		if (this->best_surprise > EXPERIMENT_SURPRISE_THRESHOLD) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_SEQUENCE) {
					this->best_sequences[s_index]->scope->id = solution->scope_counter;
					solution->scope_counter++;
				}
			}

			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.to_string();
				} else {
					cout << " S";
				}
			}
			cout << endl;

			this->state = BRANCH_EXPERIMENT_STATE_TRAIN_PRE;
			this->state_iter = 0;
		} else {
			this->state = BRANCH_EXPERIMENT_STATE_DONE;
		}
	}
}
