#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "fold_score_node.h"
#include "fold_sequence_node.h"
#include "fold_to_nodes.h"
#include "globals.h"
#include "pass_through_node.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

Scope::Scope(int num_local_states,
			 int num_input_states,
			 bool is_loop,
			 StateNetwork* continue_network,
			 StateNetwork* halt_network,
			 vector<AbstractNode*> nodes) {
	this->num_local_states = num_local_states;
	this->num_input_states = num_input_states;
	this->is_loop = is_loop;
	this->continue_network = continue_network;
	this->halt_network = halt_network;
	this->nodes = nodes;

	for (int n_index = 0; n_index < (int)this->nodes.size()-1; n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			action_node->next_node_id = n_index+1;
		} else {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			scope_node->next_node_id = n_index+1;
		}
	}
	if (this->nodes.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes.back();
		action_node->next_node_id = -1;
	} else {
		ScopeNode* scope_node = (ScopeNode*)this->nodes.back();
		scope_node->next_node_id = -1;
	}
}

Scope::Scope(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string num_local_states_line;
	getline(input_file, num_local_states_line);
	this->num_local_states = stoi(num_local_states_line);

	string num_input_states_line;
	getline(input_file, num_input_states_line);
	this->num_input_states = stoi(num_input_states_line);

	// TODO: loops
	string is_loop_line;
	getline(input_file, is_loop_line);
	this->is_loop = stoi(is_loop_line);

	if (this->is_loop) {
		ifstream continue_network_save_file;
		continue_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue.txt");
		this->continue_network = new StateNetwork(continue_network_save_file);
		continue_network_save_file.close();

		ifstream halt_network_save_file;
		halt_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt.txt");
		this->halt_network = new StateNetwork(halt_network_save_file);
		halt_network_save_file.close();
	} else {
		this->continue_network = NULL;
		this->halt_network = NULL;
	}

	string num_nodes_line;
	getline(input_file, num_nodes_line);
	int num_nodes = stoi(num_nodes_line);
	for (int n_index = 0; n_index < num_nodes; n_index++) {
		string node_type_line;
		getline(input_file, node_type_line);
		int node_type = stoi(node_type_line);

		ifstream node_save_file;
		node_save_file.open("saves/node_" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		if (node_type == NODE_TYPE_ACTION) {
			ActionNode* action_node = new ActionNode(node_save_file,
													 this->id,
													 n_index);
			this->nodes.push_back(action_node);
		} else if (node_type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = new ScopeNode(node_save_file,
												  this->id,
												  n_index);
			this->nodes.push_back(scope_node);
		} else if (node_type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = new BranchNode(node_save_file,
													 this->id,
													 n_index);
			this->nodes.push_back(branch_node);
		} else if (node_type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = new FoldScoreNode(node_save_file,
															   this->id,
															   n_index);
			this->nodes.push_back(fold_score_node);
		} else if (node_type == NODE_TYPE_FOLD_SEQUENCE) {
			FoldSequenceNode* fold_sequence_node = new FoldSequenceNode(node_save_file,
																		this->id,
																		n_index);
			this->nodes.push_back(fold_sequence_node);
		} else {
			// node_type == NODE_TYPE_PASS_THROUGH
			PassThroughNode* pass_through_node = new PassThroughNode(node_save_file,
																	 this->id,
																	 n_index);
			this->nodes.push_back(pass_through_node);
		}
		node_save_file.close();
	}
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
					 vector<ScopeHistory*>& context_histories,
					 int& early_exit_depth,
					 int& early_exit_node_id,
					 FoldHistory*& early_exit_fold_history,
					 int& explore_exit_depth,
					 int& explore_exit_node_id,
					 FoldHistory*& explore_exit_fold_history,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	// TODO: check recursive depth and exit if needed

	vector<double> local_state_vals(this->num_local_states, 0.0);

	scope_context.push_back(this->id);
	node_context.push_back(-1);
	context_histories.push_back(history);

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

			ActionNodeHistory* node_history = new ActionNodeHistory(action_node,
																	curr_node_id);
			action_node->activate(local_state_vals,
								  input_vals,
								  flat_vals,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  node_history);
			history->node_histories.push_back(node_history);

			sum_impact += action_node->average_impact;

			// if (run_helper.explore_phase == EXPLORE_PHASE_NONE
			// 		&& randuni() < action_node->average_impact/action_node->average_sum_impact
			// 		&& action_node->average_impact/action_node->average_sum_impact > 0.05) {	// TODO: find systematic way of gating
			if (run_helper.explore_phase == EXPLORE_PHASE_NONE) {
				if (action_node->explore_fold != NULL) {
					bool matches_context = true;
					if (action_node->explore_scope_context.size() > scope_context.size()) {
						matches_context = false;
					} else {
						// special case first scope context
						if (action_node->explore_scope_context[0] != scope_context.back()) {
							matches_context = false;
						} else {
							for (int c_index = 1; c_index < (int)action_node->explore_scope_context.size(); c_index++) {
								if (action_node->explore_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
										|| action_node->explore_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
									matches_context = false;
									break;
								}
							}
						}
					}
					if (matches_context) {
						run_helper.explore_phase = EXPLORE_PHASE_FLAT;

						FoldHistory* fold_history = new FoldHistory(action_node->explore_fold);
						action_node->explore_fold->explore_score_activate(
							local_state_vals,
							input_vals,
							predicted_score,
							scale_factor,
							context_histories,
							run_helper,
							fold_history);
						history->explore_fold_history = fold_history;

						if (action_node->explore_exit_depth == 0) {
							action_node->explore_fold->explore_sequence_activate(
								local_state_vals,
								input_vals,
								flat_vals,
								predicted_score,
								scale_factor,
								run_helper,
								fold_history);
							history->explore_index = (int)history->node_histories.size();

							curr_node_id = action_node->explore_next_node_id;
							continue;
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

			node_context.back() = curr_node_id;

			int inner_early_exit_depth;
			int inner_early_exit_node_id;
			FoldHistory* inner_early_exit_fold_history;
			int inner_explore_exit_depth;
			int inner_explore_exit_node_id;
			FoldHistory* inner_explore_exit_fold_history;
			ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node,
																  curr_node_id);
			scope_node->activate(local_state_vals,
								 input_vals,
								 flat_vals,
								 predicted_score,
								 scale_factor,
								 sum_impact,
								 scope_context,
								 node_context,
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

			node_context.back() = -1;

			if (inner_explore_exit_depth != -1) {
				if (inner_explore_exit_depth == 1) {
					Fold* fold = inner_explore_exit_fold_history->fold;
					fold->explore_sequence_activate(
						local_state_vals,
						input_vals,
						flat_vals,
						predicted_score,
						scale_factor,
						run_helper,
						inner_explore_exit_fold_history);
					history->explore_index = (int)history->node_histories.size();

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
							// special case first scope context
							if (scope_node->explore_scope_context[0] != scope_context.back()) {
								matches_context = false;
							} else {
								for (int c_index = 1; c_index < (int)scope_node->explore_scope_context.size(); c_index++) {
									if (scope_node->explore_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
											|| scope_node->explore_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
										matches_context = false;
										break;
									}
								}
							}
						}
						if (matches_context) {
							run_helper.explore_phase = EXPLORE_PHASE_FLAT;

							FoldHistory* fold_history = new FoldHistory(scope_node->explore_fold);
							scope_node->explore_fold->explore_score_activate(
								local_state_vals,
								input_vals,
								predicted_score,
								scale_factor,
								context_histories,
								run_helper,
								fold_history);
							history->explore_fold_history = fold_history;

							if (scope_node->explore_exit_depth == 0) {
								scope_node->explore_fold->explore_sequence_activate(
									local_state_vals,
									input_vals,
									flat_vals,
									predicted_score,
									scale_factor,
									run_helper,
									fold_history);
								history->explore_index = (int)history->node_histories.size();

								curr_node_id = scope_node->explore_next_node_id;
								continue;
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
			BranchNodeHistory* node_history = new BranchNodeHistory(branch_node,
																	curr_node_id);
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
			FoldHistory* fold_exit_fold_history;
			FoldScoreNodeHistory* node_history = new FoldScoreNodeHistory(fold_score_node,
																		  curr_node_id);
			fold_score_node->activate(local_state_vals,
									  input_vals,
									  predicted_score,
									  scale_factor,
									  scope_context,
									  node_context,
									  context_histories,
									  fold_exit_depth,
									  fold_exit_node_id,
									  fold_exit_fold_history,
									  run_helper,
									  node_history);
			history->node_histories.push_back(node_history);

			// fold_exit_depth != -1
			if (fold_exit_depth == 0) {
				curr_node_id = fold_exit_node_id;
				curr_fold_history = fold_exit_fold_history;
			} else {
				early_exit_depth = fold_exit_depth;
				early_exit_node_id = fold_exit_node_id;
				early_exit_fold_history = fold_exit_fold_history;
				break;
			}
		} else if (this->nodes[curr_node_id]->type == NODE_TYPE_FOLD_SEQUENCE) {
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)this->nodes[curr_node_id];

			FoldSequenceNodeHistory* node_history = new FoldSequenceNodeHistory(fold_sequence_node,
																				curr_node_id);
			fold_sequence_node->activate(curr_fold_history,
										 local_state_vals,
										 input_vals,
										 flat_vals,
										 predicted_score,
										 scale_factor,
										 sum_impact,
										 run_helper,
										 node_history);
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

	scope_context.pop_back();
	node_context.pop_back();
	context_histories.pop_back();
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

	if (history->explore_index == (int)history->node_histories.size()) {
		backprop_explore_fold_helper(local_state_errors,
									 input_errors,
									 target_val,
									 final_misguess,
									 predicted_score,
									 scale_factor,
									 run_helper,
									 history);
		return;
	}

	for (int h_index = (int)history->node_histories.size()-1; h_index >= 0; h_index--) {
		if (this->nodes[history->node_histories[h_index]->scope_index]
				!= history->node_histories[h_index]->node) {
			// node replaced, mainly for fold nodes

			// skip processing history, should only have minor temporary impact

			continue;
		}

		if (history->explore_index == h_index) {
			backprop_explore_fold_helper(local_state_errors,
										 input_errors,
										 target_val,
										 final_misguess,
										 predicted_score,
										 scale_factor,
										 run_helper,
										 history);
			break;
		}

		if (history->node_histories[h_index]->node->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)history->node_histories[h_index]->node;
			action_node->backprop(local_state_errors,
								  input_errors,
								  target_val,
								  final_misguess,
								  final_sum_impact,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  (ActionNodeHistory*)history->node_histories[h_index]);
		} else if (history->node_histories[h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)history->node_histories[h_index]->node;
			scope_node->backprop(local_state_errors,
								 input_errors,
								 target_val,
								 final_misguess,
								 final_sum_impact,
								 predicted_score,
								 scale_factor,
								 run_helper,
								 (ScopeNodeHistory*)history->node_histories[h_index]);

			if (run_helper.explore_phase == EXPLORE_PHASE_FLAT_BACKPROP_DONE) {
				break;
			}
		} else if (history->node_histories[h_index]->node->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)history->node_histories[h_index]->node;
			branch_node->backprop(local_state_errors,
								  input_errors,
								  target_val,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  (BranchNodeHistory*)history->node_histories[h_index]);
		} else if (history->node_histories[h_index]->node->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)history->node_histories[h_index]->node;
			fold_score_node->backprop(local_state_errors,
									  input_errors,
									  target_val,
									  predicted_score,
									  scale_factor,
									  run_helper,
									  (FoldScoreNodeHistory*)history->node_histories[h_index]);

			if (fold_score_node->fold->state == FOLD_STATE_DONE) {
				int new_outer_state_size = fold_score_node->fold->curr_num_new_outer_states;

				int starting_scope_id = fold_score_node->fold->scope_context.back();

				// add networks and update_state_sizes before updating scopes
				for (map<int, vector<vector<StateNetwork*>>>::iterator it = fold_score_node->fold->curr_outer_state_networks.begin();
						it != fold_score_node->fold->curr_outer_state_networks.end(); it++) {
					Scope* scope = solution->scopes[it->first];
					for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
						if (it->second[n_index].size() > 0) {
							ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
							for (int s_index = 0; s_index < new_outer_state_size; s_index++) {
								if (!fold_score_node->fold->curr_outer_state_networks_not_needed[it->first][n_index][s_index]) {
									if (it->first == starting_scope_id) {
										action_node->state_network_target_is_local.push_back(true);
										action_node->state_network_target_indexes.push_back(scope->num_local_states+s_index);
										action_node->state_networks.push_back(it->second[n_index][s_index]);

										action_node->state_networks.back()->update_state_sizes(scope->num_local_states,
																							   scope->num_input_states);
										action_node->state_networks.back()->new_outer_to_local();
									} else {
										action_node->state_network_target_is_local.push_back(false);
										action_node->state_network_target_indexes.push_back(scope->num_input_states+s_index);
										action_node->state_networks.push_back(it->second[n_index][s_index]);

										action_node->state_networks.back()->update_state_sizes(scope->num_local_states,
																							   scope->num_input_states);
										action_node->state_networks.back()->new_outer_to_input();
									}
								}
							}
						}
					}
				}

				StateNetwork* branch_score_network = fold_score_node->fold->curr_starting_score_network;
				branch_score_network->update_state_sizes(this->num_local_states,
														 this->num_input_states);
				if (this->id == starting_scope_id) {
					branch_score_network->new_outer_to_local();
				} else {
					branch_score_network->new_outer_to_input();
				}

				// for now, always add new_outer to starting_scope_id
				// TODO: find good way to remove need to do so
				solution->scopes[starting_scope_id]->new_outer_to_local(new_outer_state_size);

				for (set<int>::iterator it = fold_score_node->fold->curr_outer_scopes_needed.begin();
						it != fold_score_node->fold->curr_outer_scopes_needed.end(); it++) {
					if (*it != starting_scope_id) {
						solution->scopes[*it]->new_outer_to_input(new_outer_state_size);
					}
				}

				for (set<pair<int, int>>::iterator it = fold_score_node->fold->curr_outer_contexts_needed.begin();
						it != fold_score_node->fold->curr_outer_contexts_needed.end(); it++) {
					ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
					Scope* outer_scope = solution->scopes[(*it).first];
					Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
					if ((*it).first == starting_scope_id) {
						for (int s_index = 0; s_index < new_outer_state_size; s_index++) {
							scope_node->inner_input_is_local.push_back(true);
							scope_node->inner_input_indexes.push_back(outer_scope->num_local_states-new_outer_state_size+s_index);
							scope_node->inner_input_target_indexes.push_back(inner_scope->num_input_states-new_outer_state_size+s_index);
						}
					} else {
						for (int s_index = 0; s_index < new_outer_state_size; s_index++) {
							scope_node->inner_input_is_local.push_back(false);
							scope_node->inner_input_indexes.push_back(outer_scope->num_input_states-new_outer_state_size+s_index);
							scope_node->inner_input_target_indexes.push_back(inner_scope->num_input_states-new_outer_state_size+s_index);
						}
					}
				}

				BranchNode* new_branch_node = new BranchNode(fold_score_node->fold_scope_context,
															 fold_score_node->fold_node_context,
															 fold_score_node->fold_is_pass_through,
															 branch_score_network,
															 fold_score_node->fold_exit_depth,
															 fold_score_node->fold_next_node_id,
															 fold_score_node->existing_score_network,
															 fold_score_node->existing_next_node_id);
				fold_score_node->existing_score_network = NULL;		// for garbage collection
				delete this->nodes[history->node_histories[h_index]->scope_index];
				this->nodes[history->node_histories[h_index]->scope_index] = new_branch_node;
				// delete fold_score_node along with fold
			}
		} else {
			// history->node_histories[h_index]->node->type == NODE_TYPE_FOLD_SEQUENCE
			FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)history->node_histories[h_index]->node;
			fold_sequence_node->backprop(local_state_errors,
										 input_errors,
										 target_val,
										 final_misguess,
										 final_sum_impact,
										 predicted_score,
										 scale_factor,
										 run_helper,
										 (FoldSequenceNodeHistory*)history->node_histories[h_index]);

			Fold* fold = ((FoldSequenceNodeHistory*)history->node_histories[h_index])->fold_history->fold;
			if (fold->state == FOLD_STATE_DONE) {
				vector<AbstractNode*> new_nodes;
				fold_to_nodes(this,
							  fold,
							  new_nodes);

				int starting_new_node_id = (int)this->nodes.size();
				for (int n_index = 0; n_index < (int)new_nodes.size()-1; n_index++) {
					if (new_nodes[n_index]->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)new_nodes[n_index];
						action_node->next_node_id = (int)this->nodes.size()+1;
					} else {
						ScopeNode* scope_node = (ScopeNode*)new_nodes[n_index];
						scope_node->next_node_id = (int)this->nodes.size()+1;
					}
					this->nodes.push_back(new_nodes[n_index]);
				}
				if (new_nodes.back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)new_nodes.back();
					action_node->next_node_id = fold_sequence_node->next_node_id;
				} else {
					ScopeNode* scope_node = (ScopeNode*)new_nodes.back();
					scope_node->next_node_id = fold_sequence_node->next_node_id;
				}
				this->nodes.push_back(new_nodes.back());

				PassThroughNode* new_pass_through_node = new PassThroughNode(starting_new_node_id);
				delete this->nodes[history->node_histories[h_index]->scope_index];
				this->nodes[history->node_histories[h_index]->scope_index] = new_pass_through_node;
			}
		}
	}
}

void Scope::backprop_explore_fold_helper(vector<double>& local_state_errors,
										 vector<double>& input_errors,
										 double target_val,
										 double final_misguess,
										 double& predicted_score,
										 double& scale_factor,
										 RunHelper& run_helper,
										 ScopeHistory* history) {
	Fold* fold = history->explore_fold_history->fold;
	fold->explore_backprop(local_state_errors,
						   input_errors,
						   target_val,
						   final_misguess,
						   predicted_score,
						   scale_factor,
						   run_helper,
						   history->explore_fold_history);

	if (fold->state == FOLD_STATE_EXPLORE_FAIL) {
		Scope* score_scope = solution->scopes[fold->scope_context[0]];
		if (score_scope->nodes[fold->node_context[0]]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)score_scope->nodes[fold->node_context[0]];
			action_node->explore_scope_context.clear();
			action_node->explore_node_context.clear();
			action_node->explore_exit_depth = -1;
			action_node->explore_next_node_id = -1;
			delete action_node->explore_fold;
			action_node->explore_fold = NULL;
		} else {
			// score_scope->nodes[fold->node_context[0]]->type == NODE_TYPE_INNER_SCOPE
			ScopeNode* scope_node = (ScopeNode*)score_scope->nodes[fold->node_context[0]];
			scope_node->explore_scope_context.clear();
			scope_node->explore_node_context.clear();
			scope_node->explore_exit_depth = -1;
			scope_node->explore_next_node_id = -1;
			delete scope_node->explore_fold;
			scope_node->explore_fold = NULL;
		}
	} else if (fold->state == FOLD_STATE_EXPLORE_DONE) {
		fold->add_to_clean();

		// sequence_scope == this

		Scope* score_scope = solution->scopes[fold->scope_context[0]];
		StateNetwork* existing_score_network = new StateNetwork(0,
																score_scope->num_local_states,
																score_scope->num_input_states,
																0,
																0,
																20);
		bool fold_is_pass_through;
		if (fold->explore_result == FOLD_RESULT_BRANCH) {
			fold_is_pass_through = false;
		} else {
			// fold->explore_result == FOLD_RESULT_REPLACE
			fold_is_pass_through = true;
		}

		if (score_scope->nodes[fold->node_context[0]]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)score_scope->nodes[fold->node_context[0]];

			FoldSequenceNode* new_fold_sequence_node = new FoldSequenceNode(action_node->explore_next_node_id);
			this->nodes.push_back(new_fold_sequence_node);
			int fold_sequence_node_id = (int)this->nodes.size()-1;

			FoldScoreNode* new_fold_score_node = new FoldScoreNode(existing_score_network,
																   action_node->next_node_id,
																   fold,
																   fold_is_pass_through,
																   action_node->explore_scope_context,
																   action_node->explore_node_context,
																   action_node->explore_exit_depth,
																   fold_sequence_node_id);
			score_scope->nodes.push_back(new_fold_score_node);
			int fold_score_node_id = (int)score_scope->nodes.size()-1;

			action_node->next_node_id = fold_score_node_id;

			action_node->explore_scope_context.clear();
			action_node->explore_node_context.clear();
			action_node->explore_exit_depth = -1;
			action_node->explore_next_node_id = -1;
			action_node->explore_fold = NULL;
		} else {
			// score_scope->nodes[fold->node_context[0]]->type == NODE_TYPE_INNER_SCOPE
			ScopeNode* scope_node = (ScopeNode*)score_scope->nodes[fold->node_context[0]];

			FoldSequenceNode* new_fold_sequence_node = new FoldSequenceNode(scope_node->explore_next_node_id);
			this->nodes.push_back(new_fold_sequence_node);
			int fold_sequence_node_id = (int)this->nodes.size()-1;

			FoldScoreNode* new_fold_score_node = new FoldScoreNode(existing_score_network,
																   scope_node->next_node_id,
																   fold,
																   fold_is_pass_through,
																   scope_node->explore_scope_context,
																   scope_node->explore_node_context,
																   scope_node->explore_exit_depth,
																   fold_sequence_node_id);
			score_scope->nodes.push_back(new_fold_score_node);
			int fold_score_node_id = (int)score_scope->nodes.size()-1;

			scope_node->next_node_id = fold_score_node_id;

			scope_node->explore_scope_context.clear();
			scope_node->explore_node_context.clear();
			scope_node->explore_exit_depth = -1;
			scope_node->explore_next_node_id = -1;
			scope_node->explore_fold = NULL;
		}
	}

	run_helper.explore_phase = EXPLORE_PHASE_FLAT_BACKPROP_DONE;
}

void Scope::new_outer_to_local(int new_outer_size) {
	this->num_local_states += new_outer_size;

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			if (action_node->score_network->local_state_size == this->num_local_states) {
				// new node added from fold_seqeuence
			} else {
				action_node->score_network->add_local(new_outer_size);
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			if (scope_node->score_network->local_state_size == this->num_local_states) {
				// new node added from fold_seqeuence
			} else {
				scope_node->score_network->add_local(new_outer_size);
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			branch_node->branch_score_network->add_local(new_outer_size);
			branch_node->original_score_network->add_local(new_outer_size);
		} else if (this->nodes[n_index]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)this->nodes[n_index];
			fold_score_node->existing_score_network->add_local(new_outer_size);
		}
	}
}

void Scope::new_outer_to_input(int new_outer_size) {
	this->num_input_states += new_outer_size;

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			if (action_node->score_network->input_state_size == this->num_input_states) {
				// new node added from fold_seqeuence
			} else {
				action_node->score_network->add_input(new_outer_size);
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			if (scope_node->score_network->input_state_size == this->num_input_states) {
				// new node added from fold_seqeuence
			} else {
				scope_node->score_network->add_input(new_outer_size);
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			branch_node->branch_score_network->add_input(new_outer_size);
			branch_node->original_score_network->add_input(new_outer_size);
		} else if (this->nodes[n_index]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)this->nodes[n_index];
			fold_score_node->existing_score_network->add_input(new_outer_size);
		}
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->num_local_states << endl;
	output_file << this->num_input_states << endl;

	output_file << this->is_loop << endl;

	if (this->is_loop) {
		ofstream continue_network_save_file;
		continue_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue.txt");
		this->continue_network->save(continue_network_save_file);
		continue_network_save_file.close();

		ofstream halt_network_save_file;
		halt_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt.txt");
		this->halt_network->save(halt_network_save_file);
		halt_network_save_file.close();
	}

	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;

		ofstream node_save_file;
		node_save_file.open("saves/node_" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->nodes[n_index]->save(node_save_file,
								   this->id,
								   n_index);
		node_save_file.close();
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->explore_index = -1;
	this->explore_fold_history = NULL;
}

ScopeHistory::~ScopeHistory() {
	for (int h_index = 0; h_index < (int)this->node_histories.size(); h_index++) {
		delete this->node_histories[h_index];
	}

	if (this->explore_fold_history != NULL) {
		delete this->explore_fold_history;
	}
}
