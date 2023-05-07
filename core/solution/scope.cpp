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

Scope::Scope(int num_states,
			 vector<bool> is_initialized_locally,
			 bool is_loop,
			 vector<StateNetwork*> starting_state_networks,
			 StateNetwork* continue_score_network,
			 StateNetwork* continue_misguess_network,
			 StateNetwork* halt_score_network,
			 StateNetwork* halt_misguess_network,
			 double average_score,
			 double score_variance,
			 double average_misguess,
			 double misguess_variance,
			 vector<AbstractNode*> nodes) {
	this->num_states = num_states;
	this->is_initialized_locally = is_initialized_locally;
	this->is_loop = is_loop;
	this->starting_state_networks = starting_state_networks;
	this->continue_score_network = continue_score_network;
	this->continue_misguess_network = continue_misguess_network;
	this->halt_score_network = halt_score_network;
	this->halt_misguess_network = halt_misguess_network;
	this->average_score = average_score;
	this->score_variance = score_variance;
	this->average_misguess = average_misguess;
	this->misguess_variance = misguess_variance;
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

	this->furthest_successful_halt = 5;	// simply initializing to 5 for now
}

Scope::Scope(ifstream& input_file) {
	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

	string num_states_line;
	getline(input_file, num_states_line);
	this->num_states = stoi(num_states_line);
	
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		string is_initialized_locally_line;
		getline(input_file, is_initialized_locally_line);
		this->is_initialized_locally.push_back(stoi(is_initialized_locally_line));
	}

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

		string average_score_line;
		getline(input_file, average_score_line);
		this->average_score = stod(average_score_line);

		string score_variance_line;
		getline(input_file, score_variance_line);
		this->score_variance = stod(score_variance_line);

		string average_misguess_line;
		getline(input_file, average_misguess_line);
		this->average_misguess = stod(average_misguess_line);

		string misguess_variance_line;
		getline(input_file, misguess_variance_line);
		this->misguess_variance = stod(misguess_variance_line);
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
void Scope::activate(Problem& problem,
					 vector<double>& state_vals,
					 vector<bool>& inputs_initialized,
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
	early_exit_depth = -1;
	explore_exit_depth = -1;

	if (run_helper.flat_scope_id == this->id) {
		run_helper.is_recursive = true;
	}

	run_helper.curr_depth++;
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}

	vector<bool> states_initialized(this->num_states);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		if (inputs_initialized[s_index] || this->is_initialized_locally[s_index]) {
			states_initialized[s_index] = true;
		} else {
			states_initialized[s_index] = false;
		}
	}

	scope_context.push_back(this->id);
	node_context.push_back(-1);
	context_histories.push_back(history);

	if (is_loop) {
		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			history->starting_state_network_histories = vector<StateNetworkHistory*>(this->starting_state_networks.size(), NULL);
		}
		for (int s_index = 0; s_index < (int)this->starting_state_networks.size(); s_index++) {
			if (states_initialized[s_index]) {
				if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
					StateNetworkHistory* network_history = new StateNetworkHistory(this->starting_state_networks[s_index]);
					this->starting_state_networks[s_index]->activate(state_vals,
																	 network_history);
					history->starting_state_network_histories[s_index] = network_history;
				} else {
					this->starting_state_networks[s_index]->activate(state_vals);
				}
				state_vals[s_index] += this->starting_state_networks[s_index]->output->acti_vals[0];
			}
		}

		int iter_index = 0;
		while (true) {
			StateNetworkHistory* continue_score_network_history = new StateNetworkHistory(this->continue_score_network);
			this->continue_score_network->activate(state_vals,
												   continue_score_network_history);

			StateNetworkHistory* continue_misguess_network_history = new StateNetworkHistory(this->continue_misguess_network);
			this->continue_misguess_network->activate(state_vals,
													  continue_misguess_network_history);

			StateNetworkHistory* halt_score_network_history = new StateNetworkHistory(this->halt_score_network);
			this->halt_score_network->activate(state_vals,
											   halt_score_network_history);

			StateNetworkHistory* halt_misguess_network_history = new StateNetworkHistory(this->halt_misguess_network);
			this->halt_misguess_network->activate(state_vals,
												  halt_misguess_network_history);

			bool is_halt;
			if (iter_index > this->furthest_successful_halt+3) {
				is_halt = true;
			} else {
				double score_diff = scale_factor*this->continue_score_network->output->acti_vals[0]
					- scale_factor*this->halt_score_network->output->acti_vals[0];
				double score_standard_deviation = abs(scale_factor)*sqrt(this->score_variance);
				// TODO: not sure how network gradient descent corresponds to sample size, but simply set to 2500 for now
				double score_diff_t_value = score_diff
					/ (score_standard_deviation / sqrt(2500));
				if (score_diff_t_value > 2.326) {
					is_halt = false;
				} else if (score_diff_t_value < -2.326) {
					is_halt = true;

					if (iter_index > this->furthest_successful_halt) {
						this->furthest_successful_halt = iter_index;
					}
				} else {
					double misguess_diff = this->continue_misguess_network->output->acti_vals[0]
						- this->halt_misguess_network->output->acti_vals[0];
					double misguess_standard_deviation = sqrt(this->misguess_variance);
					double misguess_diff_t_value = misguess_diff
						/ (misguess_standard_deviation / sqrt(2500));
					if (misguess_diff_t_value < -2.326) {
						is_halt = false;
					} else if (misguess_diff_t_value > 2.326) {
						is_halt = true;

						if (iter_index > this->furthest_successful_halt) {
							this->furthest_successful_halt = iter_index;
						}
					} else {
						// continue if no strong signal either way
						is_halt = false;
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
																problem,
																state_vals,
																states_initialized,
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
					history->num_loop_iters = iter_index+1;	// not incremented yet
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
															 problem,
															 state_vals,
															 states_initialized,
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

	run_helper.curr_depth--;
}

bool Scope::handle_node_activate_helper(int iter_index,
										int& curr_node_id,
										FoldHistory*& curr_fold_history,
										Problem& problem,
										vector<double>& state_vals,
										vector<bool>& states_initialized,
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
		action_node->activate(problem,
							  state_vals,
							  states_initialized,
							  predicted_score,
							  scale_factor,
							  run_helper,
							  node_history);
		history->node_histories[iter_index].push_back(node_history);

		sum_impact += action_node->average_impact;

		if (run_helper.explore_phase == EXPLORE_PHASE_NONE
				&& randuni() < action_node->average_impact/action_node->average_sum_impact
				&& action_node->average_impact/action_node->average_sum_impact > 0.05) {	// TODO: find systematic way of gating
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
					action_node->explore_fold->score_activate(
						state_vals,
						predicted_score,
						scale_factor,
						context_histories,
						run_helper,
						fold_history);

					if (action_node->explore_exit_depth == 0) {
						action_node->explore_fold->sequence_activate(
							problem,
							state_vals,
							states_initialized,
							predicted_score,
							scale_factor,
							sum_impact,
							run_helper,
							fold_history);
						history->explore_iter_index = iter_index;
						history->explore_node_index = (int)history->node_histories[iter_index].size();
						history->explore_fold_history = fold_history;

						curr_node_id = action_node->explore_next_node_id;
						return false;
					} else {
						explore_exit_depth = action_node->explore_exit_depth;
						explore_exit_node_id = action_node->explore_next_node_id;
						explore_exit_fold_history = fold_history;
						return true;
					}
				} else {
					curr_node_id = action_node->next_node_id;
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
					action_node->explore_loop_fold->activate(problem,
															 state_vals,
															 states_initialized,
															 predicted_score,
															 scale_factor,
															 sum_impact,
															 context_histories,
															 run_helper,
															 loop_fold_history);
					history->explore_loop_fold_history = loop_fold_history;

					history->explore_iter_index = iter_index;
					history->explore_node_index = (int)history->node_histories[iter_index].size();

					// explore_next_node_id is just action_node->next_node_id
				}

				curr_node_id = action_node->next_node_id;
			} else {
				// new explore
				run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;

				if (action_node->action.move != ACTION_START && randuni() < 0.3) {
					node_context.back() = curr_node_id;

					explore_new_loop(curr_node_id,
									 problem,
									 state_vals,
									 states_initialized,
									 predicted_score,
									 scope_context,
									 node_context,
									 context_histories,
									 run_helper);

					node_context.back() = -1;

					// simply let state stay as is

					curr_node_id = action_node->next_node_id;
				} else {
					node_context.back() = curr_node_id;

					int new_explore_exit_depth;
					int new_explore_exit_node_id;
					explore_new_path(curr_node_id,
									 action_node->next_node_id,
									 problem,
									 state_vals,
									 states_initialized,
									 predicted_score,
									 scale_factor,
									 scope_context,
									 node_context,
									 context_histories,
									 new_explore_exit_depth,
									 new_explore_exit_node_id,
									 run_helper);

					node_context.back() = -1;

					// simply let state stay as is

					if (new_explore_exit_depth == 0) {
						curr_node_id = new_explore_exit_node_id;
					} else {
						early_exit_depth = new_explore_exit_depth;
						early_exit_node_id = new_explore_exit_node_id;
						return true;
					}
				}
			}
		} else {
			curr_node_id = action_node->next_node_id;
		}
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
		scope_node->activate(problem,
							 state_vals,
							 states_initialized,
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
				inner_explore_exit_fold_history->fold->sequence_activate(
					problem,
					state_vals,
					states_initialized,
					predicted_score,
					scale_factor,
					sum_impact,
					run_helper,
					inner_explore_exit_fold_history);
				history->explore_iter_index = iter_index;
				history->explore_node_index = (int)history->node_histories[iter_index].size();
				history->explore_fold_history = inner_explore_exit_fold_history;

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
						scope_node->explore_fold->score_activate(
							state_vals,
							predicted_score,
							scale_factor,
							context_histories,
							run_helper,
							fold_history);

						if (scope_node->explore_exit_depth == 0) {
							scope_node->explore_fold->sequence_activate(
								problem,
								state_vals,
								states_initialized,
								predicted_score,
								scale_factor,
								sum_impact,
								run_helper,
								fold_history);
							history->explore_iter_index = iter_index;
							history->explore_node_index = (int)history->node_histories[iter_index].size();
							history->explore_fold_history = fold_history;

							curr_node_id = scope_node->explore_next_node_id;
							return false;
						} else {
							explore_exit_depth = scope_node->explore_exit_depth;
							explore_exit_node_id = scope_node->explore_next_node_id;
							explore_exit_fold_history = fold_history;
							return true;
						}
					} else {
						curr_node_id = scope_node->next_node_id;
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
						scope_node->explore_loop_fold->activate(problem,
																state_vals,
																states_initialized,
																predicted_score,
																scale_factor,
																sum_impact,
																context_histories,
																run_helper,
																loop_fold_history);
						history->explore_loop_fold_history = loop_fold_history;

						history->explore_iter_index = iter_index;
						history->explore_node_index = (int)history->node_histories[iter_index].size();

						// explore_next_node_id is just scope_node->next_node_id
					}

					curr_node_id = scope_node->next_node_id;
				} else {
					// new explore
					run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;

					if (randuni() < 0.3) {
						node_context.back() = curr_node_id;

						explore_new_loop(curr_node_id,
										 problem,
										 state_vals,
										 states_initialized,
										 predicted_score,
										 scope_context,
										 node_context,
										 context_histories,
										 run_helper);

						node_context.back() = -1;

						// simply let state stay as is

						curr_node_id = scope_node->next_node_id;
					} else {
						node_context.back() = curr_node_id;

						int new_explore_exit_depth;
						int new_explore_exit_node_id;
						explore_new_path(curr_node_id,
										 scope_node->next_node_id,
										 problem,
										 state_vals,
										 states_initialized,
										 predicted_score,
										 scale_factor,
										 scope_context,
										 node_context,
										 context_histories,
										 new_explore_exit_depth,
										 new_explore_exit_node_id,
										 run_helper);

						node_context.back() = -1;

						// simply let state stay as is

						if (new_explore_exit_depth == 0) {
							curr_node_id = new_explore_exit_node_id;
						} else {
							early_exit_depth = new_explore_exit_depth;
							early_exit_node_id = new_explore_exit_node_id;
							return true;
						}
					}
				}
			} else {
				curr_node_id = scope_node->next_node_id;
			}
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		int branch_exit_depth;
		int branch_exit_node_id;
		BranchNodeHistory* node_history = new BranchNodeHistory(branch_node,
																curr_node_id);
		branch_node->activate(state_vals,
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
		fold_score_node->activate(state_vals,
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
									 problem,
									 state_vals,
									 states_initialized,
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
		loop_fold_node->activate(problem,
								 state_vals,
								 states_initialized,
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

void Scope::backprop(vector<double>& state_errors,
					 vector<bool>& inputs_initialized,
					 double target_val,
					 double final_diff,
					 double final_misguess,
					 double final_sum_impact,
					 double& predicted_score,
					 double& scale_factor,
					 double& scale_factor_error,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	vector<bool> states_initialized;
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
		states_initialized = vector<bool>(this->num_states);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			if (inputs_initialized[s_index] || this->is_initialized_locally[s_index]) {
				states_initialized[s_index] = true;
			} else {
				states_initialized[s_index] = false;
			}
		}
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

		if (history->halt_score_network_history != NULL) {	// check for if early exit
			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
				double predicted_score_error = target_val - predicted_score;
				this->halt_score_network->backprop_errors_with_no_weight_change(
					scale_factor*predicted_score_error,
					state_errors,
					history->halt_score_network_history);

				predicted_score -= scale_factor*history->halt_score_network_update;

				this->halt_misguess_network->backprop_errors_with_no_weight_change(
					final_misguess - history->halt_misguess_val,
					state_errors,
					history->halt_misguess_network_history);
			} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
				double predicted_score_error = target_val - predicted_score;

				scale_factor_error += history->halt_score_network_update*predicted_score_error;

				this->halt_score_network->backprop_weights_with_no_error_signal(
					scale_factor*predicted_score_error,
					0.002,
					history->halt_score_network_history);

				predicted_score -= scale_factor*history->halt_score_network_update;

				this->halt_misguess_network->backprop_weights_with_no_error_signal(
					final_misguess - history->halt_misguess_val,
					0.002,
					history->halt_misguess_network_history);
			}
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
					backprop_explore_fold_helper(state_errors,
												 states_initialized,
												 target_val,
												 final_diff,
												 final_misguess,
												 final_sum_impact,
												 predicted_score,
												 scale_factor,
												 scale_factor_error,
												 run_helper,
												 history);
					break;
				}

				handle_node_backprop_helper(iter_index,
											h_index,
											state_errors,
											states_initialized,
											target_val,
											final_diff,
											final_misguess,
											final_sum_impact,
											predicted_score,
											scale_factor,
											scale_factor_error,
											run_helper,
											history);

				if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE) {
					break;
				}
			}

			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE) {
				break;
			}

			if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
				double predicted_score_error = target_val - predicted_score;
				this->continue_score_network->backprop_errors_with_no_weight_change(
					scale_factor*predicted_score_error,
					state_errors,
					history->continue_score_network_histories[iter_index]);

				predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

				this->continue_misguess_network->backprop_errors_with_no_weight_change(
					final_misguess - history->continue_misguess_vals[iter_index],
					state_errors,
					history->continue_misguess_network_histories[iter_index]);
			} else if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
				double predicted_score_error = target_val - predicted_score;

				scale_factor_error += history->continue_score_network_updates[iter_index]*predicted_score_error;

				this->continue_score_network->backprop_weights_with_no_error_signal(
					scale_factor*predicted_score_error,
					0.002,
					history->continue_score_network_histories[iter_index]);

				predicted_score -= scale_factor*history->continue_score_network_updates[iter_index];

				this->continue_misguess_network->backprop_weights_with_no_error_signal(
					final_misguess - history->continue_misguess_vals[iter_index],
					0.002,
					history->continue_misguess_network_histories[iter_index]);
			}
		}

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT_LEARN) {
			for (int s_index = (int)this->starting_state_networks.size()-1; s_index >= 0; s_index--) {
				if (states_initialized[s_index]) {
					this->starting_state_networks[s_index]->backprop_errors_with_no_weight_change(
						state_errors[s_index],
						state_errors,
						history->starting_state_network_histories[s_index]);
				}
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
				backprop_explore_fold_helper(state_errors,
											 states_initialized,
											 target_val,
											 final_diff,
											 final_misguess,
											 final_sum_impact,
											 predicted_score,
											 scale_factor,
											 scale_factor_error,
											 run_helper,
											 history);
				break;
			}

			handle_node_backprop_helper(0,
										h_index,
										state_errors,
										states_initialized,
										target_val,
										final_diff,
										final_misguess,
										final_sum_impact,
										predicted_score,
										scale_factor,
										scale_factor_error,
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
										vector<double>& state_errors,
										vector<bool>& states_initialized,
										double target_val,
										double final_diff,
										double final_misguess,
										double final_sum_impact,
										double& predicted_score,
										double& scale_factor,
										double& scale_factor_error,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)history->node_histories[iter_index][h_index]->node;
		action_node->backprop(state_errors,
							  states_initialized,
							  target_val,
							  final_misguess,
							  final_sum_impact,
							  predicted_score,
							  scale_factor,
							  scale_factor_error,
							  run_helper,
							  (ActionNodeHistory*)history->node_histories[iter_index][h_index]);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)history->node_histories[iter_index][h_index]->node;
		scope_node->backprop(state_errors,
							 states_initialized,
							 target_val,
							 final_diff,
							 final_misguess,
							 final_sum_impact,
							 predicted_score,
							 scale_factor,
							 scale_factor_error,
							 run_helper,
							 (ScopeNodeHistory*)history->node_histories[iter_index][h_index]);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)history->node_histories[iter_index][h_index]->node;
		branch_node->backprop(state_errors,
							  target_val,
							  predicted_score,
							  scale_factor,
							  scale_factor_error,
							  run_helper,
							  (BranchNodeHistory*)history->node_histories[iter_index][h_index]);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_FOLD_SCORE) {
		FoldScoreNode* fold_score_node = (FoldScoreNode*)history->node_histories[iter_index][h_index]->node;
		fold_score_node->backprop(state_errors,
								  target_val,
								  predicted_score,
								  scale_factor,
								  scale_factor_error,
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
								action_node->state_network_target_indexes.push_back(scope->num_states+s_index);
								action_node->state_networks.push_back(it->second[n_index][s_index]);

								action_node->state_networks.back()->update_state_size(scope->num_states);
								action_node->state_networks.back()->new_external_to_state();
							}
						}
					}
				}
			}

			StateNetwork* branch_score_network = fold_score_node->fold->curr_starting_score_network;
			branch_score_network->update_state_size(this->num_states);
			branch_score_network->new_external_to_state();

			// new_outer_state_size may be 0
			solution->scopes[starting_scope_id]->add_new_state(new_outer_state_size,
															   true);

			for (set<int>::iterator it = fold_score_node->fold->curr_outer_scopes_needed.begin();
					it != fold_score_node->fold->curr_outer_scopes_needed.end(); it++) {
				if (*it != starting_scope_id) {
					solution->scopes[*it]->add_new_state(new_outer_state_size,
														 false);
				}
			}

			for (set<pair<int, int>>::iterator it = fold_score_node->fold->curr_outer_contexts_needed.begin();
					it != fold_score_node->fold->curr_outer_contexts_needed.end(); it++) {
				ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
				Scope* outer_scope = solution->scopes[(*it).first];
				Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
				for (int s_index = 0; s_index < new_outer_state_size; s_index++) {
					scope_node->inner_input_indexes.push_back(outer_scope->num_states-new_outer_state_size+s_index);
					scope_node->inner_input_target_indexes.push_back(inner_scope->num_states-new_outer_state_size+s_index);
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
		fold_sequence_node->backprop(state_errors,
									 states_initialized,
									 target_val,
									 final_diff,
									 final_misguess,
									 final_sum_impact,
									 predicted_score,
									 scale_factor,
									 scale_factor_error,
									 run_helper,
									 (FoldSequenceNodeHistory*)history->node_histories[iter_index][h_index]);

		// re-check as might have been updated by inner
		if (this->nodes[history->node_histories[iter_index][h_index]->scope_index]->type == NODE_TYPE_FOLD_SEQUENCE) {
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
									action_node->state_network_target_indexes.push_back(scope->num_states+i_index);
									action_node->state_networks.push_back(it->second[n_index][i_index]);
									action_node->state_networks.back()->update_state_size(scope->num_states);
									action_node->state_networks.back()->new_external_to_state();
								}
							}
						}
					}
				}

				for (set<int>::iterator it = fold->curr_inner_scopes_needed.begin();
						it != fold->curr_inner_scopes_needed.end(); it++) {
					solution->scopes[*it]->add_new_state(fold->curr_num_new_inner_states,
														 false);
				}

				for (set<pair<int, int>>::iterator it = fold->curr_inner_contexts_needed.begin();
						it != fold->curr_inner_contexts_needed.end(); it++) {
					ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
					Scope* outer_scope = solution->scopes[(*it).first];
					Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
					for (int i_index = 0; i_index < fold->curr_num_new_inner_states; i_index++) {
						scope_node->inner_input_indexes.push_back(outer_scope->num_states-fold->curr_num_new_inner_states+i_index);
						scope_node->inner_input_target_indexes.push_back(inner_scope->num_states-fold->curr_num_new_inner_states+i_index);
					}
				}

				vector<AbstractNode*> new_nodes;
				fold_to_nodes(this,
							  fold,
							  new_nodes);

				if (new_nodes.size() > 0) {
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
				} else {
					PassThroughNode* new_pass_through_node = new PassThroughNode(fold_sequence_node->next_node_id);
					delete this->nodes[history->node_histories[iter_index][h_index]->scope_index];
					this->nodes[history->node_histories[iter_index][h_index]->scope_index] = new_pass_through_node;
				}
			}
		}
	} else {
		// history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_LOOP_FOLD
		LoopFoldNode* loop_fold_node = (LoopFoldNode*)history->node_histories[iter_index][h_index]->node;
		loop_fold_node->backprop(state_errors,
								 states_initialized,
								 target_val,
								 final_diff,
								 final_misguess,
								 final_sum_impact,
								 predicted_score,
								 scale_factor,
								 scale_factor_error,
								 run_helper,
								 (LoopFoldNodeHistory*)history->node_histories[iter_index][h_index]);

		// re-check as might have been updated by inner
		if (this->nodes[history->node_histories[iter_index][h_index]->scope_index]->type == NODE_TYPE_LOOP_FOLD) {
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
									action_node->state_network_target_indexes.push_back(scope->num_states+i_index);
									action_node->state_networks.push_back(it->second[n_index][i_index]);

									action_node->state_networks.back()->update_state_size(scope->num_states);
									action_node->state_networks.back()->new_external_to_state();
								}
							}
						}
					}
				}

				for (set<int>::iterator it = loop_fold->curr_inner_scopes_needed.begin();
						it != loop_fold->curr_inner_scopes_needed.end(); it++) {
					solution->scopes[*it]->add_new_state(loop_fold->curr_num_new_inner_states,
														 false);
				}

				for (set<pair<int, int>>::iterator it = loop_fold->curr_inner_contexts_needed.begin();
						it != loop_fold->curr_inner_contexts_needed.end(); it++) {
					ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
					Scope* outer_scope = solution->scopes[(*it).first];
					Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
					for (int i_index = 0; i_index < loop_fold->curr_num_new_inner_states; i_index++) {
						scope_node->inner_input_indexes.push_back(outer_scope->num_states-loop_fold->curr_num_new_inner_states+i_index);
						scope_node->inner_input_target_indexes.push_back(inner_scope->num_states-loop_fold->curr_num_new_inner_states+i_index);
					}
				}

				int new_scope_id;
				loop_fold_to_scope(loop_fold,
								   new_scope_id);

				int new_scope_num_states = solution->scopes[new_scope_id]->num_states;

				vector<int> new_inner_input_indexes;
				vector<int> new_inner_input_target_indexes;
				for (int s_index = 0; s_index < loop_fold->num_states; s_index++) {
					// TODO: remove unneeded num_states
					new_inner_input_indexes.push_back(s_index);
					new_inner_input_target_indexes.push_back(new_scope_num_states
						- loop_fold->num_states
						- loop_fold->curr_num_new_outer_states
						+ s_index);
				}
				for (int o_index = 0; o_index < loop_fold->curr_num_new_outer_states; o_index++) {
					// outer not updated yet
					new_inner_input_indexes.push_back(this->num_states + o_index);
					new_inner_input_target_indexes.push_back(new_scope_num_states
						- loop_fold->curr_num_new_outer_states
						+ o_index);
				}
				StateNetwork* new_score_network = new StateNetwork(0,
																   this->num_states,
																   0,
																   0,
																   20);
				ScopeNode* new_scope_node = new ScopeNode(vector<int>(),
														  vector<StateNetwork*>(),
														  new_scope_id,
														  new_inner_input_indexes,
														  new_inner_input_target_indexes,
														  new Scale(1.0),
														  vector<int>(),
														  vector<StateNetwork*>(),
														  new_score_network);
				new_scope_node->next_node_id = loop_fold_node->next_node_id;

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
									action_node->state_network_target_indexes.push_back(scope->num_states+s_index);
									action_node->state_networks.push_back(it->second[n_index][s_index]);

									action_node->state_networks.back()->update_state_size(scope->num_states);
									action_node->state_networks.back()->new_external_to_state();
								}
							}
						}
					}
				}

				// new_outer_state_size may be 0
				solution->scopes[outer_scope_id]->add_new_state(loop_fold->curr_num_new_outer_states,
																true);

				for (set<int>::iterator it = loop_fold->curr_outer_scopes_needed.begin();
						it != loop_fold->curr_outer_scopes_needed.end(); it++) {
					if (*it != outer_scope_id) {
						solution->scopes[*it]->add_new_state(loop_fold->curr_num_new_outer_states,
															 false);
					}
				}

				for (set<pair<int, int>>::iterator it = loop_fold->curr_outer_contexts_needed.begin();
						it != loop_fold->curr_outer_contexts_needed.end(); it++) {
					ScopeNode* scope_node = (ScopeNode*)solution->scopes[(*it).first]->nodes[(*it).second];
					Scope* outer_scope = solution->scopes[(*it).first];
					Scope* inner_scope = solution->scopes[scope_node->inner_scope_id];
					for (int s_index = 0; s_index < loop_fold->curr_num_new_outer_states; s_index++) {
						scope_node->inner_input_indexes.push_back(outer_scope->num_states-loop_fold->curr_num_new_outer_states+s_index);
						scope_node->inner_input_target_indexes.push_back(inner_scope->num_states-loop_fold->curr_num_new_outer_states+s_index);
					}
				}

				delete this->nodes[history->node_histories[iter_index][h_index]->scope_index];
				this->nodes[history->node_histories[iter_index][h_index]->scope_index] = new_scope_node;
				// delete loop_fold_node along with loop_fold
			}
		}
	}
}

void Scope::explore_new_loop(int curr_node_id,
							 Problem& problem,
							 vector<double>& state_vals,
							 vector<bool>& states_initialized,
							 double& predicted_score,
							 vector<int>& scope_context,
							 vector<int>& node_context,
							 vector<ScopeHistory*>& context_histories,
							 RunHelper& run_helper) {
	vector<bool> explore_is_inner_scope;
	vector<int> explore_existing_scope_ids;
	vector<Action> explore_actions;
	solution->backtrack_for_loop(context_histories,
								 explore_is_inner_scope,
								 explore_existing_scope_ids,
								 explore_actions);

	int num_iters = 2 + rand()%3;	// 3-5
	for (int i_index = 0; i_index < num_iters; i_index++) {
		for (int f_index = 0; f_index < (int)explore_is_inner_scope.size(); f_index++) {
			if (explore_is_inner_scope[f_index]) {
				Scope* inner_scope = solution->scopes[explore_existing_scope_ids[f_index]];

				vector<double> inner_input_vals(inner_scope->num_states);
				vector<bool> inner_inputs_initialized(inner_scope->num_states);
				for (int s_index = 0; s_index < inner_scope->num_states; s_index++) {
					if (randuni() < 0.2 || state_vals.size() == 0) {
						inner_input_vals[s_index] = 0.0;
						inner_inputs_initialized[s_index] = false;
					} else {
						// can be 0.0 and uninitialized as well
						int rand_index = rand()%(int)state_vals.size();
						inner_input_vals[s_index] = state_vals[rand_index];
						inner_inputs_initialized[s_index] = states_initialized[rand_index];
					}
				}

				// unused
				double inner_predicted_score = 0.0;
				double inner_scale_factor;
				if (randuni() < 0.2) {
					inner_scale_factor = -1.0;
				} else {
					inner_scale_factor = 1.0;
				}
				double inner_sum_impact = 0.0;
				vector<int> inner_scope_context;
				vector<int> inner_node_context;
				vector<ScopeHistory*> inner_context_histories;
				int inner_early_exit_depth;
				int inner_early_exit_node_id;
				FoldHistory* inner_early_exit_fold_history;
				int inner_explore_exit_depth;
				int inner_explore_exit_node_id;
				FoldHistory* inner_explore_exit_fold_history;

				ScopeHistory* scope_history = new ScopeHistory(inner_scope);
				inner_scope->activate(problem,
									  inner_input_vals,
									  inner_inputs_initialized,
									  inner_predicted_score,
									  inner_scale_factor,
									  inner_sum_impact,
									  inner_scope_context,
									  inner_node_context,
									  inner_context_histories,
									  inner_early_exit_depth,
									  inner_early_exit_node_id,
									  inner_early_exit_fold_history,
									  inner_explore_exit_depth,
									  inner_explore_exit_node_id,
									  inner_explore_exit_fold_history,
									  run_helper,
									  scope_history);
				delete scope_history;
			} else {
				problem.perform_action(explore_actions[f_index]);
			}
		}
	}

	vector<int> explore_scope_context;
	vector<int> explore_node_context;
	geometric_distribution<int> num_context_geo_dist(0.5);
	int num_contexts = 1 + num_context_geo_dist(generator);
	if (num_contexts > (int)scope_context.size()) {
		num_contexts = (int)scope_context.size();
	}
	for (int c_index = 0; c_index < num_contexts; c_index++) {
		explore_scope_context.push_back(scope_context[(int)scope_context.size()-1-c_index]);
		explore_node_context.push_back(node_context[(int)node_context.size()-1-c_index]);
	}

	run_helper.explore_scope_id = this->id;
	run_helper.explore_node_id = curr_node_id;
	run_helper.explore_scope_context = explore_scope_context;
	run_helper.explore_node_context = explore_node_context;
	run_helper.explore_is_loop = true;
	run_helper.explore_is_inner_scope = explore_is_inner_scope;
	run_helper.explore_existing_scope_ids = explore_existing_scope_ids;
	run_helper.explore_actions = explore_actions;
	run_helper.explore_seed_start_predicted_score = predicted_score;
}

void Scope::explore_new_path(int curr_node_id,
							 int next_node_id,
							 Problem& problem,
							 vector<double>& state_vals,
							 vector<bool>& states_initialized,
							 double& predicted_score,
							 double& scale_factor,
							 vector<int>& scope_context,
							 vector<int>& node_context,
							 vector<ScopeHistory*>& context_histories,
							 int& new_explore_exit_depth,
							 int& new_explore_exit_node_id,
							 RunHelper& run_helper) {
	vector<int> potential_exit_depths;
	vector<int> potential_next_node_ids;
	solution->random_run_continuation(next_node_id,
									  scope_context,
									  node_context,
									  potential_exit_depths,
									  potential_next_node_ids);

	int random_exit_index = rand()%(int)potential_exit_depths.size();
	bool can_be_empty;
	if (random_exit_index == 0) {
		can_be_empty = false;
	} else {
		can_be_empty = true;
	}

	vector<bool> explore_is_inner_scope;
	vector<int> explore_existing_scope_ids;
	vector<Action> explore_actions;
	solution->new_sequence(explore_is_inner_scope,
						   explore_existing_scope_ids,
						   explore_actions,
						   can_be_empty);

	for (int f_index = 0; f_index < (int)explore_is_inner_scope.size(); f_index++) {
		if (explore_is_inner_scope[f_index]) {
			Scope* inner_scope = solution->scopes[explore_existing_scope_ids[f_index]];

			vector<double> inner_input_vals(inner_scope->num_states);
			vector<bool> inner_inputs_initialized(inner_scope->num_states);
			for (int s_index = 0; s_index < inner_scope->num_states; s_index++) {
				if (randuni() < 0.2 || state_vals.size() == 0) {
					inner_input_vals[s_index] = 0.0;
					inner_inputs_initialized[s_index] = false;
				} else {
					// can be 0.0 and uninitialized as well
					int rand_index = rand()%(int)state_vals.size();
					inner_input_vals[s_index] = state_vals[rand_index];
					inner_inputs_initialized[s_index] = states_initialized[rand_index];
				}
			}

			// unused
			double inner_predicted_score = 0.0;
			double inner_scale_factor;
			if (randuni() < 0.2) {
				inner_scale_factor = -1.0;
			} else {
				inner_scale_factor = 1.0;
			}
			double inner_sum_impact = 0.0;
			vector<int> inner_scope_context;
			vector<int> inner_node_context;
			vector<ScopeHistory*> inner_context_histories;
			int inner_early_exit_depth;
			int inner_early_exit_node_id;
			FoldHistory* inner_early_exit_fold_history;
			int inner_explore_exit_depth;
			int inner_explore_exit_node_id;
			FoldHistory* inner_explore_exit_fold_history;

			ScopeHistory* scope_history = new ScopeHistory(inner_scope);
			inner_scope->activate(problem,
								  inner_input_vals,
								  inner_inputs_initialized,
								  inner_predicted_score,
								  inner_scale_factor,
								  inner_sum_impact,
								  inner_scope_context,
								  inner_node_context,
								  inner_context_histories,
								  inner_early_exit_depth,
								  inner_early_exit_node_id,
								  inner_early_exit_fold_history,
								  inner_explore_exit_depth,
								  inner_explore_exit_node_id,
								  inner_explore_exit_fold_history,
								  run_helper,
								  scope_history);
			delete scope_history;
		} else {
			problem.perform_action(explore_actions[f_index]);
		}
	}

	vector<int> explore_scope_context;
	vector<int> explore_node_context;
	geometric_distribution<int> num_context_geo_dist(0.5);
	int num_contexts = 1 + potential_exit_depths[random_exit_index] + num_context_geo_dist(generator);
	if (num_contexts > (int)scope_context.size()) {
		num_contexts = (int)scope_context.size();
	}
	for (int c_index = 0; c_index < num_contexts; c_index++) {
		explore_scope_context.push_back(scope_context[(int)scope_context.size()-1-c_index]);
		explore_node_context.push_back(node_context[(int)node_context.size()-1-c_index]);
	}

	run_helper.explore_scope_id = this->id;
	run_helper.explore_node_id = curr_node_id;
	run_helper.explore_scope_context = explore_scope_context;
	run_helper.explore_node_context = explore_node_context;
	run_helper.explore_is_loop = false;
	run_helper.explore_exit_depth = potential_exit_depths[random_exit_index];
	run_helper.explore_next_node_id = potential_next_node_ids[random_exit_index];
	run_helper.explore_is_inner_scope = explore_is_inner_scope;
	run_helper.explore_existing_scope_ids = explore_existing_scope_ids;
	run_helper.explore_actions = explore_actions;
	run_helper.explore_seed_start_predicted_score = predicted_score;
	run_helper.explore_seed_start_scale_factor = scale_factor;
	run_helper.explore_seed_state_vals_snapshot = state_vals;
	run_helper.explore_seed_outer_context_history = new ScopeHistory(context_histories[(int)context_histories.size()-num_contexts]);

	new_explore_exit_depth = potential_exit_depths[random_exit_index];
	new_explore_exit_node_id = potential_next_node_ids[random_exit_index];
}

void Scope::backprop_explore_fold_helper(vector<double>& state_errors,
										 vector<bool>& states_initialized,
										 double target_val,
										 double final_diff,
										 double final_misguess,
										 double final_sum_impact,
										 double& predicted_score,
										 double& scale_factor,
										 double& scale_factor_error,	// unused
										 RunHelper& run_helper,
										 ScopeHistory* history) {
	if (history->explore_fold_history != NULL) {
		Fold* fold = history->explore_fold_history->fold;
		fold->sequence_backprop(state_errors,
								states_initialized,
								target_val,
								final_diff,
								final_misguess,
								final_sum_impact,
								predicted_score,
								scale_factor,
								scale_factor_error,	// unused
								run_helper,
								history->explore_fold_history);

		if (fold->state == FOLD_STATE_EXPERIMENT_FAIL) {
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
		} else if (fold->state == FOLD_STATE_EXPERIMENT_DONE) {
			fold->experiment_to_clean();

			// sequence_scope == this

			Scope* score_scope = solution->scopes[fold->scope_context[0]];
			StateNetwork* existing_score_network = new StateNetwork(0,
																	score_scope->num_states,
																	0,
																	0,
																	20);
			bool fold_is_pass_through;
			if (fold->experiment_result == FOLD_RESULT_BRANCH) {
				fold_is_pass_through = false;
			} else {
				// fold->experiment_result == FOLD_RESULT_REPLACE
				fold_is_pass_through = true;
			}

			if (score_scope->nodes[fold->node_context[0]]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)score_scope->nodes[fold->node_context[0]];

				FoldSequenceNode* new_fold_sequence_node = new FoldSequenceNode(action_node->explore_next_node_id);
				this->nodes.push_back(new_fold_sequence_node);
				int fold_sequence_node_id = (int)this->nodes.size()-1;

				fold->fold_node_scope_id = this->id;
				fold->fold_node_scope_index = fold_sequence_node_id;

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

				fold->fold_node_scope_id = this->id;
				fold->fold_node_scope_index = fold_sequence_node_id;

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
		loop_fold->backprop(state_errors,
							states_initialized,
							target_val,
							final_diff,
							final_misguess,
							final_sum_impact,
							predicted_score,
							scale_factor,
							scale_factor_error,	// unused
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
			loop_fold->experiment_to_clean();

			if (this->nodes[loop_fold->node_context[0]]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->nodes[loop_fold->node_context[0]];

				LoopFoldNode* new_loop_fold_node = new LoopFoldNode(loop_fold,
																	action_node->explore_scope_context,
																	action_node->explore_node_context,
																	action_node->next_node_id);
				this->nodes.push_back(new_loop_fold_node);
				int new_node_id = (int)this->nodes.size()-1;
				action_node->next_node_id = new_node_id;

				loop_fold->fold_node_scope_id = this->id;
				loop_fold->fold_node_scope_index = new_node_id;

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

				loop_fold->fold_node_scope_id = this->id;
				loop_fold->fold_node_scope_index = new_node_id;

				scope_node->explore_scope_context.clear();
				scope_node->explore_node_context.clear();
				scope_node->explore_loop_fold = NULL;
			}
		}
	}

	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT_BACKPROP_DONE;
}

void Scope::add_new_state(int new_state_size,
						  bool initialized_locally) {
	this->num_states += new_state_size;
	for (int s_index = 0; s_index < new_state_size; s_index++) {
		this->is_initialized_locally.push_back(initialized_locally);
	}

	if (is_loop) {
		this->continue_score_network->add_state(new_state_size);
		this->continue_misguess_network->add_state(new_state_size);
		this->halt_score_network->add_state(new_state_size);
		this->halt_misguess_network->add_state(new_state_size);
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			if (action_node->score_network->state_size == this->num_states) {
				// new node added from fold_seqeuence
			} else {
				action_node->score_network->add_state(new_state_size);
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_INNER_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			if (scope_node->score_network->state_size == this->num_states) {
				// new node added from fold_seqeuence
			} else {
				scope_node->score_network->add_state(new_state_size);
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			branch_node->branch_score_network->add_state(new_state_size);
			branch_node->original_score_network->add_state(new_state_size);
		} else if (this->nodes[n_index]->type == NODE_TYPE_FOLD_SCORE) {
			FoldScoreNode* fold_score_node = (FoldScoreNode*)this->nodes[n_index];
			fold_score_node->existing_score_network->add_state(new_state_size);
		}
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->id << endl;

	output_file << this->num_states << endl;
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		output_file << this->is_initialized_locally[s_index] << endl;
	}

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

		output_file << this->average_score << endl;
		output_file << this->score_variance << endl;
		output_file << this->average_misguess << endl;
		output_file << this->misguess_variance << endl;
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

void Scope::save_for_display(ofstream& output_file) {
	output_file << this->is_loop << endl;

	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;
		this->nodes[n_index]->save_for_display(output_file);
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->halt_score_network_history = NULL;
	this->halt_misguess_network_history = NULL;

	this->explore_iter_index = -1;
	this->explore_node_index = -1;
	this->explore_fold_history = NULL;
	this->explore_loop_fold_history = NULL;

	this->exceeded_depth = false;
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	this->halt_score_network_history = NULL;
	this->halt_misguess_network_history = NULL;

	this->explore_fold_history = NULL;
	this->explore_loop_fold_history = NULL;

	for (int i_index = 0; i_index < (int)original->node_histories.size(); i_index++) {
		this->node_histories.push_back(vector<AbstractNodeHistory*>());
		for (int h_index = 0; h_index < (int)original->node_histories[i_index].size(); h_index++) {
			if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ActionNodeHistory(action_node_history));
			} else if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ScopeNodeHistory(scope_node_history));
			}
		}
	}
}

ScopeHistory::~ScopeHistory() {
	for (int iter_index = 0; iter_index < (int)this->node_histories.size(); iter_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[iter_index].size(); h_index++) {
			delete this->node_histories[iter_index][h_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->starting_state_network_histories.size(); s_index++) {
		if (this->starting_state_network_histories[s_index] != NULL) {
			delete this->starting_state_network_histories[s_index];
		}
	}

	for (int iter_index = 0; iter_index < (int)this->continue_score_network_histories.size(); iter_index++) {
		delete this->continue_score_network_histories[iter_index];
	}

	for (int iter_index = 0; iter_index < (int)this->continue_misguess_network_histories.size(); iter_index++) {
		delete this->continue_misguess_network_histories[iter_index];
	}

	if (this->halt_score_network_history != NULL) {
		delete this->halt_score_network_history;
	}

	if (this->halt_misguess_network_history != NULL) {
		delete this->halt_misguess_network_history;
	}

	if (this->explore_fold_history != NULL) {
		delete this->explore_fold_history;
	}

	if (this->explore_loop_fold_history != NULL) {
		delete this->explore_loop_fold_history;
	}
}
