#include "scope.h"

using namespace std;

Scope::Scope(int num_input_states,
			 int num_local_states,
			 bool is_loop,
			 Network* continue_network,
			 Network* halt_network,
			 vector<AbstractNode*> nodes) {
	this->num_input_states = num_input_states;
	this->num_local_states = num_local_states;
	this->is_loop = is_loop;
	this->continue_network = continue_network;
	this->halt_network = halt_network;
	this->nodes = nodes;
}

Scope::~Scope() {
	if (this->continue_network != NULL) {
		delete this->continue_network;
	}

	if (this->halt_network != NULL) {
		delete this->halt_network;
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

// if early_exit_depth...
// ... = -1, no early exit
// ... = 0, early exit to here, jump to early_exit_node_id
// ... > 0, decrement and continue early exit
void Scope::explore_on_path_activate(vector<double>& input_vals,
									 vector<vector<double>>& flat_vals,
									 double& predicted_score,
									 double& scale_factor,
									 vector<int>& scope_context,
									 vector<int>& node_context,
									 vector<int>& context_iter,
									 vector<ContextHistory*>& context_histories,
									 int& early_exit_depth,
									 int& early_exit_node_id,
									 FoldHistory*& early_exit_fold_history,
									 int& explore_exit_depth,
									 int& explore_exit_node_id,
									 FoldHistory*& explore_exit_fold_history,
									 RunHelper& run_helper,
									 ScopeHistory* history) {
	vector<double> local_state_vals(this->num_local_states, 0.0);

	early_exit_depth = -1;
	explore_exit_depth = -1;

	double rand_explore_val = randnorm();
	double explore_weight_scale_factor = this->starting_explore_weight->weight;

	int curr_node_id = 0;
	FoldHistory* curr_fold_history = NULL;
	while (true) {
		if (curr_node_id == -1) {
			break;
		}

		if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

			ContextHistory* context_history = new ContextHistory();
			context_history->scope_id = this->id;
			context_history->node_id = curr_node_id;
			context_history->obs_snapshot = flat_vals.begin()[0];

			if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
				ActionNodeHistory* node_history = new ActionNodeHistory(action_node);
				action_node->explore_on_path_activate(
					local_state_vals,
					input_vals,
					flat_vals,
					predicted_score,
					scale_factor,
					run_helper,
					node_history);
				history->node_histories.push_back(node_history);
			} else {
				action_node->explore_on_path_activate(
					local_state_vals,
					input_vals,
					flat_vals,
					predicted_score,
					scale_factor,
					run_helper,
					NULL);
			}

			context_history->inpur_vals_snapshot = input_vals;
			context_history->local_state_vals_snapshot = local_state_vals;
			context_histories.push_back(context_history);

			rand_explore_val -= explore_weight_scale_factor*action_node->explore_weight;
			// if doesn't match, still pass explore weight on

			if (rand_explore_val < 0.0) {
				if (action_node->explore_weight > 0.05) {	// TODO: find systematic way of gating
					if (action_node->explore_fold != NULL) {
						bool matches_context = true;
						if (action_node->explore_scope_context.size() > scope_context.size()) {
							matches_context = false;
						} else {
							for (int c_index = 0; c_index < (int)action_node->explore_scope_context.size(); c_index++) {
								if (action_node->explore_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
										|| action_node->explore_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
									matches_context = false;
									break;
								}
							}
						}
						if (matches_context) {
							FoldHistory* fold_history = new FoldHistory(explore_fold);
							action_node->explore_fold->explore_on_path_score_activate(
								local_state_vals,
								input_vals,
								predicted_score,
								scale_factor,
								context_iter,
								context_histories,
								run_helper,
								fold_history);
							run_helper.explore_fold_history = explore_fold_history;

							if (action_node->explore_scope_context.size() == 1) {
								action_node->explore_fold->explore_on_path_sequence_activate(
									local_state_vals,
									input_vals,
									flat_vals,
									predicted_score,
									scale_factor,
									run_helper,
									fold_history);

								history->is_explore_sequence = true;

								curr_node_id = action_node->explore_next_node_id;
							} else {
								explore_exit_depth = (int)action_node->explore_scope_context.size() - 1;
								explore_exit_node_id = action_node->explore_next_node_id;
								explore_exit_fold_history = fold_history;
								break;
							}
						}
					} else {
						// new explore
					}
				}
			}

			curr_node_id = action_node->next_node_id;
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

			rand_explore_val -= explore_weight_scale_factor*scope_node->average_impact;
			// if doesn't match, still pass explore weight on

			if (rand_explore_val < 0.0
					&& scope_node->inner_explore_weight > 0.05) {	// TODO: find systematic way of gating
				scope_context.push_back(scope_node->inner_scope_id);
				node_context.push_back(curr_node_id);
				context_iter.push_back(context_histories.size());

				int inner_early_exit_depth;
				int inner_early_exit_node_id;
				FoldHistory* inner_early_exit_fold_history;
				int inner_explore_exit_depth;
				int inner_explore_exit_node_id;
				FoldHistory* inner_explore_exit_fold_history;
				ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
				scope_node->explore_on_path_activate(local_state_vals,
													 input_vals,
													 flat_vals,
													 predicted_score,
													 scale_factor,
													 scope_context,
													 node_context,
													 context_iter,
													 context_histories,
													 inner_early_exit_depth,
													 inner_early_exit_node_id,
													 inner_early_exit_fold_history,
													 inner_explore_exit_depth,
													 inner_explore_exit_node_id,
													 inner_explore_exit_fold_history,
													 run_helper,
													 node_history);
				history->node_histories.push_back(node_history);

				scope_context.pop_back();
				node_context.pop_back();
				context_iter.pop_back();

				if (inner_explore_exit_depth != -1) {
					if (inner_explore_exit_depth == 1) {
						Fold* fold = inner_explore_exit_fold_history->fold;
						fold->explore_on_path_sequence_activate(
							local_state_vals,
							input_vals,
							flat_vals,
							predicted_score,
							scale_factor,
							run_helper,
							inner_explore_exit_fold_history);

						curr_node_id = inner_explore_exit_node_id;
					} else {
						explore_exit_depth = inner_explore_exit_depth-1;
						explore_exit_node_id = inner_explore_exit_node_id;
						explore_exit_fold_history = inner_explore_exit_fold_history;
						break;
					}
				} else if (inner_early_exit_depth != -1) {
					if (inner_early_exit_depth == 1) {
						curr_node_id = inner_early_exit_node_id;
						curr_fold_history = inner_early_exit_fold_history;
					} else {
						early_exit_depth = inner_early_exit_depth-1;
						early_exit_node_id = inner_early_exit_node_id;
						early_exit_fold_history = inner_early_exit_fold_history;
						break;
					}
				} else {
					curr_node_id = scope_node->next_node_id;
				}
			} else {
				scope_context.push_back(scope_node->inner_scope_id);
				node_context.push_back(curr_node_id);
				context_iter.push_back(context_histories.size());

				int inner_early_exit_depth;
				int inner_early_exit_node_id;
				FoldHistory* inner_early_exit_fold_history;
				if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
					ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
					scope_node->explore_off_path_activate(
						local_state_vals,
						input_vals,
						flat_vals,
						predicted_score,
						scale_factor,
						scope_context,
						node_context,
						context_iter,
						context_histories,
						inner_early_exit_depth,
						inner_early_exit_node_id,
						inner_early_exit_fold_history,
						run_helper,
						node_history);
					history->node_histories.push_back(node_history);
				} else {
					scope_node->explore_off_path_activate(
						local_state_vals,
						input_vals,
						flat_vals,
						predicted_score,
						scale_factor,
						scope_context,
						node_context,
						context_iter,
						context_histories,
						inner_early_exit_depth,
						inner_early_exit_node_id,
						inner_early_exit_fold_history,
						run_helper,
						NULL);
				}

				scope_context.pop_back();
				node_context.pop_back();
				context_iter.pop_back();

				if (inner_early_exit_depth != -1) {
					if (inner_early_exit_depth == 1) {
						curr_node_id = inner_early_exit_node_id;
						curr_fold_history = inner_early_exit_fold_history;
					} else {
						early_exit_depth = inner_early_exit_depth-1;
						early_exit_node_id = inner_early_exit_node_id;
						early_exit_fold_history = inner_early_exit_fold_history;
						break;
					}
				} else {
					curr_node_id = scope_node->next_node_id;
				}
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

			int branch_exit_depth;
			int branch_exit_node_id;
			if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
				BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
				branch_node->explore_on_path_activate(local_state_vals,
													  input_vals,
													  flat_vals,
													  predicted_score,
													  scale_factor,
													  explore_weight_scale_factor,
													  scope_context,
													  node_context,
													  branch_exit_depth,
													  branch_exit_node_id,
													  run_helper,
													  node_history);
				history->node_histories.push_back(node_history);
			} else {
				branch_node->explore_on_path_activate(local_state_vals,
													  input_vals,
													  flat_vals,
													  predicted_score,
													  scale_factor,
													  explore_weight_scale_factor,
													  scope_context,
													  node_context,
													  branch_exit_depth,
													  branch_exit_node_id,
													  run_helper,
													  NULL);
			}

			// branch_exit_depth != -1
			if (branch_exit_depth == 0) {
				curr_node_id = branch_exit_node_id;
			} else {
				early_exit_depth = branch_exit_depth;
				early_exit_node_id = branch_exit_node_id;
				early_exit_fold_history = NULL;
				break;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)this->nodes[curr_node_id];

			int fold_exit_depth;
			int fold_exit_node_id;
			FoldHistory* fold_history;
			fold_score_node->explore_on_path_activate(
				local_state_vals,
				input_vals,
				predicted_score,
				scale_factor,
				context_iter,
				context_histories,
				fold_exit_depth,
				fold_exit_node_id,
				fold_history,
				run_helper);

			// fold_exit_depth != -1
			if (fold_exit_depth == 0) {
				curr_node_id = fold_exit_node_id;
				curr_fold_history = fold_history;
			} else {
				early_exit_depth = fold_exit_depth;
				early_exit_node_id = fold_exit_node_id;
				early_exit_fold_history = fold_history;
				break;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SEQUENCE) {
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)this->nodes[curr_node_id];

			if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
				FoldSequenceNodeHistory* node_history = new FoldSequenceNodeHistory(fold_sequence_node);
				fold_sequence_node->explore_on_path_activate(curr_fold_history,
															 local_state_vals,
															 input_vals,
															 flat_vals,
															 predicted_score,
															 scale_factor,
															 run_helper,
															 node_history);
				history->node_histories.push_back(node_history);
			} else {
				fold_sequence_node->explore_on_path_activate(curr_fold_history,
															 local_state_vals,
															 input_vals,
															 flat_vals,
															 predicted_score,
															 scale_factor,
															 run_helper,
															 NULL);
			}

			curr_node_id = fold_sequence_node->next_node_id;
			curr_fold_history = NULL;
		} else {
			// this->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
			PassThroughNode* pass_through_node = (PassThroughNode*)this->nodes[curr_node_id];

			// always goes local as can't care about context/branch

			curr_node_id = pass_through_node->next_node_id;
		}
	}

	// Note:
	// - don't explore at start as have no new information, so would essentially be random
	// - at end, can both explore and kick
	//   - exploring within keeps access to local state
	//   - kicking enables more possible sequences
}

void Scope::explore_on_path_backprop(vector<double>& input_errors,
									 double target_val,
									 double final_misguess,
									 double& predicted_score,
									 double& scale_factor,
									 RunHelper& run_helper,
									 ScopeHistory* history) {
	vector<double> local_state_errors(this->num_local_states, 0.0);

	for (int n_index = scope_history->node_histories.size()-1; n_index >= 0; n_index--) {
		if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)history->node_histories[n_index]->node;
			action_node->explore_on_path_backprop(local_state_errors,
												  input_errors,
												  target_val,
												  predicted_score,
												  scale_factor,
												  run_helper,
												  history->node_histories[n_index]);
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)history->node_histories[n_index]->node;
			scope_node->explore_on_path_backprop(local_state_errors,
												 input_errors,
												 target_val,
												 final_misguess,
												 predicted_score,
												 scale_factor,
												 run_helper,
												 history->node_histories[n_index]);
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)history->node_histories[n_index]->node;
			branch_node->explore_on_path_backprop(local_state_errors,
												  input_errors,
												  target_val,
												  final_misguess,
												  predicted_score,
												  scale_factor,
												  run_helper,
												  history->node_histories[n_index]);
		} else {
			// scope_history->node_histories[n_index]->node->type == NODE_TYPE_FOLD_SEQUENCE
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)history->node_histories[n_index]->node;
			fold_sequence_node->explore_on_path_backprop(local_state_errors,
														 input_errors,
														 target_val,
														 final_misguess,
														 predicted_score,
														 scale_factor,
														 run_helper,
														 history->node_histories[n_index]);
		}
		// can't be NODE_TYPE_FOLD_SCORE or NODE_TYPE_PASS_THROUGH
	}

	if (history->is_explore_sequence) {
		Fold* fold = run_helper.explore_fold_history->fold;
		fold->explore_on_path_backprop(local_state_errors,
									   input_errors,
									   target_val,
									   final_misguess,
									   predicted_score,
									   scale_factor,
									   run_helper.explore_fold_history);
	}
}

void Scope::update_activate(vector<double>& input_vals,
							vector<vector<double>>& flat_vals,
							double& predicted_score,
							double& scale_factor,
							vector<int>& scope_context,
							vector<int>& node_context,
							vector<int>& context_iter,
							vector<ContextHistory*>& context_histories,
							int& early_exit_depth,
							int& early_exit_node_id,
							FoldHistory*& early_exit_fold_history,
							RunHelper& run_helper,
							ScopeHistory* history) {
	vector<double> local_state_vals(this->num_local_states, 0.0);

	early_exit_depth = -1;

	double sum_impact = 0.0;
	double explore_weight_scale_factor = this->starting_explore_weight->weight;

	int curr_node_id = 0;
	FoldHistory* curr_fold_history = NULL;
	while (true) {
		if (curr_node_id == -1) {
			break;
		}

		if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

			ContextHistory* context_history = new ContextHistory();
			context_history->scope_id = this->id;
			context_history->node_id = curr_node_id;
			context_history->obs_snapshot = flat_vals.begin()[0];

			ActionNodeHistory* node_history = new ActionNodeHistory(action_node);
			action_node->update_activate(local_state_vals,
										 input_vals,
										 flat_vals,
										 predicted_score,
										 scale_factor,
										 sum_impact,	// TODO: account for scaling by scale_factor
										 explore_weight_scale_factor,
										 node_history);
			history->node_histories.push_back(node_history);

			context_history->inpur_vals_snapshot = input_vals;
			context_history->local_state_vals_snapshot = local_state_vals;
			context_histories.push_back(context_history);

			curr_node_id = action_node->next_node_id;
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
			scope_context.push_back(scope_node->inner_scope_id);
			node_context.push_back(curr_node_id);
			context_iter.push_back(context_histories.size());

			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

			int inner_early_exit_depth;
			int inner_early_exit_node_id;
			FoldHistory* inner_early_exit_fold_history;
			ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
			scope_node->update_activate(local_state_vals,
										input_vals,
										flat_vals,
										predicted_score,
										scale_factor,
										sum_impact,
										explore_weight_scale_factor,
										scope_context,
										node_context,
										context_iter,
										context_histories,
										inner_early_exit_depth,
										inner_early_exit_node_id,
										inner_early_exit_fold_history,
										run_helper,
										node_history);
			history->node_histories.push_back(node_history);

			scope_context.pop_back();
			node_context.pop_back();
			context_iter.pop_back();

			if (inner_early_exit_depth != -1) {
				if (inner_early_exit_depth == 1) {
					curr_node_id = inner_early_exit_node_id;
					curr_fold_history = inner_early_exit_fold_history;
				} else {
					early_exit_depth = inner_early_exit_depth-1;
					early_exit_node_id = inner_early_exit_node_id;
					early_exit_fold_history = inner_early_exit_fold_history;
					break;
				}
			} else {
				curr_node_id = scope_node->next_node_id;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

			int branch_exit_depth;
			int branch_exit_node_id;
			BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
			branch_node->update_activate(local_state_vals,
										 input_vals,
										 flat_vals,
										 predicted_score,
										 scale_factor,
										 explore_weight_scale_factor,
										 scope_context,
										 node_context,
										 branch_exit_depth,
										 branch_exit_node_id,
										 run_helper,
										 node_history);
			history->node_histories.push_back(node_history);

			// branch_exit_depth != -1
			if (branch_exit_depth == 0) {
				curr_node_id = branch_exit_node_id;
			} else {
				early_exit_depth = branch_exit_depth;
				early_exit_node_id = branch_exit_node_id;
				early_exit_fold_history = NULL;
				break;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)this->nodes[curr_node_id];

			int fold_exit_depth;
			int fold_exit_node_id;
			FoldHistory* fold_history;
			fold_score_node->update_activate(local_state_vals,
											 input_vals,
											 predicted_score,
											 scale_factor,
											 context_iter,
											 context_histories,
											 fold_exit_depth,
											 fold_exit_node_id,
											 fold_history,
											 run_helper);

			// fold_exit_depth != -1
			if (fold_exit_depth == 0) {
				curr_node_id = fold_exit_node_id;
				curr_fold_history = fold_history;
			} else {
				early_exit_depth = fold_exit_depth;
				early_exit_node_id = fold_exit_node_id;
				early_exit_fold_history = fold_history;
				break;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SEQUENCE) {
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)this->nodes[curr_node_id];

			FoldSequenceNodeHistory* node_history = new FoldSequenceNodeHistory(fold_sequence_node);
			fold_sequence_node->update_activate(curr_fold_history,
												local_state_vals,
												input_vals,
												flat_vals,
												predicted_score,
												scale_factor,
												run_helper,
												node_history);
			history->node_histories.push_back(node_history);

			curr_node_id = fold_sequence_node->next_node_id;
			curr_fold_history = NULL;
		} else {
			// this->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
			PassThroughNode* pass_through_node = (PassThroughNode*)this->nodes[curr_node_id];

			curr_node_id = pass_through_node->next_node_id;
		}
	}

	history->sum_impact = sum_impact;
	history->ending_explore_weight_scale_factor = explore_weight_scale_factor;
}

void Scope::update_backprop(double target_val,
							double final_misguess,
							double& predicted_score,
							double& scale_factor,
							ScopeHistory* history) {
	double sum_impact_error = 1.0 - history->sum_impact;
	double explore_weight_scale_factor = history->ending_explore_weight_scale_factor;
	double explore_weight_scale_factor_error = 0.0;

	for (int n_index = scope_history->node_histories.size()-1; n_index >= 0; n_index--) {
		if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)history->node_histories[n_index]->node;
			action_node->update_backprop(target_val,
										 final_misguess,
										 predicted_score,
										 scale_factor,
										 sum_impact_error,
										 explore_weight_scale_factor_error,
										 history->node_histories[n_index]);
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)history->node_histories[n_index]->node;
			scope_node->update_backprop(target_val,
										final_misguess,
										predicted_score,
										scale_factor,
										sum_impact_error,
										explore_weight_scale_factor_error,
										history->node_histories[n_index]);
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)history->node_histories[n_index]->node;
			branch_node->update_backprop(target_val,
										 final_misguess,
										 predicted_score,
										 scale_factor,
										 explore_weight_scale_factor,
										 explore_weight_scale_factor_error,
										 history->node_histories[n_index]);
		} else {
			// scope_history->node_histories[n_index]->node->type == NODE_TYPE_FOLD_SEQUENCE
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)history->node_histories[n_index]->node;
			fold_sequence_node->update_backprop(target_val,
												final_misguess,
												predicted_score,
												scale_factor,
												history->node_histories[n_index]);
		}
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->is_explore_sequence = false;
}
