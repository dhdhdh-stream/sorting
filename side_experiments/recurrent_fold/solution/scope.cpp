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
									 FoldHistory& early_exit_fold_history,
									 int& explore_exit_depth,
									 int& explore_exit_node_id,
									 FoldHistory& explore_fold_history,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	vector<double> local_state_vals(this->num_local_states, 0.0);

	early_exit_depth = -1;
	explore_exit_depth = -1;

	double rand_explore_val = randnorm();
	double explore_weight_scale_factor = 1.0;

	int curr_node_id = 0;
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
			action_node->activate(
				input_vals,
				local_state_vals,
				flat_vals,
				predicted_score,
				scale_factor,
				run_helper,
				node_history);
			history->node_histories.push_back(node_history);

			context_history->inpur_vals_snapshot = input_vals;
			context_history->local_state_vals_snapshot = local_state_vals;
			context_histories.push_back(context_history);

			rand_explore_val -= explore_weight_scale_factor*action_node->explore_weight;
			if (rand_explore_val < 0.0) {
				if (action_node->explore_weight > 0.05) {	// TODO: find systematic way of gating
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
						run_helper.explore_fold = action_node->explore_fold;
						run_helper.explore_fold_history = new FoldHistory(explore_fold);
						run_helper.explore_fold->explore_score_activate(
							input_vals,
							local_state_vals,
							predicted_score,
							scale_factor,
							context_iter,
							context_histories,
							run_helper,
							run_helper.explore_fold_history);

						history->explore_index = history->node_histories.size();

						if (action_node->explore_scope_context.size() == 1) {
							run_helper.explore_fold->explore_sequence_activate(
								input_vals,
								local_state_vals,
								flat_vals,
								predicted_score,
								scale_factor,
								run_helper,
								run_helper.explore_fold_history);

							curr_node_id = action_node->explore_next_node_id;
						} else {
							explore_exit_depth = (int)action_node->explore_scope_context.size() - 1;
							explore_exit_node_id = action_node->explore_next_node_id;
							break;
						}
					}
				}
			}

			curr_node_id = action_node->next_node_id;
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

			rand_explore_val -= explore_weight_scale_factor*scope_node->inner_explore_weight;

			if (rand_explore_val < 0.0
					&& scope_node->inner_explore_weight > 0.05) {	// TODO: find systematic way of gating
				int inner_early_exit_depth;
				int inner_early_exit_node_id;
				int inner_explore_exit_depth;
				int inner_explore_exit_node_id;
				ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
				scope_node->explore_on_path_activate(input_vals,
													 local_state_vals,
													 flat_vals,
													 predicted_score,
													 scale_factor,
													 scope_context,
													 node_context,
													 context_iter,
													 context_histories,
													 inner_early_exit_depth,
													 inner_early_exit_node_id,
													 inner_explore_exit_depth,
													 inner_explore_exit_node_id,
													 run_helper,
													 node_history);
				history->node_histories.push_back(node_history);

				history->explore_index = history->node_histories.size();

				if (inner_explore_exit_depth != -1) {
					if (inner_explore_exit_depth == 1) {
						run_helper.explore_fold->explore_sequence_activate(
							input_vals,
							local_state_vals,
							flat_vals,
							predicted_score,
							scale_factor,
							run_helper,
							run_helper.explore_fold_history);

						curr_node_id = inner_explore_exit_node_id;
					} else {
						explore_exit_depth = inner_explore_exit_depth-1;
						explore_exit_node_id = inner_explore_exit_node_id;
						break;
					}
				} else if (inner_early_exit_depth != -1) {
					if (inner_early_exit_depth == 1) {
						curr_node_id = inner_early_exit_node_id;
					} else {
						early_exit_depth = inner_early_exit_depth-1;
						early_exit_node_id = inner_early_exit_node_id;
						break;
					}
				} else {
					curr_node_id = scope_node->next_node_id;
				}
			} else {
				int inner_early_exit_depth;
				int inner_early_exit_node_id;
				ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
				scope_node->explore_off_path_activate(
					input_vals,
					local_state_vals,
					flat_vals,
					predicted_score,
					scale_factor,
					inner_early_exit_depth,
					inner_early_exit_node_id,
					run_helper,
					node_history);
				history->node_histories.push_back(node_history);

				if (inner_early_exit_depth != -1) {
					if (inner_early_exit_depth == 1) {
						curr_node_id = inner_early_exit_node_id;
					} else {
						early_exit_depth = inner_early_exit_depth-1;
						early_exit_node_id = inner_early_exit_node_id;
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
			BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
			int next_node_id = branch_node->activate(input_vals,
													 local_state_vals,
													 flat_vals,
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
				break;
			}
		} else {
			// this->nodes[curr_node_id]->type == NODE_TYPE_FOLD
			// Note: starting_score_network already passed on, fold in outer context
			FoldNode* fold_node = (FoldNode*)this->nodes[curr_node_id];

			int fold_exit_depth;
			int fold_exit_node_id;
			FoldNodeHistory* node_history = new FoldNodeHistory(fold_node);
			fold_node->activate(
				input_vals,
				local_state_vals,
				flat_vals,
				predicted_score,
				scale_factor,
				run_helper,
				node_history);
			history->node_histories.push_back(node_history);

			curr_node_id = fold_node->next_node_id;
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
		if (n_index == explore_index) {
			// TODO: redo logic, need to hit fold
			run_helper.explore_fold->explore_backprop();
			// no need to backprop further
			break;
		}

		if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)scope_history->node_histories[n_index]->node;
			action_node->backprop(input_errors,
								  local_state_errors,
								  target_val,
								  predicted_score,
								  scale_factor,
								  history->node_histories[n_index]);
		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_INNER_SCOPE) {

		} else if (scope_history->node_histories[n_index]->node->type == NODE_TYPE_BRANCH) {

		} else {
			// scope_history->node_histories[n_index]->node->type == NODE_TYPE_FOLD

		}
	}
}
