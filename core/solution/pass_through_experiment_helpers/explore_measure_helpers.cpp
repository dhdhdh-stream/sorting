#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_SAMPLES_PER_ITER = 2;
#else
const int NUM_SAMPLES_PER_ITER = 20;
#endif /* MDEBUG */

void PassThroughExperiment::explore_measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	if (this->throw_id != -1) {
		run_helper.throw_id = -1;
	}

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
		} else if (this->curr_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_existing_scopes[s_index]);
			this->curr_existing_scopes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->curr_potential_scopes[s_index]);
			this->curr_potential_scopes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}

		if (run_helper.throw_id != -1) {
			this->curr_catch_throw_ids[s_index].insert(run_helper.throw_id);
			run_helper.throw_id = -1;
		}
	}

	if (this->curr_exit_throw_id != -1) {
		run_helper.throw_id = this->curr_exit_throw_id;
	} else {
		if (this->curr_exit_depth == 0) {
			curr_node = this->curr_exit_next_node;
		} else {
			exit_depth = this->curr_exit_depth-1;
			exit_node = this->curr_exit_next_node;
		}
	}
}

void PassThroughExperiment::explore_measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->curr_score += target_val - this->existing_average_score;

	this->sub_state_iter++;
	if (this->sub_state_iter >= NUM_SAMPLES_PER_ITER) {
		this->curr_score /= NUM_SAMPLES_PER_ITER;
		#if defined(MDEBUG) && MDEBUG
		/**
		 * - at least has a chance to not exceed limit
		 */
		if (!run_helper.exceeded_limit) {
		#else
		if (this->curr_score > this->best_score) {
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					delete this->best_existing_scopes[s_index];
				} else {
					delete this->best_potential_scopes[s_index]->scope;
					delete this->best_potential_scopes[s_index];
				}
			}

			this->best_score = curr_score;
			this->best_step_types = this->curr_step_types;
			this->best_actions = this->curr_actions;
			this->best_existing_scopes = this->curr_existing_scopes;
			this->best_potential_scopes = this->curr_potential_scopes;
			this->best_exit_depth = this->curr_exit_depth;
			this->best_exit_next_node = this->curr_exit_next_node;
			this->best_exit_throw_id = this->curr_exit_throw_id;
			this->best_catch_throw_ids = this->curr_catch_throw_ids;

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_existing_scopes.clear();
			this->curr_potential_scopes.clear();
			this->curr_catch_throw_ids.clear();
		} else {
			for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
				if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->curr_actions[s_index];
				} else if (this->curr_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					delete this->curr_existing_scopes[s_index];
				} else {
					delete this->curr_potential_scopes[s_index]->scope;
					delete this->curr_potential_scopes[s_index];
				}
			}

			this->curr_score = 0.0;
			this->curr_step_types.clear();
			this->curr_actions.clear();
			this->curr_potential_scopes.clear();
			this->curr_existing_scopes.clear();
			this->curr_catch_throw_ids.clear();
		}

		this->state_iter++;
		if (this->state_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
			#if defined(MDEBUG) && MDEBUG
			if (this->best_score != numeric_limits<double>::lowest()) {
			#else
			if (this->best_score >= 0.0) {
			#endif /* MDEBUG */
				// cout << "PassThrough" << endl;
				// cout << "this->scope_context:" << endl;
				// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				// 	cout << c_index << ": " << this->scope_context[c_index]->id << endl;
				// }
				// cout << "this->node_context:" << endl;
				// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				// 	if (this->node_context[c_index] == NULL) {
				// 		cout << c_index << ": -1" << endl;
				// 	} else {
				// 		cout << c_index << ": " << this->node_context[c_index]->id << endl;
				// 	}
				// }
				// cout << "this->is_branch: " << this->is_branch << endl;
				// cout << "this->throw_id: " << this->throw_id << endl;
				// cout << "new explore path:";
				// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				// 		cout << " " << this->best_actions[s_index]->action.move;
				// 	} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				// 		cout << " E";
				// 	} else {
				// 		cout << " P";
				// 	}
				// }
				// cout << endl;

				// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
				// if (this->best_exit_next_node == NULL) {
				// 	cout << "this->best_exit_next_node->id: " << -1 << endl;
				// } else {
				// 	cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				// }
				// cout << "this->best_exit_throw_id: " << this->best_exit_throw_id << endl;

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->parent = this->scope_context.back();
						this->best_actions[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;
					} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						this->best_existing_scopes[s_index]->parent = this->scope_context.back();
						this->best_existing_scopes[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;
					} else {
						this->best_potential_scopes[s_index]->parent = this->scope_context.back();
						this->best_potential_scopes[s_index]->id = this->scope_context.back()->node_counter;
						this->scope_context.back()->node_counter++;

						int new_scope_id = solution->scope_counter;
						solution->scope_counter++;
						this->best_potential_scopes[s_index]->scope->id = new_scope_id;

						for (map<int, AbstractNode*>::iterator it = this->best_potential_scopes[s_index]->scope->nodes.begin();
								it != this->best_potential_scopes[s_index]->scope->nodes.end(); it++) {
							if (it->second->type == NODE_TYPE_BRANCH) {
								BranchNode* branch_node = (BranchNode*)it->second;
								branch_node->scope_context_ids[0] = new_scope_id;
								for (int i_index = 0; i_index < (int)branch_node->input_scope_context_ids.size(); i_index++) {
									if (branch_node->input_scope_context_ids[i_index].size() > 0) {
										branch_node->input_scope_context_ids[i_index][0] = new_scope_id;
									}
								}
							}
						}
					}
				}

				int exit_node_id;
				AbstractNode* exit_node;
				if (this->best_exit_depth > 0
						|| this->best_exit_throw_id != -1) {
					ExitNode* new_exit_node = new ExitNode();
					new_exit_node->parent = this->scope_context.back();
					new_exit_node->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;

					new_exit_node->exit_depth = this->best_exit_depth;
					new_exit_node->next_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->best_exit_depth]->id;
					if (this->best_exit_next_node == NULL) {
						new_exit_node->next_node_id = -1;
					} else {
						new_exit_node->next_node_id = this->best_exit_next_node->id;
					}
					new_exit_node->next_node = this->best_exit_next_node;
					if (this->best_exit_throw_id == TEMP_THROW_ID) {
						new_exit_node->throw_id = solution->throw_counter;
						solution->throw_counter++;
					} else {
						new_exit_node->throw_id = this->best_exit_throw_id;
					}

					this->exit_node = new_exit_node;

					exit_node_id = new_exit_node->id;
					exit_node = new_exit_node;
				} else {
					if (this->best_exit_next_node == NULL) {
						exit_node_id = -1;
					} else {
						exit_node_id = this->best_exit_next_node->id;
					}
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
						} else if (this->best_step_types[s_index+1] == STEP_TYPE_EXISTING_SCOPE) {
							next_node_id = this->best_existing_scopes[s_index+1]->id;
							next_node = this->best_existing_scopes[s_index+1];
						} else {
							next_node_id = this->best_potential_scopes[s_index+1]->id;
							next_node = this->best_potential_scopes[s_index+1];
						}
					}

					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						this->best_actions[s_index]->next_node_id = next_node_id;
						this->best_actions[s_index]->next_node = next_node;
					} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						this->best_existing_scopes[s_index]->next_node_id = next_node_id;
						this->best_existing_scopes[s_index]->next_node = next_node;

						for (set<int>::iterator it = this->best_catch_throw_ids[s_index].begin();
								it != this->best_catch_throw_ids[s_index].end(); it++) {
							this->best_existing_scopes[s_index]->catch_ids[*it] = next_node_id;
							this->best_existing_scopes[s_index]->catches[*it] = next_node;
						}
					} else {
						this->best_potential_scopes[s_index]->next_node_id = next_node_id;
						this->best_potential_scopes[s_index]->next_node = next_node;

						for (set<int>::iterator it = this->best_catch_throw_ids[s_index].begin();
								it != this->best_catch_throw_ids[s_index].end(); it++) {
							this->best_potential_scopes[s_index]->catch_ids[*it] = next_node_id;
							this->best_potential_scopes[s_index]->catches[*it] = next_node;
						}
					}
				}

				this->o_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		} else {
			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE_CREATE;
		}
	}
}
