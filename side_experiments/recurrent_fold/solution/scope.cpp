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
void Scope::activate(vector<double>& input_vals,
					 vector<vector<double>>& flat_vals,
					 double& predicted_score,
					 double& scale_factor,
					 double& sum_impact,
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
			action_node->activate(local_state_vals,
								  input_vals,
								  flat_vals,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  node_history);
			history->node_histories.push_back(node_history);

			context_history->inpur_vals_snapshot = input_vals;
			context_history->local_state_vals_snapshot = local_state_vals;
			context_histories.push_back(context_history);

			sum_impact += action_node->average_impact;

			if (run_helper.explore_phase == EXPLORE_PHASE_NONE
					&& randuni() < action_node->average_impact/action_node->average_sum_impact
					&& action_node->average_impact/action_node->average_sum_impact > 0.05) {	// TODO: find systematic way of gating
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
						FoldHistory* fold_history = new FoldHistory(action_node->explore_fold);
						action_node->explore_fold->explore_score_activate(
							local_state_vals,
							input_vals,
							predicted_score,
							scale_factor,
							context_iter,
							context_histories,
							fold_history);
						history->explore_index = history->node_histories.size();
						history->explore_fold_history = fold_history;

						run_helper.explore_phase = EXPLORE_PHASE_FLAT;

						if (action_node->explore_exit_depth == 0) {
							action_node->explore_fold->explore_sequence_activate(
								local_state_vals,
								input_vals,
								flat_vals,
								predicted_score,
								scale_factor,
								run_helper,
								fold_history);

							curr_node_id = action_node->explore_next_node_id;
						} else {
							explore_exit_depth = action_node->explore_exit_depth;
							explore_exit_node_id = action_node->explore_next_node_id;
							explore_exit_fold_history = fold_history;
							break;
						}
					}
				} else {
					// new explore
				}
			}

			curr_node_id = action_node->next_node_id;
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

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
			scope_node->activate(local_state_vals,
								 input_vals,
								 flat_vals,
								 predicted_score,
								 scale_factor,
								 sum_impact,
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
				sum_impact += scope_node->average_impact;

				if (run_helper.explore_phase == EXPLORE_PHASE_NONE
						&& randuni() < scope_node->average_impact/scope_node->average_sum_impact
						&& scope_node->average_impact/scope_node->average_sum_impact > 0.05) {	// TODO: find systematic way of gating
					if (scope_node->explore_fold != NULL) {
						bool matches_context = true;
						if (scope_node->explore_scope_context.size() > scope_context.size()) {
							matches_context = false;
						} else {
							for (int c_index = 0; c_index < (int)scope_node->explore_scope_context.size(); c_index++) {
								if (scope_node->explore_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
										|| scope_node->explore_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
									matches_context = false;
									break;
								}
							}
						}
						if (matches_context) {
							FoldHistory* fold_history = new FoldHistory(scope_node->explore_fold);
							scope_node->explore_fold->explore_score_activate(
								local_state_vals,
								input_vals,
								predicted_score,
								scale_factor,
								context_iter,
								context_histories,
								fold_history);
							history->explore_index = history->node_histories.size();
							history->explore_fold_history = fold_history;

							run_helper.explore_phase = EXPLORE_PHASE_FLAT;

							if (scope_node->explore_exit_depth == 0) {
								scope_node->explore_fold->explore_sequence_activate(
									local_state_vals,
									input_vals,
									flat_vals,
									predicted_score,
									scale_factor,
									run_helper,
									fold_history);

								curr_node_id = scope_node->explore_next_node_id;
							} else {
								explore_exit_depth = scope_node->explore_exit_depth;
								explore_exit_node_id = scope_node->explore_next_node_id;
								explore_exit_fold_history = fold_history;
								break;
							}
						}
					} else {
						// new explore
					}
				}

				curr_node_id = scope_node->next_node_id;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

			int branch_exit_depth;
			int branch_exit_node_id;
			BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
			branch_node->activate(local_state_vals,
								  input_vals,
								  predicted_score,
								  scale_factor,
								  scope_context,
								  node_context,
								  branch_exit_depth,
								  branch_exit_node_id,
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
			fold_score_node->activate(local_state_vals,
									  input_vals,
									  predicted_score,
									  scale_factor,
									  scope_context,
									  node_context,
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
			fold_sequence_node->activate(curr_fold_history,
										 local_state_vals,
										 input_vals,
										 flat_vals,
										 predicted_score,
										 scale_factor,
										 run_helper,
										 node_history);
			// don't worry about impact in fold
			history->node_histories.push_back(node_history);

			curr_node_id = fold_sequence_node->next_node_id;
			curr_fold_history = NULL;
		} else {
			// this->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
			PassThroughNode* pass_through_node = (PassThroughNode*)this->nodes[curr_node_id];

			// always goes local as can't care about context/branch

			curr_node_id = pass_through_node->next_node_id;
		}
	}
}

void Scope::backprop(vector<double>& input_errors,
					 double target_val,
					 double final_misguess,
					 double final_sum_impact,
					 double& predicted_score,
					 double& scale_factor,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	vector<double> local_state_errors;
	if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
		local_state_errors = vector<double>(this->num_local_states, 0.0);
	}

	for (int n_index = scope_history->node_histories.size()-1; n_index >= 0; n_index--) {
		if (history->explore_index == n_index) {
			Fold* fold = history->explore_fold_history->fold;
			fold->explore_backprop(local_state_errors,
								   input_errors,
								   target_val,
								   final_misguess,
								   predicted_score,
								   scale_factor,
								   history->explore_fold_history);

			run_helper.explore_phase == EXPLORE_PHASE_FLAT_BACKPROP_DONE;
			break;
		}

		if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)history->node_histories[n_index]->node;
			action_node->backprop(local_state_errors,
								  input_errors,
								  target_val,
								  final_misguess,
								  final_sum_impact,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  history->node_histories[n_index]);
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)history->node_histories[n_index]->node;
			scope_node->backprop(local_state_errors,
								 input_errors,
								 target_val,
								 final_misguess,
								 final_sum_impact,
								 predicted_score,
								 scale_factor,
								 run_helper,
								 history->node_histories[n_index]);

			if (run_helper.explore_phase == EXPLORE_PHASE_FLAT_BACKPROP_DONE) {
				break;
			}
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)history->node_histories[n_index]->node;
			branch_node->backprop(local_state_errors,
								  input_errors,
								  target_val,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  history->node_histories[n_index]);
		} else {
			// scope_history->node_histories[n_index]->node->type == NODE_TYPE_FOLD_SEQUENCE
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)history->node_histories[n_index]->node;
			fold_sequence_node->backprop(local_state_errors,
										 input_errors,
										 target_val,
										 predicted_score,
										 scale_factor,
										 run_helper,
										 history->node_histories[n_index]);
		}
		// can't be NODE_TYPE_FOLD_SCORE or NODE_TYPE_PASS_THROUGH
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->explore_index = -1;
	this->explore_fold_history = NULL;
}
