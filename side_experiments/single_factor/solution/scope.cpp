#include "scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "branch_experiment.h"
#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "layer.h"
#include "loop_experiment.h"
#include "scope_node.h"
#include "score_network.h"
#include "state_network.h"

using namespace std;

// Scope::Scope(int id,
// 			 int num_states,
// 			 vector<bool> state_initialized_locally,
// 			 vector<int> state_family_ids,
// 			 vector<int> state_default_class_ids,
// 			 bool is_loop,
// 			 ScoreNetwork* continue_score_network,
// 			 ScoreNetwork* continue_misguess_network,
// 			 ScoreNetwork* halt_score_network,
// 			 ScoreNetwork* halt_misguess_network) {
// 	this->id = id;
// 	this->num_states = num_states;
// 	this->state_initialized_locally = state_initialized_locally;
// 	this->state_family_ids = state_family_ids;
// 	this->state_default_class_ids = state_default_class_ids;
// 	this->is_loop = is_loop;
// 	this->continue_score_network = continue_score_network;
// 	this->continue_misguess_network = continue_misguess_network;
// 	this->halt_score_network = halt_score_network;
// 	this->halt_misguess_network = halt_misguess_network;
// 	this->furthest_successful_halt = 5;	// simply initializing to 5
// }

// Scope::Scope(ifstream& input_file,
// 			 int id) {
// 	this->id = id;

// 	string num_states_line;
// 	getline(input_file, num_states_line);
// 	this->num_states = stoi(num_states_line);

// 	for (int s_index = 0; s_index < this->num_states; s_index++) {
// 		string state_initialized_locally_line;
// 		getline(input_file, state_initialized_locally_line);
// 		this->state_initialized_locally.push_back(stoi(state_initialized_locally_line));

// 		string state_family_id_line;
// 		getline(input_file, state_family_id_line);
// 		this->state_family_ids.push_back(stoi(state_family_id_line));

// 		string state_default_class_id_line;
// 		getline(input_file, state_default_class_id_line);
// 		this->state_default_class_ids.push_back(stoi(state_default_class_id_line));
// 	}

// 	string is_loop_line;
// 	getline(input_file, is_loop_line);
// 	this->is_loop = stoi(is_loop_line);

// 	if (this->is_loop) {
// 		ifstream continue_score_network_save_file;
// 		continue_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_score.txt");
// 		this->continue_score_network = new ScoreNetwork(continue_score_network_save_file);
// 		continue_score_network_save_file.close();

// 		ifstream continue_misguess_network_save_file;
// 		continue_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_misguess.txt");
// 		this->continue_misguess_network = new ScoreNetwork(continue_misguess_network_save_file);
// 		continue_misguess_network_save_file.close();

// 		ifstream halt_score_network_save_file;
// 		halt_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_score.txt");
// 		this->halt_score_network = new ScoreNetwork(halt_score_network_save_file);
// 		halt_score_network_save_file.close();

// 		ifstream halt_misguess_network_save_file;
// 		halt_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_misguess.txt");
// 		this->halt_misguess_network = new ScoreNetwork(halt_misguess_network_save_file);
// 		halt_misguess_network_save_file.close();

// 		string furthest_successful_halt_line;
// 		getline(input_file, furthest_successful_halt_line);
// 		this->furthest_successful_halt = stoi(furthest_successful_halt_line);
// 	} else {
// 		this->continue_score_network = NULL;
// 		this->continue_misguess_network = NULL;
// 		this->halt_score_network = NULL;
// 		this->halt_misguess_network = NULL;
// 	}

// 	string num_nodes_line;
// 	getline(input_file, num_nodes_line);
// 	int num_nodes = stoi(num_nodes_line);
// 	for (int n_index = 0; n_index < num_nodes; n_index++) {
// 		string node_type_line;
// 		getline(input_file, node_type_line);
// 		int node_type = stoi(node_type_line);

// 		ifstream node_save_file;
// 		node_save_file.open("saves/node_" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
// 		if (node_type == NODE_TYPE_ACTION) {
// 			ActionNode* action_node = new ActionNode(node_save_file,
// 													 this,
// 													 n_index);
// 			this->nodes.push_back(action_node);
// 		} else if (node_type == NODE_TYPE_SCOPE) {
// 			ScopeNode* scope_node = new ScopeNode(node_save_file,
// 												  this,
// 												  n_index);
// 			this->nodes.push_back(scope_node);
// 		} else if (node_type == NODE_TYPE_BRANCH) {
// 			BranchNode* branch_node = new BranchNode(node_save_file,
// 													 this,
// 													 n_index);
// 			this->nodes.push_back(branch_node);
// 		} else {
// 			ExitNode* exit_node = new ExitNode(node_save_file,
// 											   this,
// 											   n_index);
// 			this->nodes.push_back(exit_node);
// 		}
// 		node_save_file.close();
// 	}
// }

// Scope::~Scope() {
// 	if (this->continue_score_network != NULL) {
// 		delete this->continue_score_network;
// 	}

// 	if (this->continue_misguess_network != NULL) {
// 		delete this->continue_misguess_network;
// 	}

// 	if (this->halt_score_network != NULL) {
// 		delete this->halt_score_network;
// 	}

// 	if (this->halt_misguess_network != NULL) {
// 		delete this->halt_misguess_network;
// 	}

// 	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
// 		delete this->nodes[n_index];
// 	}
// }

void Scope::activate(vector<int>& starting_node_ids,
					 vector<vector<double>*>& starting_state_vals,
					 vector<vector<double>>& starting_state_weights,
					 vector<double>& flat_vals,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 int& exit_node_id,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	exit_depth = -1;

	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_depth = true;
		history->exceeded_depth = true;
		return;
	}
	run_helper.curr_depth++;

	if (this->is_loop) {
		int target_iter;
		RemeasureScopeHistory* remeasure_scope_history;
		if (run_helper.phase == UPDATE_PHASE_NONE
				&& this->remeasure_counter > solution->remeasure_target) {
			run_helper.phase = UPDATE_PHASE_REMEASURE;
			run_helper.remeasure_type = REMEASURE_TYPE_SCOPE;
			remeasure_scope_history = new RemeasureScopeHistory(this);
			run_helper.remeasure_scope_history = remeasure_scope_history;

			if (rand()%10 == 0) {
				remeasure_scope_history->train_continue = true;
				target_iter = this->furthest_successful_halt+2;
			} else {
				remeasure_scope_history->train_continue = false;
				target_iter = rand()%(this->furthest_successful_halt+3);
			}
		} else {
			target_iter = -1;
		}

		bool is_explore_iter_save = run_helper.is_explore_iter;
		int explore_iter;
		if (run_helper.is_explore_iter) {
			explore_iter = rand()%this->furthest_successful_halt;
		} else {
			explore_iter = -1;
		}

		int iter_index = 0;
		while (true) {
			vector<vector<double>*> state_vals(this->context_indexes.size()+1);
			state_vals[0] = context[0].state_vals;
			for (int c_index = 0; c_index < (int)this->context_indexes.size(); c_index++) {
				state_vals[1+c_index] = context[context.size()-this->context_indexes.size()+c_index].state_vals;
			}

			if (target_iter != -1) {
				if (iter_index == target_iter) {
					if (!remeasure_scope_history->train_continue) {
						ScoreNetworkHistory* halt_score_network_history = new ScoreNetworkHistory(this->halt_score_network);
						this->halt_score_network->activate(state_vals,
														   halt_score_network_history);
						remeasure_scope_history->halt_score_network_history = halt_score_network_history;
						remeasure_scope_history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

						ScoreNetworkHistory* halt_misguess_network_history = new ScoreNetworkHistory(this->halt_misguess_network);
						this->halt_misguess_network->activate(state_vals,
															  halt_misguess_network_history);
						remeasure_scope_history->halt_misguess_network_history = halt_misguess_network_history;
						remeasure_scope_history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];
					}

					break;
				} else {
					if (remeasure_scope_history->train_continue) {
						ScoreNetworkHistory* continue_score_network_history = new ScoreNetworkHistory(this->continue_score_network);
						this->continue_score_network->activate(state_vals,
															   continue_score_network_history);
						remeasure_scope_history->continue_score_network_histories.push_back(continue_score_network_history);
						remeasure_scope_history->continue_score_network_outputs.push_back(this->continue_score_network->output->acti_vals[0]);

						ScoreNetworkHistory* continue_misguess_network_history = new ScoreNetworkHistory(this->continue_misguess_network);
						this->continue_misguess_network->activate(state_vals,
																  continue_misguess_network_history);
						remeasure_scope_history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
						remeasure_scope_history->continue_misguess_network_outputs.push_back(this->continue_misguess_network->output->acti_vals[0]);

						this->halt_score_network->activate(state_vals);
						remeasure_scope_history->halt_score_snapshots.push_back(this->halt_score_network->output->acti_vals[0]);

						this->halt_misguess_network->activate(state_vals);
						remeasure_scope_history->halt_misguess_snapshots.push_back(this->halt_misguess_network->output->acti_vals[0]);
					}

					// continue
				}
			} else {
				if (iter_index > this->furthest_successful_halt+3) {
					break;
				} else {
					this->continue_score_network->activate(state_vals);
					this->halt_score_network->activate(state_vals);
					double score_diff = this->continue_score_network->output->acti_vals[0]
						- this->halt_score_network->output->acti_vals[0];
					double score_val = score_diff / solution->average_misguess;
					if (score_val > 0.1) {
						// continue
					} else if (score_val < -0.1) {
						if (iter_index > this->furthest_successful_halt) {
							this->furthest_successful_halt = iter_index;
						}
						break;
					} else {
						this->continue_misguess_network->activate(state_vals);
						this->halt_misguess_network->activate(state_vals);
						double misguess_diff = this->continue_misguess_network->output->acti_vals[0]
							- this->halt_misguess_network->output->acti_vals[0];
						double misguess_val = misguess_diff / solution->misguess_standard_deviation;
						if (misguess_val < -0.1) {
							// continue
						} else if (misguess_val > 0.1) {
							if (iter_index > this->furthest_successful_halt) {
								this->furthest_successful_halt = iter_index;
							}
							break;
						} else {
							// halt if no strong signal either way
							break;
						}
					}
				}
			}

			if (iter_index == explore_iter) {
				run_helper.is_explore_iter = true;
			} else {
				run_helper.is_explore_iter = false;
			}

			int curr_node_id = 0;
			history->node_histories.push_back(vector<AbstractNodeHistory*>());
			while (true) {
				if (curr_node_id == -1 || exit_depth != -1) {
					break;
				}

				handle_node_activate_helper(iter_index,
											curr_node_id,
											flat_vals,
											context,
											exit_depth,
											exit_node_id,
											run_helper,
											history);
			}

			iter_index++;
		}

		run_helper.is_explore_iter = is_explore_iter_save;
	} else {
		history->node_histories.push_back(vector<AbstractNodeHistory*>());

		int curr_node_id = starting_node_ids[0];
		starting_node_ids.erase(starting_node_ids.begin());
		if (starting_node_ids.size() > 0) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

			int inner_exit_depth;
			int inner_exit_node_id;

			scope_node->halfway_activate(starting_node_ids,
										 starting_state_vals,
										 starting_state_weights,
										 flat_vals,
										 context,
										 inner_exit_depth,
										 inner_exit_node_id,
										 run_helper,
										 history->node_histories[0]);

			if (inner_exit_depth == -1) {
				curr_node_id = scope_node->next_node_id;

				if (scope_node->explore != NULL
						&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
					bool matches_context = true;
					if (scope_node->explore->scope_context.size() > context.size()) {
						matches_context = false;
					} else {
						for (int c_index = 0; c_index < (int)scope_node->explore->scope_context.size()-1; c_index++) {
							if (scope_node->explore->scope_context[c_index] != 
										context[context.size()-scope_node->explore->scope_context.size()+c_index].scope_id
									|| scope_node->explore->node_context[c_index] !=
										context[context.size()-scope_node->explore->scope_context.size()+c_index].node_id) {
								matches_context = false;
								break;
							}
						}
					}
					if (matches_context) {
						run_helper.explore_seen = true;
						if (run_helper.is_explore_iter) {
							int explore_exit_depth;
							int explore_exit_node_id;

							ExploreHistory* explore_history = new ExploreHistory(scope_node->explore);
							run_helper.explore_history = explore_history;
							scope_node->explore->activate(flat_vals,
														  context,
														  explore_exit_depth,
														  explore_exit_node_id,
														  run_helper,
														  explore_history);

							if (explore_exit_depth == 0) {
								curr_node_id = explore_exit_node_id;
							} else {
								exit_depth = explore_exit_depth;
								exit_node_id = explore_exit_node_id;
							}
						}
					}
				}
			} else if (inner_exit_depth == 0) {
				curr_node_id = inner_exit_node_id;
			} else {
				exit_depth = inner_exit_depth-1;
				exit_node_id = inner_exit_node_id;
			}
		}

		while (true) {
			if (curr_node_id == -1 || exit_depth != -1) {
				break;
			}

			handle_node_activate_helper(0,
										curr_node_id,
										flat_vals,
										context,
										exit_depth,
										exit_node_id,
										run_helper,
										history);
		}
	}

	for (int i_index = 0; i_index < (int)this->initialized_locally_indexes.size(); i_index++) {
		double state_val = context.back().state_vals->at(this->initialized_locally_indexes[i_index]);
		double state_weight = context.back().state_weights[this->initialized_locally_indexes[i_index]];

		history->initialized_locally_val_snapshots.push_back(state_val);
		history->initialized_locally_weight_snapshots.push_back(state_weight);

		run_helper.predicted_score += run_helper.scale_factor*state_weight*state_val*this->ending_score_scales[i_index]->weight;
	}

	run_helper.curr_depth--;
}

void Scope::handle_node_activate_helper(int iter_index,
										int& curr_node_id,
										vector<double>& flat_vals,
										vector<ContextLayer>& context,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		action_node->activate(flat_vals,
							  context,
							  run_helper,
							  history->node_histories);

		curr_node_id = action_node->next_node_id;

		if (action_node->explore != NULL
				&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			bool matches_context = true;
			if (action_node->explore->scope_context.size() > context.size()) {
				matches_context = false;
			} else {
				for (int c_index = 0; c_index < (int)action_node->explore->scope_context.size()-1; c_index++) {
					if (action_node->explore->scope_context[c_index] != 
								context[context.size()-action_node->explore->scope_context.size()+c_index].scope_id
							|| action_node->explore->node_context[c_index] !=
								context[context.size()-action_node->explore->scope_context.size()+c_index].node_id) {
						matches_context = false;
						break;
					}
				}
			}
			if (matches_context) {
				run_helper.explore_seen = true;
				if (run_helper.is_explore_iter) {
					int explore_exit_depth;
					int explore_exit_node_id;

					ExploreHistory* explore_history = new ExploreHistory(action_node->explore);
					run_helper.explore_history = explore_history;
					action_node->explore->activate(flat_vals,
												   context,
												   explore_exit_depth,
												   explore_exit_node_id,
												   run_helper,
												   explore_history);

					if (explore_exit_depth == 0) {
						curr_node_id = explore_exit_node_id;
					} else {
						exit_depth = explore_exit_depth;
						exit_node_id = explore_exit_node_id;
					}
				}
			}
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		int inner_exit_depth;
		int inner_exit_node_id;

		scope_node->activate(flat_vals,
							 context,
							 inner_exit_depth,
							 inner_exit_node_id,
							 run_helper,
							 history->node_histories);

		if (inner_exit_depth == -1) {
			curr_node_id = scope_node->next_node_id;

			if (scope_node->explore != NULL
					&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
				bool matches_context = true;
				if (scope_node->explore->scope_context.size() > context.size()) {
					matches_context = false;
				} else {
					for (int c_index = 0; c_index < (int)scope_node->explore->scope_context.size()-1; c_index++) {
						if (scope_node->explore->scope_context[c_index] != 
									context[context.size()-scope_node->explore->scope_context.size()+c_index].scope_id
								|| scope_node->explore->node_context[c_index] !=
									context[context.size()-scope_node->explore->scope_context.size()+c_index].node_id) {
							matches_context = false;
							break;
						}
					}
				}
				if (matches_context) {
					run_helper.explore_seen = true;
					if (run_helper.is_explore_iter) {
						int explore_exit_depth;
						int explore_exit_node_id;

						ExploreHistory* explore_history = new ExploreHistory(scope_node->explore);
						run_helper.explore_node = scope_node;
						run_helper.explore_history = explore_history;
						scope_node->explore->activate(flat_vals,
													  context,
													  explore_exit_depth,
													  explore_exit_node_id,
													  run_helper,
													  explore_history);

						if (explore_exit_depth == 0) {
							curr_node_id = explore_exit_node_id;
						} else {
							exit_depth = explore_exit_depth;
							exit_node_id = explore_exit_node_id;
						}
					}
				}
			}
		} else if (inner_exit_depth == 0) {
			curr_node_id = inner_exit_node_id;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node_id = inner_exit_node_id;
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		bool& is_branch;
		branch_node->activate(context,
							  run_helper,
							  is_branch,
							  history->node_histories);

		if (is_branch) {
			curr_node_id = branch_node->branch_next_node_id;
		} else {
			curr_node_id = branch_node->original_next_node_id;
		}
	} else {
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		exit_node->activate(context);

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth;
			exit_node_id = exit_node->exit_node_id;
		}
	}
}

void Scope::backprop(double& scale_factor_error,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	double predicted_score_error = run_helper.target_val - run_helper.predicted_score;
	for (int i_index = 0; i_index < (int)this->initialized_locally_indexes.size(); i_index++) {
		double state_val = history->initialized_locally_val_snapshots[i_index];
		double state_weight = history->initialized_locally_weight_snapshots[i_index];

		scale_factor_error += state_weight*state_val*this->ending_score_scales[i_index]->weight*predicted_score_error;

		if (run_helper.phase == UPDATE_PHASE_NONE) {
			this->ending_score_scales[i_index]->backprop(
				run_helper.scale_factor*state_weight*state_val*predicted_score_error,
				0.0002);
		}

		run_helper.predicted_score -= run_helper.scale_factor*state_weight*state_val*this->ending_score_scales[i_index]->weight;
	}

	if (history->exceeded_depth) {
		return;
	}

	for (int i_index = (int)history->node_histories.size()-1; i_index >= 0; i_index--) {
		for (int h_index = (int)history->node_histories[i_index].size()-1; h_index >= 0; h_index--) {
			handle_node_backprop_helper(i_index,
										h_index,
										scale_factor_error,
										run_helper,
										history);
		}
	}
}

void Scope::handle_node_backprop_helper(int iter_index,
										int h_index,
										double& scale_factor_error,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_SCOPE) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[iter_index][h_index];
		ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
		scope_node->backprop(scale_factor_error,
							 run_helper,
							 scope_node_history);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_BRANCH) {
		BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history->node_histories[iter_index][h_index];
		BranchNode* branch_node = (BranchNode*)branch_node_history->node;
		branch_node->backprop(scale_factor_error,
							  run_helper,
							  branch_node_history);
	}
	// if NODE_TYPE_ACTION, do nothing
}

void Scope::remeasure_backprop(RunHelper& run_helper,
							   RemeasureScopeHistory* history) {
	if (!history->train_continue) {
		double halt_predicted_score_error = run_helper.target_val - history->halt_score_network_output;
		this->halt_score_network->backprop_weights_with_no_error_signal(
			halt_predicted_score_error,
			0.002,
			history->halt_score_network_history);

		double halt_misguess_error = run_helper.final_misguess - history->halt_misguess_network_output;
		this->halt_misguess_network->backprop_weights_with_no_error_signal(
			halt_misguess_error,
			0.002,
			history->halt_misguess_network_history);
	} else {
		for (int i_index = (int)history->node_histories.size()-1; i_index >= 0; i_index--) {
			double best_halt_score = history->halt_score_network_output;
			double best_halt_misguess = history->halt_misguess_network_output;
			// back to front
			for (int ii_index = (int)history->node_histories.size()-1; ii_index >= i_index+1; ii_index--) {
				double score_diff = history->halt_score_snapshots[ii_index] - best_halt_score;
				double score_val = score_diff / solution->average_misguess;
				if (score_val > 0.1) {
					best_halt_score = history->halt_score_snapshots[ii_index];
					best_halt_misguess = history->halt_misguess_snapshots[ii_index];
				} else if (score_val < -0.1) {
					continue;
				} else {
					double misguess_diff = history->halt_misguess_snapshots[ii_index] - best_halt_misguess;
					double misguess_val = misguess_diff / solution->misguess_standard_deviation;
					if (misguess_val < -0.1) {
						best_halt_score = history->halt_score_snapshots[ii_index];
						best_halt_misguess = history->halt_misguess_snapshots[ii_index];
					} else if (misguess_val > 0.1) {
						continue;
					} else {
						// use earlier iter if no strong signal either way
						best_halt_score = history->halt_score_snapshots[ii_index];
						best_halt_misguess = history->halt_misguess_snapshots[ii_index];
					}
				}
			}

			double continue_predicted_score_error = best_halt_score - history->continue_score_network_outputs[i_index];
			this->continue_score_network->backprop_weights_with_no_error_signal(
				continue_predicted_score_error,
				0.002,
				history->continue_score_network_histories[i_index]);

			double continue_misguess_error = best_halt_misguess - history->continue_misguess_network_outputs[i_index];
			this->continue_misguess_network->backprop_weights_with_no_error_signal(
				continue_misguess_error,
				0.002,
				history->continue_misguess_network_histories[i_index]);
		}
	}
}

// void Scope::add_state(bool initialized_locally,
// 					  int family_id,
// 					  int default_class_id) {
// 	this->num_states++;
// 	this->state_initialized_locally.push_back(initialized_locally);
// 	this->state_family_ids.push_back(family_id);
// 	this->state_default_class_ids.push_back(default_class_id);

// 	if (this->is_loop) {
// 		this->continue_score_network->add_state();
// 		this->continue_misguess_network->add_state();
// 		this->halt_score_network->add_state();
// 		this->halt_misguess_network->add_state();
// 	}

// 	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
// 		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
// 			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
// 			action_node->score_network->add_state();
// 			action_node->misguess_network->add_state();
// 		} else if (this->nodes[n_index]->type == NODE_TYPE_BRANCH) {
// 			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
// 			if (branch_node->branch_score_network != NULL) {
// 				branch_node->branch_score_network->add_state();
// 			}
// 			if (branch_node->branch_misguess_network != NULL) {
// 				branch_node->branch_misguess_network->add_state();
// 			}
// 			if (branch_node->original_score_network != NULL) {
// 				branch_node->original_score_network->add_state();
// 			}
// 			if (branch_node->original_misguess_network != NULL) {
// 				branch_node->original_misguess_network->add_state();
// 			}
// 		}
// 	}
// }

// void Scope::save(ofstream& output_file) {
// 	output_file << this->num_states << endl;
// 	for (int s_index = 0; s_index < this->num_states; s_index++) {
// 		output_file << this->state_initialized_locally[s_index] << endl;
// 		output_file << this->state_family_ids[s_index] << endl;
// 		output_file << this->state_default_class_ids[s_index] << endl;
// 	}

// 	output_file << this->is_loop << endl;

// 	if (this->is_loop) {
// 		ofstream continue_score_network_save_file;
// 		continue_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_score.txt");
// 		this->continue_score_network->save(continue_score_network_save_file);
// 		continue_score_network_save_file.close();

// 		ofstream continue_misguess_network_save_file;
// 		continue_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_misguess.txt");
// 		this->continue_misguess_network->save(continue_misguess_network_save_file);
// 		continue_misguess_network_save_file.close();

// 		ofstream halt_score_network_save_file;
// 		halt_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_score.txt");
// 		this->halt_score_network->save(halt_score_network_save_file);
// 		halt_score_network_save_file.close();

// 		ofstream halt_misguess_network_save_file;
// 		halt_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_misguess.txt");
// 		this->halt_misguess_network->save(halt_misguess_network_save_file);
// 		halt_misguess_network_save_file.close();

// 		output_file << this->furthest_successful_halt << endl;
// 	}

// 	output_file << this->nodes.size() << endl;
// 	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
// 		output_file << this->nodes[n_index]->type << endl;

// 		ofstream node_save_file;
// 		node_save_file.open("saves/node_" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
// 		this->nodes[n_index]->save(node_save_file);
// 		node_save_file.close();
// 	}
// }

// void Scope::save_for_display(ofstream& output_file) {

// }

// ScopeHistory::ScopeHistory(Scope* scope) {
// 	this->scope = scope;

// 	this->exceeded_depth = false;

// 	this->halt_score_network_history = NULL;
// 	this->halt_misguess_network_history = NULL;
// }

// ScopeHistory::ScopeHistory(ScopeHistory* original) {
// 	this->scope = original->scope;

// 	for (int i_index = 0; i_index < (int)original->node_histories.size(); i_index++) {
// 		this->node_histories.push_back(vector<AbstractNodeHistory*>());
// 		for (int h_index = 0; h_index < (int)original->node_histories[i_index].size(); h_index++) {
// 			if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
// 				ActionNodeHistory* action_node_history = (ActionNodeHistory*)original->node_histories[i_index][h_index];
// 				this->node_histories.back().push_back(new ActionNodeHistory(action_node_history));
// 			} else if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
// 				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)original->node_histories[i_index][h_index];
// 				this->node_histories.back().push_back(new ScopeNodeHistory(scope_node_history));
// 			}
// 		}
// 	}
// }

// ScopeHistory::~ScopeHistory() {
// 	for (int iter_index = 0; iter_index < (int)this->continue_score_network_histories.size(); iter_index++) {
// 		delete this->continue_score_network_histories[iter_index];
// 	}

// 	for (int iter_index = 0; iter_index < (int)this->continue_misguess_network_histories.size(); iter_index++) {
// 		delete this->continue_misguess_network_histories[iter_index];
// 	}

// 	if (this->halt_score_network_history != NULL) {
// 		delete this->halt_score_network_history;
// 	}

// 	if (this->halt_misguess_network_history != NULL) {
// 		delete this->halt_misguess_network_history;
// 	}

// 	for (int iter_index = 0; iter_index < (int)this->node_histories.size(); iter_index++) {
// 		for (int h_index = 0; h_index < (int)this->node_histories[iter_index].size(); h_index++) {
// 			delete this->node_histories[iter_index][h_index];
// 		}
// 	}
// }
