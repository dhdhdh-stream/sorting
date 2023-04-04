#include "scope.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "fold_score_node.h"
#include "fold_sequence_node.h"
#include "fold_to_nodes.h"
#include "globals.h"
#include "loop_fold_node.h"
#include "loop_fold_to_scope.h"
#include "pass_through_node.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

Scope::Scope(int num_local_states,
			 int num_input_states,
			 bool is_loop,
			 vector<StateNetwork*> starting_state_networks,
			 StateNetwork* continue_score_network,
			 StateNetwork* continue_misguess_network,
			 StateNetwork* halt_score_network,
			 StateNetwork* halt_misguess_network,
			 vector<AbstractNode*> nodes) {
	this->num_local_states = num_local_states;
	this->num_input_states = num_input_states;
	this->is_loop = is_loop;
	this->starting_state_networks = starting_state_networks;
	this->continue_score_network = continue_score_network;
	this->continue_misguess_network = continue_misguess_network;
	this->halt_score_network = halt_score_network;
	this->halt_misguess_network = halt_misguess_network;
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

	string is_loop_line;
	getline(input_file, is_loop_line);
	this->is_loop = stoi(is_loop_line);

	if (this->is_loop) {
		string starting_state_networks_size_line;
		getline(input_file, starting_state_networks_size_line);
		int starting_state_networks_size = stoi(starting_state_networks_size_line);
		for (int l_index = 0; l_index < starting_state_networks_size; l_index++) {
			ifstream starting_state_network_save_file;
			starting_state_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_starting_state_" + to_string(l_index) + ".txt");
			this->starting_state_networks.push_back(new StateNetwork(starting_state_network_save_file));
			starting_state_network_save_file.close();
		}

		ifstream continue_score_network_save_file;
		continue_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_score.txt");
		this->continue_score_network = new StateNetwork(continue_score_network_save_file);
		continue_score_network_save_file.close();

		ifstream continue_misguess_network_save_file;
		continue_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_misguess.txt");
		this->continue_misguess_network = new StateNetwork(continue_misguess_network_save_file);
		continue_misguess_network_save_file.close();

		ifstream halt_score_network_save_file;
		halt_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_score.txt");
		this->halt_score_network = new StateNetwork(halt_score_network_save_file);
		halt_score_network_save_file.close();

		ifstream halt_misguess_network_save_file;
		halt_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_misguess.txt");
		this->halt_misguess_network = new StateNetwork(halt_misguess_network_save_file);
		halt_misguess_network_save_file.close();
	} else {
		this->continue_score_network = NULL;
		this->continue_misguess_network = NULL;
		this->halt_score_network = NULL;
		this->halt_misguess_network = NULL;
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
		} else if (node_type == NODE_TYPE_LOOP_FOLD) {
			LoopFoldNode* loop_fold_node = new LoopFoldNode(node_save_file,
															this->id,
															n_index);
			this->nodes.push_back(loop_fold_node);
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
	for (int l_index = 0; l_index < (int)this->starting_state_networks.size(); l_index++) {
		delete this->starting_state_networks[l_index];
	}

	if (this->continue_score_network != NULL) {
		delete this->continue_score_network;
	}

	if (this->continue_misguess_network != NULL) {
		delete this->continue_misguess_network;
	}

	if (this->halt_score_network != NULL) {
		delete this->halt_score_network;
	}

	if (this->halt_misguess_network != NULL) {
		delete this->halt_misguess_network;
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

	if (is_loop) {
		for (int l_index = 0; l_index < (int)this->starting_state_networks.size(); l_index++) {
			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
				StateNetworkHistory* network_history = new StateNetworkHistory(this->starting_state_networks[l_index]);
				this->starting_state_networks[l_index]->activate(local_state_vals,
																 input_vals,
																 network_history);
				history->starting_state_network_histories.push_back(network_history);
			} else {
				this->starting_state_networks[l_index]->activate(local_state_vals,
																 input_vals);
			}
			local_state_vals[l_index] += this->starting_state_networks[l_index]->output->acti_vals[0];
		}

		int iter_index = 0;
		while (true) {
			StateNetworkHistory* continue_score_network_history = new StateNetworkHistory(this->continue_score_network);
			this->continue_score_network->activate(local_state_vals,
												   input_vals,
												   continue_score_network_history);

			StateNetworkHistory* continue_misguess_network_history = new StateNetworkHistory(this->continue_misguess_network);
			this->continue_misguess_network->activate(local_state_vals,
													  input_vals,
													  continue_misguess_network_history);

			StateNetworkHistory* halt_score_network_history = new StateNetworkHistory(this->halt_score_network);
			this->halt_score_network->activate(local_state_vals,
											   input_vals,
											   halt_score_network_history);

			StateNetworkHistory* halt_misguess_network_history = new StateNetworkHistory(this->halt_misguess_network);
			this->halt_misguess_network->activate(local_state_vals,
												  input_vals,
												  halt_misguess_network_history);

			bool is_halt;
			if (iter_index > this->furthest_successful_halt+3) {
				is_halt = true;
			} else {
				double score_diff = scale_factor*this->continue_score_network->output->acti_vals[0]
					- scale_factor*this->halt_score_network->output->acti_vals[0];
				double score_standard_deviation = abs(scale_factor)*sqrt(this->score_variance);
				double score_diff_t_value = score_diff / score_standard_deviation;
				if (score_diff_t_value > 1.0) {	// >75%
					is_halt = false;
				} else if (score_diff_t_value < -1.0) {
					is_halt = true;

					if (iter_index > this->furthest_successful_halt) {
						this->furthest_successful_halt = iter_index;
					}
				} else {
					double misguess_diff = this->continue_misguess_network->output->acti_vals[0]
						- this->halt_misguess_network->output->acti_vals[0];
					double misguess_standard_deviation = sqrt(this->misguess_variance);
					double misguess_diff_t_value = misguess_diff / misguess_standard_deviation;
					if (misguess_diff_t_value < -1.0) {
						is_halt = false;
					} else if (misguess_diff_t_value > 1.0) {
						is_halt = true;

						if (iter_index > this->furthest_successful_halt) {
							this->furthest_successful_halt = iter_index;
						}
					} else {
						// halt if no strong signal either way
						is_halt = true;
					}
				}
			}

			if (is_halt) {
				history->halt_score_network_update = this->halt_score_network->output->acti_vals[0];
				history->halt_score_network_history = halt_score_network_history;

				history->halt_misguess_val = this->halt_misguess_network->output->acti_vals[0];
				history->halt_misguess_network_history = halt_misguess_network_history;

				delete continue_score_network_history;
				delete continue_misguess_network_history;

				predicted_score += scale_factor*this->halt_score_network->output->acti_vals[0];

				history->num_loop_iters = iter_index;
				
				break;
			} else {
				history->continue_score_network_updates.push_back(this->continue_score_network->output->acti_vals[0]);
				history->continue_score_network_histories.push_back(continue_score_network_history);

				history->continue_misguess_vals.push_back(this->continue_misguess_network->output->acti_vals[0]);
				history->continue_misguess_network_histories.push_back(continue_misguess_network_history);

				delete halt_score_network_history;
				delete halt_misguess_network_history;

				predicted_score += scale_factor*this->continue_score_network->output->acti_vals[0];

				history->node_histories.push_back(vector<AbstractNodeHistory*>());

				int curr_node_id = 0;
				FoldHistory* curr_fold_history = NULL;
				bool is_early_exit;
				while (true) {
					if (curr_node_id == -1) {
						break;
					}

					is_early_exit = handle_node_activate_helper(iter_index,
																curr_node_id,
																curr_fold_history,
																local_state_vals,
																input_vals,
																flat_vals,
																predicted_score,
																scale_factor,
																sum_impact,
																scope_context,
																node_context,
																context_histories,
																early_exit_depth,
																early_exit_node_id,
																early_exit_fold_history,
																explore_exit_depth,
																explore_exit_node_id,
																explore_exit_fold_history,
																run_helper,
																history);

					if (is_early_exit) {
						break;
					}
				}

				if (is_early_exit) {
					break;
				}
			}

			iter_index++;
		}
	} else {
		history->node_histories.push_back(vector<AbstractNodeHistory*>());

		int curr_node_id = 0;
		FoldHistory* curr_fold_history = NULL;
		while (true) {
			if (curr_node_id == -1) {
				break;
			}

			bool is_early_exit = handle_node_activate_helper(0,
															 curr_node_id,
															 curr_fold_history,
															 local_state_vals,
															 input_vals,
															 flat_vals,
															 predicted_score,
															 scale_factor,
															 sum_impact,
															 scope_context,
															 node_context,
															 context_histories,
															 early_exit_depth,
															 early_exit_node_id,
															 early_exit_fold_history,
															 explore_exit_depth,
															 explore_exit_node_id,
															 explore_exit_fold_history,
															 run_helper,
															 history);

			if (is_early_exit) {
				break;
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
	context_histories.pop_back();
}

bool Scope::handle_node_activate_helper(int iter_index,
										int& curr_node_id,
										FoldHistory*& curr_fold_history,
										vector<double>& local_state_vals,
										vector<double>& input_vals,
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
		history->node_histories[iter_index].push_back(node_history);

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
					// explore_phase set in fold
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
						history->explore_iter_index = iter_index;
						history->explore_node_index = (int)history->node_histories.size();

						curr_node_id = action_node->explore_next_node_id;
						return false;
					} else {
						explore_exit_depth = action_node->explore_exit_depth;
						explore_exit_node_id = action_node->explore_next_node_id;
						explore_exit_fold_history = fold_history;
						return true;
					}
				}
			} else if (action_node->explore_loop_fold != NULL) {
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
					// explore_phase set in fold
					LoopFoldHistory* loop_fold_history = new LoopFoldHistory(action_node->explore_loop_fold);
					action_node->explore_loop_fold->experiment_activate(local_state_vals,
																		input_vals,
																		flat_vals,
																		predicted_score,
																		scale_factor,
																		context_histories,
																		run_helper,
																		loop_fold_history);
					history->explore_loop_fold_history = loop_fold_history;

					history->explore_iter_index = iter_index;
					history->explore_node_index = (int)history->node_histories.size();

					// explore_next_node_id is just action_node->next_node_id
				}
			} else {
				// new explore
				run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;


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
		history->node_histories[iter_index].push_back(node_history);

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
				history->explore_iter_index = iter_index;
				history->explore_node_index = (int)history->node_histories.size();

				curr_node_id = inner_explore_exit_node_id;
			} else {
				explore_exit_depth = inner_explore_exit_depth-1;
				explore_exit_node_id = inner_explore_exit_node_id;
				explore_exit_fold_history = inner_explore_exit_fold_history;
				return true;
			}
		} else if (inner_early_exit_depth != -1) {
			if (inner_early_exit_depth == 1) {
				curr_node_id = inner_early_exit_node_id;
				curr_fold_history = inner_early_exit_fold_history;
			} else {
				early_exit_depth = inner_early_exit_depth-1;
				early_exit_node_id = inner_early_exit_node_id;
				early_exit_fold_history = inner_early_exit_fold_history;
				return true;
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
						// explore_phase set in fold
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
							history->explore_iter_index = iter_index;
							history->explore_node_index = (int)history->node_histories.size();

							curr_node_id = scope_node->explore_next_node_id;
							return false;
						} else {
							explore_exit_depth = scope_node->explore_exit_depth;
							explore_exit_node_id = scope_node->explore_next_node_id;
							explore_exit_fold_history = fold_history;
							return true;
						}
					}
				} else if (scope_node->explore_loop_fold != NULL) {
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
						// explore_phase set in fold
						LoopFoldHistory* loop_fold_history = new LoopFoldHistory(scope_node->explore_loop_fold);
						scope_node->explore_loop_fold->experiment_activate(local_state_vals,
																		   input_vals,
																		   flat_vals,
																		   predicted_score,
																		   scale_factor,
																		   context_histories,
																		   run_helper,
																		   loop_fold_history);
						history->explore_loop_fold_history = loop_fold_history;

						history->explore_iter_index = iter_index;
						history->explore_node_index = (int)history->node_histories.size();

						// explore_next_node_id is just scope_node->next_node_id
					}
				} else {
					// new explore
					run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;


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
		history->node_histories[iter_index].push_back(node_history);

		// branch_exit_depth != -1
		if (branch_exit_depth == 0) {
			curr_node_id = branch_exit_node_id;
		} else {
			early_exit_depth = branch_exit_depth;
			early_exit_node_id = branch_exit_node_id;
			early_exit_fold_history = NULL;
			return true;
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
		history->node_histories[iter_index].push_back(node_history);

		// fold_exit_depth != -1
		if (fold_exit_depth == 0) {
			curr_node_id = fold_exit_node_id;
			curr_fold_history = fold_exit_fold_history;
		} else {
			early_exit_depth = fold_exit_depth;
			early_exit_node_id = fold_exit_node_id;
			early_exit_fold_history = fold_exit_fold_history;
			return true;
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
		history->node_histories[iter_index].push_back(node_history);

		curr_node_id = fold_sequence_node->next_node_id;
		curr_fold_history = NULL;
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_LOOP_FOLD) {
		LoopFoldNode* loop_fold_node = (LoopFoldNode*)this->nodes[curr_node_id];

		LoopFoldNodeHistory* node_history = new LoopFoldNodeHistory(loop_fold_node,
																	curr_node_id);
		loop_fold_node->activate(local_state_vals,
								 input_vals,
								 flat_vals,
								 predicted_score,
								 scale_factor,
								 sum_impact,
								 scope_context,
								 node_context,
								 context_histories,
								 run_helper,
								 node_history);
		history->node_histories[iter_index].push_back(node_history);

		curr_node_id = loop_fold_node->next_node_id;
	} else {
		// this->nodes[curr_node_id]->type == NODE_TYPE_PASS_THROUGH
		PassThroughNode* pass_through_node = (PassThroughNode*)this->nodes[curr_node_id];

		// always goes local as can't care about context/branch

		curr_node_id = pass_through_node->next_node_id;
	}

	return false;
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
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		local_state_errors = vector<double>(this->num_local_states, 0.0);
	}

	if (is_loop) {
		if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
			this->average_score = 0.9999*this->average_score + 0.0001*target_val;
			double curr_score_variance = (this->average_score - target_val)*(this->average_score - target_val);
			this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;

			this->average_misguess = 0.9999*this->average_misguess + 0.0001*final_misguess;
			double curr_misguess_variance = (this->average_misguess - final_misguess)*(this->average_misguess - final_misguess);
			this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;
		}

		for (int iter_index = history->num_loop_iters-1; iter_index >= 0; iter_index--) {
			for (int h_index = (int)history->node_histories[iter_index].size()-1; h_index >= 0; h_index--) {
				if (this->nodes[history->node_histories[iter_index][h_index]->scope_index]
						!= history->node_histories[iter_index][h_index]->node) {
					// node replaced, mainly for fold nodes

					// skip processing history, should only have minor temporary impact

					continue;
				}

				if (history->explore_iter_index == iter_index
						&& history->explore_node_index == h_index+1) {	// node_history appended to, then explore indexes captured
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

				handle_node_backprop_helper(iter_index,
											h_index,
											local_state_errors,
											input_errors,
											target_val,
											final_misguess,
											final_sum_impact,
											predicted_score,
											scale_factor,
											run_helper,
											history);

				if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE) {
					break;
				}
			}

			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE) {
				break;
			}

			if (iter_index != history->num_loop_iters-1) {
				if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
					this->continue_score_network->backprop_errors_with_no_weight_change(
						target_val - predicted_score,
						local_state_errors,
						input_errors,
						history->continue_score_network_histories[iter_index]);

					predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

					this->continue_misguess_network->backprop_errors_with_no_weight_change(
						final_misguess - history->continue_misguess_vals[iter_index],
						local_state_errors,
						input_errors,
						history->continue_misguess_network_histories[iter_index]);
				} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
					this->continue_score_network->backprop_weights_with_no_error_signal(
						target_val - predicted_score,
						0.002,
						history->continue_score_network_histories[iter_index]);

					predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

					this->continue_misguess_network->backprop_weights_with_no_error_signal(
						final_misguess - history->continue_misguess_vals[iter_index],
						0.002,
						history->continue_misguess_network_histories[iter_index]);
				}
			} else {
				if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
					this->halt_score_network->backprop_errors_with_no_weight_change(
						target_val - predicted_score,
						local_state_errors,
						input_errors,
						history->halt_score_network_history);

					predicted_score -= scale_factor*history->halt_score_network_update;

					this->halt_misguess_network->backprop_errors_with_no_weight_change(
						final_misguess - history->halt_misguess_val,
						local_state_errors,
						input_errors,
						history->halt_misguess_network_history);
				} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
					this->halt_score_network->backprop_weights_with_no_error_signal(
						target_val - predicted_score,
						0.002,
						history->halt_score_network_history);

					predicted_score -= scale_factor*history->halt_score_network_update;

					this->halt_misguess_network->backprop_weights_with_no_error_signal(
						final_misguess - history->halt_misguess_val,
						0.002,
						history->halt_misguess_network_history);
				}
			}
		}

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			for (int l_index = (int)this->starting_state_networks.size()-1; l_index >= 0; l_index--) {
				this->starting_state_networks[l_index]->backprop_errors_with_no_weight_change(
					local_state_errors[l_index],
					local_state_errors,
					input_errors,
					history->starting_state_network_histories[l_index]);
			}
		}
	} else {
		for (int h_index = (int)history->node_histories[0].size()-1; h_index >= 0; h_index--) {
			if (this->nodes[history->node_histories[0][h_index]->scope_index]
					!= history->node_histories[0][h_index]->node) {
				// node replaced, mainly for fold nodes

				// skip processing history, should only have minor temporary impact

				continue;
			}

			if (history->explore_node_index == h_index+1) {	// node_history appended to, then explore indexes captured
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

			handle_node_backprop_helper(0,
										h_index,
										local_state_errors,
										input_errors,
										target_val,
										final_misguess,
										final_sum_impact,
										predicted_score,
										scale_factor,
										run_helper,
										history);

			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE) {
				break;
			}
		}
	}
}

void Scope::handle_node_backprop_helper(int iter_index,
										int h_index,
										vector<double>& local_state_errors,
										vector<double>& input_errors,
										double target_val,
										double final_misguess,
										double final_sum_impact,
										double& predicted_score,
										double& scale_factor,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)history->node_histories[iter_index][h_index]->node;
		action_node->backprop(local_state_errors,
							  input_errors,
							  target_val,
							  final_misguess,
							  final_sum_impact,
							  predicted_score,
							  scale_factor,
							  run_helper,
							  (ActionNodeHistory*)history->node_histories[iter_index][h_index]);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)history->node_histories[iter_index][h_index]->node;
		scope_node->backprop(local_state_errors,
							 input_errors,
							 target_val,
							 final_misguess,
							 final_sum_impact,
							 predicted_score,
							 scale_factor,
							 run_helper,
							 (ScopeNodeHistory*)history->node_histories[iter_index][h_index]);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)history->node_histories[iter_index][h_index]->node;
		branch_node->backprop(local_state_errors,
							  input_errors,
							  target_val,
							  predicted_score,
							  scale_factor,
							  run_helper,
							  (BranchNodeHistory*)history->node_histories[iter_index][h_index]);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_FOLD_SCORE) {
		FoldScoreNode* fold_score_node = (FoldScoreNode*)history->node_histories[iter_index][h_index]->node;
		fold_score_node->backprop(local_state_errors,
								  input_errors,
								  target_val,
								  predicted_score,
								  scale_factor,
								  run_helper,
								  (FoldScoreNodeHistory*)history->node_histories[iter_index][h_index]);

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

			// new_outer_state_size may be 0
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
			delete this->nodes[history->node_histories[iter_index][h_index]->scope_index];
			this->nodes[history->node_histories[iter_index][h_index]->scope_index] = new_branch_node;
			// delete fold_score_node along with fold
		}
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_FOLD_SEQUENCE) {
		FoldSequenceNode* fold_sequence_node = (FoldSequenceNode*)history->node_histories[iter_index][h_index]->node;
		fold_sequence_node->backprop(local_state_errors,
									 input_errors,
									 target_val,
									 final_misguess,
									 final_sum_impact,
									 predicted_score,
									 scale_factor,
									 run_helper,
									 (FoldSequenceNodeHistory*)history->node_histories[iter_index][h_index]);

		Fold* fold = ((FoldSequenceNodeHistory*)history->node_histories[iter_index][h_index])->fold_history->fold;
		if (fold->state == FOLD_STATE_DONE) {
			// update inner scopes before fold_to_nodes and FoldSequenceNode outer scopes updates
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = fold->curr_inner_state_networks.begin();
					it != fold->curr_inner_state_networks.end(); it++) {
				Scope* scope = solution->scopes[it->first];
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
						for (int i_index = 0; i_index < fold->curr_num_new_inner_states; i_index++) {
							if (!fold->curr_inner_state_networks_not_needed[it->first][n_index][i_index]) {
								action_node->state_network_target_is_local.push_back(false);
								action_node->state_network_target_indexes.push_back(scope->num_input_states+i_index);
								action_node->state_networks.push_back(it->second[n_index][i_index]);
								action_node->state_networks.back()->update_state_sizes(scope->num_local_states,
																					   scope->num_input_states);
								action_node->state_networks.back()->new_outer_to_input();
							}
						}
					}
				}
			}

			for (set<int>::iterator it = fold->curr_inner_scopes_needed.begin();
					it != fold->curr_inner_scopes_needed.end(); it++) {
				solution->scopes[*it]->new_outer_to_input(fold->curr_num_new_inner_states);
			}

			for (set<pair<int, int>>::iterator it = fold->curr_inner_contexts_needed.begin();
					it != fold->curr_inner_contexts_needed.end(); it++) {
				ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
				Scope* outer_scope = solution->scopes[(*it).first];
				Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
				for (int i_index = 0; i_index < fold->curr_num_new_inner_states; i_index++) {
					scope_node->inner_input_is_local.push_back(false);
					scope_node->inner_input_indexes.push_back(outer_scope->num_input_states-fold->curr_num_new_inner_states+i_index);
					scope_node->inner_input_target_indexes.push_back(inner_scope->num_input_states-fold->curr_num_new_inner_states+i_index);
				}
			}

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
			delete this->nodes[history->node_histories[iter_index][h_index]->scope_index];
			this->nodes[history->node_histories[iter_index][h_index]->scope_index] = new_pass_through_node;
		}
	} else {
		// history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_LOOP_FOLD
		LoopFoldNode* loop_fold_node = (LoopFoldNode*)history->node_histories[iter_index][h_index]->node;
		loop_fold_node->backprop(local_state_errors,
								 input_errors,
								 target_val,
								 final_misguess,
								 final_sum_impact,
								 predicted_score,
								 scale_factor,
								 run_helper,
								 (LoopFoldNodeHistory*)history->node_histories[iter_index][h_index]);

		LoopFold* loop_fold = loop_fold_node->loop_fold;
		if (loop_fold->state == LOOP_FOLD_STATE_DONE) {
			// input
			for (map<int, vector<vector<StateNetwork*>>>::iterator it = loop_fold->curr_inner_state_networks.begin();
					it != loop_fold->curr_inner_state_networks.end(); it++) {
				Scope* scope = solution->scopes[it->first];
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
						for (int i_index = 0; i_index < loop_fold->curr_num_new_inner_states; i_index++) {
							if (!loop_fold->curr_inner_state_networks_not_needed[it->first][n_index][i_index]) {
								action_node->state_network_target_is_local.push_back(false);
								action_node->state_network_target_indexes.push_back(scope->num_input_states+i_index);
								action_node->state_networks.push_back(it->second[n_index][i_index]);
								action_node->state_networks.back()->update_state_sizes(scope->num_local_states,
																					   scope->num_input_states);
								action_node->state_networks.back()->new_outer_to_input();
							}
						}
					}
				}
			}

			for (set<int>::iterator it = loop_fold->curr_inner_scopes_needed.begin();
					it != loop_fold->curr_inner_scopes_needed.end(); it++) {
				solution->scopes[*it]->new_outer_to_input(loop_fold->curr_num_new_inner_states);
			}

			for (set<pair<int, int>>::iterator it = loop_fold->curr_inner_contexts_needed.begin();
					it != loop_fold->curr_inner_contexts_needed.end(); it++) {
				ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
				Scope* outer_scope = solution->scopes[(*it).first];
				Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
				for (int i_index = 0; i_index < loop_fold->curr_num_new_inner_states; i_index++) {
					scope_node->inner_input_is_local.push_back(false);
					scope_node->inner_input_indexes.push_back(outer_scope->num_input_states-loop_fold->curr_num_new_inner_states+i_index);
					scope_node->inner_input_target_indexes.push_back(inner_scope->num_input_states-loop_fold->curr_num_new_inner_states+i_index);
				}
			}

			int new_scope_id;
			loop_fold_to_scope(loop_fold,
							   new_scope_id);

			vector<bool> new_inner_input_is_local;
			vector<int> new_inner_input_indexes;
			vector<int> new_inner_input_target_indexes;
			for (int l_index = 0; l_index < loop_fold->num_local_states; l_index++) {
				new_inner_input_is_local.push_back(true);
				new_inner_input_indexes.push_back(l_index);
				new_inner_input_target_indexes.push_back(l_index);
			}
			for (int i_index = 0; i_index < loop_fold->num_input_states; i_index++) {
				new_inner_input_is_local.push_back(false);
				new_inner_input_indexes.push_back(i_index);
				new_inner_input_target_indexes.push_back(loop_fold->num_local_states+i_index);
			}
			StateNetwork* new_score_network = new StateNetwork(0,
															   this->num_local_states,
															   this->num_input_states,
															   0,
															   0,
															   20);
			ScopeNode* new_scope_node = new ScopeNode(vector<bool>(),
													  vector<int>(),
													  vector<StateNetwork*>(),
													  new_scope_id,
													  new_inner_input_is_local,
													  new_inner_input_indexes,
													  new_inner_input_target_indexes,
													  vector<bool>(),
													  vector<int>(),
													  vector<StateNetwork*>(),
													  new_score_network);

			// outer
			int outer_scope_id = loop_fold->scope_context.back();

			for (map<int, vector<vector<StateNetwork*>>>::iterator it = loop_fold->curr_outer_state_networks.begin();
					it != loop_fold->curr_outer_state_networks.end(); it++) {
				Scope* scope = solution->scopes[it->first];
				for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
					if (it->second[n_index].size() > 0) {
						ActionNode* action_node = (ActionNode*)scope->nodes[n_index];
						for (int s_index = 0; s_index < loop_fold->curr_num_new_outer_states; s_index++) {
							if (!loop_fold->curr_outer_state_networks_not_needed[it->first][n_index][s_index]) {
								if (it->first == outer_scope_id) {
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

			// new_outer_state_size may be 0
			solution->scopes[outer_scope_id]->new_outer_to_local(loop_fold->curr_num_new_outer_states);

			for (set<int>::iterator it = loop_fold->curr_outer_scopes_needed.begin();
					it != loop_fold->curr_outer_scopes_needed.end(); it++) {
				if (*it != outer_scope_id) {
					solution->scopes[*it]->new_outer_to_input(loop_fold->curr_num_new_outer_states);
				}
			}

			for (set<pair<int, int>>::iterator it = loop_fold->curr_outer_contexts_needed.begin();
					it != loop_fold->curr_outer_contexts_needed.end(); it++) {
				ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
				Scope* outer_scope = solution->scopes[(*it).first];
				Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
				if ((*it).first == outer_scope_id) {
					for (int s_index = 0; s_index < loop_fold->curr_num_new_outer_states; s_index++) {
						scope_node->inner_input_is_local.push_back(true);
						scope_node->inner_input_indexes.push_back(outer_scope->num_local_states-loop_fold->curr_num_new_outer_states+s_index);
						scope_node->inner_input_target_indexes.push_back(inner_scope->num_input_states-loop_fold->curr_num_new_outer_states+s_index);
					}
				} else {
					for (int s_index = 0; s_index < loop_fold->curr_num_new_outer_states; s_index++) {
						scope_node->inner_input_is_local.push_back(false);
						scope_node->inner_input_indexes.push_back(outer_scope->num_input_states-loop_fold->curr_num_new_outer_states+s_index);
						scope_node->inner_input_target_indexes.push_back(inner_scope->num_input_states-loop_fold->curr_num_new_outer_states+s_index);
					}
				}
			}

			delete this->nodes[history->node_histories[iter_index][h_index]->scope_index];
			this->nodes[history->node_histories[iter_index][h_index]->scope_index] = new_scope_node;
			// delete loop_fold_node along with loop_fold
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
	if (history->explore_fold_history != NULL) {
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
	} else {
		// history->explore_loop_fold_history != NULL
		LoopFold* loop_fold = history->explore_loop_fold_history->loop_fold;
		loop_fold->experiment_backprop(local_state_errors,
									   input_errors,
									   target_val,
									   final_misguess,
									   predicted_score,
									   scale_factor,
									   run_helper,
									   history->explore_loop_fold_history);

		if (loop_fold->state == LOOP_FOLD_STATE_EXPERIMENT_FAIL) {
			if (this->nodes[loop_fold->node_context[0]]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->nodes[loop_fold->node_context[0]];
				action_node->explore_scope_context.clear();
				action_node->explore_node_context.clear();
				delete action_node->explore_loop_fold;
				action_node->explore_loop_fold = NULL;
			} else {
				// this->nodes[loop_fold->node_context[0]]->type == NODE_TYPE_INNER_SCOPE
				ScopeNode* scope_node = (ScopeNode*)this->nodes[loop_fold->node_context[0]];
				scope_node->explore_scope_context.clear();
				scope_node->explore_node_context.clear();
				delete scope_node->explore_loop_fold;
				scope_node->explore_loop_fold = NULL;
			}
		} else if (loop_fold->state == LOOP_FOLD_STATE_EXPERIMENT_DONE) {
			loop_fold->add_to_clean();

			if (this->nodes[loop_fold->node_context[0]]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->nodes[loop_fold->node_context[0]];

				LoopFoldNode* new_loop_fold_node = new LoopFoldNode(loop_fold,
																	action_node->explore_scope_context,
																	action_node->explore_node_context,
																	action_node->next_node_id);
				this->nodes.push_back(new_loop_fold_node);
				int new_node_id = (int)this->nodes.size()-1;
				action_node->next_node_id = new_node_id;

				action_node->explore_scope_context.clear();
				action_node->explore_node_context.clear();
				action_node->explore_loop_fold = NULL;
			} else {
				// this->nodes[loop_fold->node_context[0]]->type == NODE_TYPE_INNER_SCOPE
				ScopeNode* scope_node = (ScopeNode*)this->nodes[loop_fold->node_context[0]];

				LoopFoldNode* new_loop_fold_node = new LoopFoldNode(loop_fold,
																	scope_node->explore_scope_context,
																	scope_node->explore_node_context,
																	scope_node->next_node_id);
				this->nodes.push_back(new_loop_fold_node);
				int new_node_id = (int)this->nodes.size()-1;
				scope_node->next_node_id = new_node_id;

				scope_node->explore_scope_context.clear();
				scope_node->explore_node_context.clear();
				scope_node->explore_loop_fold = NULL;
			}
		}
	}

	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE;
}

void Scope::new_outer_to_local(int new_outer_size) {
	this->num_local_states += new_outer_size;

	if (is_loop) {
		this->continue_score_network->add_local(new_outer_size);
		this->continue_misguess_network->add_local(new_outer_size);
		this->halt_score_network->add_local(new_outer_size);
		this->halt_misguess_network->add_local(new_outer_size);
	}

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

	if (is_loop) {
		this->continue_score_network->add_input(new_outer_size);
		this->continue_misguess_network->add_input(new_outer_size);
		this->halt_score_network->add_input(new_outer_size);
		this->halt_misguess_network->add_input(new_outer_size);
	}

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
		output_file << this->starting_state_networks.size() << endl;
		for (int l_index = 0; l_index < (int)this->starting_state_networks.size(); l_index++) {
			ofstream starting_state_network_save_file;
			starting_state_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_starting_state_" + to_string(l_index) + ".txt");
			this->starting_state_networks[l_index]->save(starting_state_network_save_file);
			starting_state_network_save_file.close();
		}

		ofstream continue_score_network_save_file;
		continue_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_score.txt");
		this->continue_score_network->save(continue_score_network_save_file);
		continue_score_network_save_file.close();

		ofstream continue_misguess_network_save_file;
		continue_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_misguess.txt");
		this->continue_misguess_network->save(continue_misguess_network_save_file);
		continue_misguess_network_save_file.close();

		ofstream halt_score_network_save_file;
		halt_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_score.txt");
		this->halt_score_network->save(halt_score_network_save_file);
		halt_score_network_save_file.close();

		ofstream halt_misguess_network_save_file;
		halt_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_misguess.txt");
		this->halt_misguess_network->save(halt_misguess_network_save_file);
		halt_misguess_network_save_file.close();
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

	this->explore_iter_index = -1;
	this->explore_node_index = -1;
	this->explore_fold_history = NULL;
	this->explore_loop_fold_history = NULL;
}

ScopeHistory::~ScopeHistory() {
	for (int iter_index = 0; iter_index < (int)this->node_histories.size(); iter_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[iter_index].size(); h_index++) {
			delete this->node_histories[iter_index][h_index];
		}
	}

	if (this->explore_fold_history != NULL) {
		delete this->explore_fold_history;
	}

	if (this->explore_loop_fold_history != NULL) {
		delete this->explore_loop_fold_history;
	}
}
