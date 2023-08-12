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

Scope::Scope(int id,
			 int num_states,
			 vector<bool> state_initialized_locally,
			 vector<int> state_family_ids,
			 vector<int> state_default_class_ids,
			 bool is_loop,
			 ScoreNetwork* continue_score_network,
			 ScoreNetwork* continue_misguess_network,
			 ScoreNetwork* halt_score_network,
			 ScoreNetwork* halt_misguess_network) {
	this->id = id;
	this->num_states = num_states;
	this->state_initialized_locally = state_initialized_locally;
	this->state_family_ids = state_family_ids;
	this->state_default_class_ids = state_default_class_ids;
	this->is_loop = is_loop;
	this->continue_score_network = continue_score_network;
	this->continue_misguess_network = continue_misguess_network;
	this->halt_score_network = halt_score_network;
	this->halt_misguess_network = halt_misguess_network;
	this->furthest_successful_halt = 5;	// simply initializing to 5
}

Scope::Scope(ifstream& input_file,
			 int id) {
	this->id = id;

	string num_states_line;
	getline(input_file, num_states_line);
	this->num_states = stoi(num_states_line);

	for (int s_index = 0; s_index < this->num_states; s_index++) {
		string state_initialized_locally_line;
		getline(input_file, state_initialized_locally_line);
		this->state_initialized_locally.push_back(stoi(state_initialized_locally_line));

		string state_family_id_line;
		getline(input_file, state_family_id_line);
		this->state_family_ids.push_back(stoi(state_family_id_line));

		string state_default_class_id_line;
		getline(input_file, state_default_class_id_line);
		this->state_default_class_ids.push_back(stoi(state_default_class_id_line));
	}

	string is_loop_line;
	getline(input_file, is_loop_line);
	this->is_loop = stoi(is_loop_line);

	if (this->is_loop) {
		ifstream continue_score_network_save_file;
		continue_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_score.txt");
		this->continue_score_network = new ScoreNetwork(continue_score_network_save_file);
		continue_score_network_save_file.close();

		ifstream continue_misguess_network_save_file;
		continue_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_continue_misguess.txt");
		this->continue_misguess_network = new ScoreNetwork(continue_misguess_network_save_file);
		continue_misguess_network_save_file.close();

		ifstream halt_score_network_save_file;
		halt_score_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_score.txt");
		this->halt_score_network = new ScoreNetwork(halt_score_network_save_file);
		halt_score_network_save_file.close();

		ifstream halt_misguess_network_save_file;
		halt_misguess_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_halt_misguess.txt");
		this->halt_misguess_network = new ScoreNetwork(halt_misguess_network_save_file);
		halt_misguess_network_save_file.close();

		string furthest_successful_halt_line;
		getline(input_file, furthest_successful_halt_line);
		this->furthest_successful_halt = stoi(furthest_successful_halt_line);
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
													 this,
													 n_index);
			this->nodes.push_back(action_node);
		} else if (node_type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = new ScopeNode(node_save_file,
												  this,
												  n_index);
			this->nodes.push_back(scope_node);
		} else if (node_type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = new BranchNode(node_save_file,
													 this,
													 n_index);
			this->nodes.push_back(branch_node);
		} else {
			ExitNode* exit_node = new ExitNode(node_save_file,
											   this,
											   n_index);
			this->nodes.push_back(exit_node);
		}
		node_save_file.close();
	}
}

Scope::~Scope() {
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

void Scope::activate(vector<int>& starting_node_ids,
					 vector<vector<double>*>& starting_state_vals,
					 vector<vector<bool>>& starting_states_initialized,
					 vector<double>& flat_vals,
					 vector<ForwardContextLayer>& context,
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

	vector<bool> was_initialized_locally(this->num_states, false);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		if (this->state_initialized_locally[s_index]) {
			if (!context.back().states_initialized[s_index]) {
				was_initialized_locally[s_index] = true;
				context.back().states_initialized[s_index] = true;
			}
		}
	}

	bool experiment_variables_initialized = false;
	vector<vector<StateNetwork*>>* experiment_scope_state_networks;
	vector<ScoreNetwork*>* experiment_scope_score_networks;
	int experiment_scope_distance;

	if (this->is_loop) {
		int target_iter;
		if (run_helper.can_train_loops && rand()%10 == 0) {
			if (rand()%10 == 0) {
				history->train_continue = true;
				target_iter = this->furthest_successful_halt+2;
			} else {
				history->train_continue = false;
				target_iter = rand()%(this->furthest_successful_halt+3);
			}
		} else {
			history->train_continue = false;
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
			ScoreNetworkHistory* continue_score_network_history = new ScoreNetworkHistory(this->continue_score_network);
			this->continue_score_network->activate(*(context.back().state_vals),
												   continue_score_network_history);
			double continue_score = run_helper.scale_factor*this->continue_score_network->output->acti_vals[0];

			ScoreNetworkHistory* continue_misguess_network_history = new ScoreNetworkHistory(this->continue_misguess_network);
			this->continue_misguess_network->activate(*(context.back().state_vals),
													  continue_misguess_network_history);

			ScoreNetworkHistory* halt_score_network_history = new ScoreNetworkHistory(this->halt_score_network);
			this->halt_score_network->activate(*(context.back().state_vals),
											   halt_score_network_history);
			double halt_score = run_helper.scale_factor*this->halt_score_network->output->acti_vals[0];

			ScoreNetworkHistory* halt_misguess_network_history = new ScoreNetworkHistory(this->halt_misguess_network);
			this->halt_misguess_network->activate(*(context.back().state_vals),
												  halt_misguess_network_history);

			bool is_halt;
			if (iter_index == target_iter) {
				is_halt = true;
			} else if (iter_index > this->furthest_successful_halt+3) {
				is_halt = true;
			} else {
				if (target_iter != -1) {
					is_halt = false;
				} else {
					double score_diff = continue_score - halt_score;
					double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
					if (score_val > 0.1) {
						is_halt = false;
					} else if (score_val < -0.1) {
						is_halt = true;

						if (iter_index > this->furthest_successful_halt) {
							this->furthest_successful_halt = iter_index;
						}
					} else {
						double misguess_diff = this->continue_misguess_network->output->acti_vals[0]
							- this->halt_misguess_network->output->acti_vals[0];
						double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
						if (misguess_val < -0.1) {
							is_halt = false;
						} else if (misguess_val > 0.1) {
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
			}

			if (is_halt) {
				delete continue_score_network_history;
				delete continue_misguess_network_history;

				history->ending_state_vals_snapshot = *(context.back().state_vals);

				history->halt_score_network_history = halt_score_network_history;
				history->halt_score_network_output = this->halt_score_network->output->acti_vals[0];

				history->halt_misguess_network_history = halt_misguess_network_history;
				history->halt_misguess_network_output = this->halt_misguess_network->output->acti_vals[0];

				break;
			} else {
				delete halt_score_network_history;
				delete halt_misguess_network_history;

				history->iter_state_vals_snapshots.push_back(*(context.back().state_vals));

				history->continue_score_network_histories.push_back(continue_score_network_history);
				history->continue_score_network_outputs.push_back(this->continue_score_network->output->acti_vals[0]);

				history->halt_score_snapshots.push_back(
					run_helper.predicted_score + run_helper.scale_factor*this->halt_score_network->output->acti_vals[0]);

				history->continue_misguess_network_histories.push_back(continue_misguess_network_history);
				history->continue_misguess_network_outputs.push_back(this->continue_misguess_network->output->acti_vals[0]);

				history->halt_misguess_snapshots.push_back(this->halt_misguess_network->output->acti_vals[0]);

				// continue
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
											experiment_variables_initialized,
											experiment_scope_state_networks,
											experiment_scope_score_networks,
											experiment_scope_distance,
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
			context.back().node_id = 0;

			ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

			int inner_exit_depth;
			int inner_exit_node_id;

			ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
			history->node_histories[0].push_back(node_history);
			scope_node->halfway_activate(starting_node_ids,
										 starting_state_vals,
										 starting_states_initialized,
										 flat_vals,
										 context,
										 inner_exit_depth,
										 inner_exit_node_id,
										 run_helper,
										 node_history);

			context.back().node_id = -1;

			if (inner_exit_depth == -1) {
				curr_node_id = scope_node->next_node_id;
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
										experiment_variables_initialized,
										experiment_scope_state_networks,
										experiment_scope_score_networks,
										experiment_scope_distance,
										exit_depth,
										exit_node_id,
										run_helper,
										history);
		}
	}

	if (run_helper.experiment_on_path) {
		run_helper.experiment_context_index--;
		if (run_helper.experiment_context_index == 0) {
			run_helper.experiment_on_path = false;
		}
	}

	for (int s_index = 0; s_index < this->num_states; s_index++) {
		if (was_initialized_locally[s_index]) {
			run_helper.last_seen_vals[this->state_default_class_ids[s_index]] = context.back().state_vals->at(s_index);
		}
	}

	run_helper.curr_depth--;
}

void Scope::handle_node_activate_helper(int iter_index,
										int& curr_node_id,
										vector<double>& flat_vals,
										vector<ForwardContextLayer>& context,
										bool& experiment_variables_initialized,
										vector<vector<StateNetwork*>>*& experiment_scope_state_networks,
										vector<ScoreNetwork*>*& experiment_scope_score_networks,
										int& experiment_scope_distance,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (this->nodes[curr_node_id]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->nodes[curr_node_id];

		if (!experiment_variables_initialized) {
			experiment_variables_helper(experiment_variables_initialized,
										experiment_scope_state_networks,
										experiment_scope_score_networks,
										experiment_scope_distance,
										run_helper);
		}

		ActionNodeHistory* node_history = new ActionNodeHistory(action_node);
		history->node_histories[iter_index].push_back(node_history);
		action_node->activate(flat_vals,
							  context,
							  experiment_scope_state_networks,
							  experiment_scope_score_networks,
							  experiment_scope_distance,
							  run_helper,
							  node_history);

		curr_node_id = action_node->next_node_id;

		if (action_node->is_explore
				&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			run_helper.explore_seen = true;
			if (run_helper.is_explore_iter) {
				if (action_node->experiment != NULL) {
					bool matches_context = true;
					if (action_node->experiment->scope_context.size() > context.size()) {
						matches_context = false;
					} else {
						for (int c_index = 0; c_index < (int)action_node->experiment->scope_context.size()-1; c_index++) {
							if (action_node->experiment->scope_context[c_index] != 
										context[context.size()-action_node->experiment->scope_context.size()+c_index].scope_id
									|| action_node->experiment->node_context[c_index] !=
										context[context.size()-action_node->experiment->scope_context.size()+c_index].node_id) {
								matches_context = false;
								break;
							}
						}
					}
					if (matches_context) {
						if (action_node->experiment->type == EXPERIMENT_TYPE_BRANCH) {
							BranchExperiment* branch_experiment = (BranchExperiment*)action_node->experiment;

							BranchExperimentHistory* branch_experiment_history = new BranchExperimentHistory(branch_experiment);
							node_history->experiment_history = branch_experiment_history;
							branch_experiment->activate(flat_vals,
														context,
														run_helper,
														branch_experiment_history);

							if (run_helper.explore_phase == EXPLORE_PHASE_WRAPUP
									&& !branch_experiment_history->is_branch) {
								curr_node_id = action_node->next_node_id;
							} else {
								if (branch_experiment->exit_depth == 0) {
									curr_node_id = branch_experiment->exit_node_id;
								} else {
									exit_depth = branch_experiment->exit_depth;
									exit_node_id = branch_experiment->exit_node_id;
								}
							}
						} else {
							LoopExperiment* loop_experiment = (LoopExperiment*)action_node->experiment;

							LoopExperimentHistory* loop_experiment_history = new LoopExperimentHistory(loop_experiment);
							node_history->experiment_history = loop_experiment_history;
							loop_experiment->activate(flat_vals,
													  context,
													  run_helper,
													  loop_experiment_history);

							curr_node_id = action_node->next_node_id;
						}
					}
				} else {
					// TODO: explore

				}
			}
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)this->nodes[curr_node_id];

		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[iter_index].push_back(node_history);
		scope_node->activate(flat_vals,
							 context,
							 inner_exit_depth,
							 inner_exit_node_id,
							 run_helper,
							 node_history);

		if (inner_exit_depth == -1) {
			curr_node_id = scope_node->next_node_id;
		} else if (inner_exit_depth == 0) {
			curr_node_id = inner_exit_node_id;
		} else {
			exit_depth = inner_exit_depth-1;
			exit_node_id = inner_exit_node_id;
		}
	} else if (this->nodes[curr_node_id]->type == NODE_TYPE_BRANCH) {
		BranchNode* branch_node = (BranchNode*)this->nodes[curr_node_id];

		BranchNodeHistory* node_history = new BranchNodeHistory(branch_node);
		history->node_histories[iter_index].push_back(node_history);
		branch_node->activate(context,
							  run_helper,
							  node_history);

		if (node_history->is_branch) {
			curr_node_id = branch_node->branch_next_node_id;
		} else {
			curr_node_id = branch_node->original_next_node_id;
		}
	} else {
		// this->nodes[curr_node_id]->type == NODE_TYPE_EXIT
		ExitNode* exit_node = (ExitNode*)this->nodes[curr_node_id];

		ExitNodeHistory* node_history = new ExitNodeHistory(exit_node);
		history->node_histories[iter_index].push_back(node_history);
		exit_node->activate(context,
							run_helper,
							node_history);

		if (exit_node->exit_depth == 0) {
			curr_node_id = exit_node->exit_node_id;
		} else {
			exit_depth = exit_node->exit_depth;
			exit_node_id = exit_node->exit_node_id;
		}
	}
}

void Scope::experiment_variables_helper(
		bool& experiment_variables_initialized,
		vector<vector<StateNetwork*>>*& experiment_scope_state_networks,
		vector<ScoreNetwork*>*& experiment_scope_score_networks,
		int& experiment_scope_distance,
		RunHelper& run_helper) {
	if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT) {
		map<int, vector<vector<StateNetwork*>>>::iterator state_it = run_helper.experiment->state_networks.find(this->id);
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);

		if (state_it == run_helper.experiment->state_networks.end()) {
			state_it = run_helper.experiment->state_networks.insert({this->id, vector<vector<StateNetwork*>>()}).first;
			score_it = run_helper.experiment->score_networks.insert({this->id, vector<ScoreNetwork*>()}).first;

			run_helper.experiment->scope_furthest_layer_seen_in[this->id] = run_helper.experiment_context_index;
			if (run_helper.experiment->type == EXPERIMENT_TYPE_BRANCH) {
				BranchExperiment* branch_experiment = (BranchExperiment*)run_helper.experiment;
				run_helper.experiment->scope_steps_seen_in[this->id] = vector<bool>(branch_experiment->num_steps, false);
			} else {
				// run_helper.experiment->type == EXPERIMENT_TYPE_LOOP
				run_helper.experiment->scope_steps_seen_in[this->id] = vector<bool>(1, false);
			}
		}

		int size_diff = (int)this->nodes.size() - (int)state_it->second.size();
		state_it->second.insert(state_it->second.end(), size_diff, vector<StateNetwork*>());
		score_it->second.insert(score_it->second.end(), size_diff, NULL);

		map<int, int>::iterator layer_seen_in_it = run_helper.experiment->scope_furthest_layer_seen_in.find(this->id);
		if (run_helper.experiment_context_index < layer_seen_in_it->second) {
			layer_seen_in_it->second = run_helper.experiment_context_index;

			int new_furthest_distance = (int)run_helper.experiment->scope_context.size()+2 - run_helper.experiment_context_index;
			for (int n_index = 0; n_index < (int)state_it->second.size(); n_index++) {
				if (state_it->second[n_index].size() != 0) {
					for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
						state_it->second[n_index][s_index]->update_lasso_weights(new_furthest_distance);
					}
					score_it->second[n_index]->update_lasso_weights(new_furthest_distance);
				}
			}
		}

		if (run_helper.experiment_step_index != -1) {
			run_helper.experiment->scope_steps_seen_in[this->id][run_helper.experiment_step_index] = true;
		}

		experiment_scope_state_networks = &(state_it->second);
		experiment_scope_score_networks = &(score_it->second);
		experiment_scope_distance = (int)run_helper.experiment->scope_context.size()+2 - layer_seen_in_it->second;

		experiment_variables_initialized = true;
	} else if (run_helper.explore_phase == EXPLORE_PHASE_MEASURE
			|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
		map<int, vector<vector<StateNetwork*>>>::iterator state_it = run_helper.experiment->state_networks.find(this->id);
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);
		if (state_it != run_helper.experiment->state_networks.end()) {
			experiment_scope_state_networks = &(state_it->second);
			experiment_scope_score_networks = &(score_it->second);
		} else {
			experiment_scope_state_networks = NULL;
			experiment_scope_score_networks = NULL;
		}

		if (run_helper.experiment->state == EXPERIMENT_STATE_SECOND_CLEAN) {
			for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
				set<int>::iterator needed_it = run_helper.experiment->scope_additions_needed[s_index].find(this->id);
				if (needed_it != run_helper.experiment->scope_additions_needed[s_index].end()) {
					// if needed, then starting must be on path
					int starting_index;
					if (run_helper.experiment->new_state_furthest_layer_needed_in[s_index] == 0) {
						starting_index = 0;
					} else {
						starting_index = run_helper.experiment_context_start_index
							+ run_helper.experiment->new_state_furthest_layer_needed_in[s_index]-1;
						// new_state_furthest_layer_needed_in[s_index] < scope_context.size()+2
					}
					for (int c_index = starting_index; c_index < (int)run_helper.experiment_helper_scope_context.size()-1; c_index++) {
						run_helper.experiment->scope_node_additions_needed[s_index].insert({
							run_helper.experiment_helper_scope_context[c_index],
							run_helper.experiment_helper_node_context[c_index]});
					}
				}
			}
		}

		experiment_variables_initialized = true;
	} else if (run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
		map<int, vector<ScoreNetwork*>>::iterator score_it = run_helper.experiment->score_networks.find(this->id);
		if (score_it != run_helper.experiment->score_networks.end()) {
			experiment_scope_score_networks = &(score_it->second);
		} else {
			experiment_scope_score_networks = NULL;
		}

		experiment_variables_initialized = true;
	}
}

void Scope::backprop(vector<int>& starting_node_ids,
					 vector<vector<double>*>& starting_state_errors,
					 vector<BackwardContextLayer>& context,
					 double& scale_factor_error,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	if (is_loop) {
		if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
				|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
			double halt_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->halt_score_network_output;
			double halt_predicted_score_error = run_helper.target_val - halt_predicted_score;
			this->halt_score_network->backprop_weights_with_no_error_signal(
				run_helper.scale_factor*halt_predicted_score_error,
				0.002,
				history->ending_state_vals_snapshot,
				history->halt_score_network_history);

			double halt_misguess_error = run_helper.final_misguess - history->halt_misguess_network_output;
			this->halt_misguess_network->backprop_weights_with_no_error_signal(
				halt_misguess_error,
				0.002,
				history->ending_state_vals_snapshot,
				history->halt_misguess_network_history);
		} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
				|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
			if (!run_helper.backprop_is_pre_experiment) {
				double halt_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->halt_score_network_output;
				double halt_predicted_score_error = run_helper.target_val - halt_predicted_score;
				this->halt_score_network->backprop_errors_with_no_weight_change(
					run_helper.scale_factor*halt_predicted_score_error,
					*(context.back().state_errors),
					history->ending_state_vals_snapshot,
					history->halt_score_network_history);

				double halt_misguess_error = run_helper.final_misguess - history->halt_misguess_network_output;
				this->halt_misguess_network->backprop_errors_with_no_weight_change(
					halt_misguess_error,
					*(context.back().state_errors),
					history->ending_state_vals_snapshot,
					history->halt_misguess_network_history);
			}
		}

		for (int i_index = (int)history->node_histories.size()-1; i_index >= 0; i_index--) {
			for (int h_index = (int)history->node_histories[i_index].size()-1; h_index >= 0; h_index--) {
				handle_node_backprop_helper(i_index,
											h_index,
											context,
											scale_factor_error,
											run_helper,
											history);
			}

			if (history->train_continue) {
				double best_halt_score = run_helper.target_val;
				double best_halt_misguess = run_helper.final_misguess;
				// back to front
				for (int ii_index = (int)history->node_histories.size()-1; ii_index >= i_index+1; ii_index--) {
					double score_diff = history->halt_score_snapshots[ii_index] - best_halt_score;
					double score_val = score_diff / (solution->average_misguess*abs(run_helper.scale_factor));
					if (score_val > 0.1) {
						best_halt_score = history->halt_score_snapshots[ii_index];
						best_halt_misguess = history->halt_misguess_snapshots[ii_index];
					} else if (score_val < -0.1) {
						continue;
					} else {
						double misguess_diff = history->halt_misguess_snapshots[ii_index] - best_halt_misguess;
						double misguess_val = misguess_diff / (solution->misguess_standard_deviation*abs(run_helper.scale_factor));
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
				if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
						|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
					double continue_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->continue_score_network_outputs[i_index];
					double continue_predicted_score_error = best_halt_score - continue_predicted_score;
					this->continue_score_network->backprop_weights_with_no_error_signal(
						run_helper.scale_factor*continue_predicted_score_error,
						0.002,
						history->iter_state_vals_snapshots[i_index],
						history->continue_score_network_histories[i_index]);

					double continue_misguess_error = best_halt_misguess - history->continue_misguess_network_outputs[i_index];
					this->continue_misguess_network->backprop_weights_with_no_error_signal(
						continue_misguess_error,
						0.002,
						history->iter_state_vals_snapshots[i_index],
						history->continue_misguess_network_histories[i_index]);
				} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
						|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN) {
					double continue_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->continue_score_network_outputs[i_index];
					double continue_predicted_score_error = best_halt_score - continue_predicted_score;
					this->continue_score_network->backprop_errors_with_no_weight_change(
						run_helper.scale_factor*continue_predicted_score_error,
						*(context.back().state_errors),
						history->iter_state_vals_snapshots[i_index],
						history->continue_score_network_histories[i_index]);

					double continue_misguess_error = best_halt_misguess - history->continue_misguess_network_outputs[i_index];
					this->continue_misguess_network->backprop_errors_with_no_weight_change(
						continue_misguess_error,
						*(context.back().state_errors),
						history->iter_state_vals_snapshots[i_index],
						history->continue_misguess_network_histories[i_index]);
				}
			}
		}
	} else {
		for (int h_index = (int)history->node_histories[0].size()-1; h_index >= 1; h_index--) {
			handle_node_backprop_helper(0,
										h_index,
										context,
										scale_factor_error,
										run_helper,
										history);
		}

		if ((run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
					|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN)
				&& !run_helper.backprop_is_pre_experiment
				&& starting_state_errors.size() > 0) {
			starting_node_ids.erase(starting_node_ids.begin());

			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[0][0];
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
			scope_node->halfway_backprop(starting_node_ids,
										 starting_state_errors,
										 context,
										 scale_factor_error,
										 run_helper,
										 scope_node_history);
		} else {
			handle_node_backprop_helper(0,
										0,
										context,
										scale_factor_error,
										run_helper,
										history);
		}
	}
}

void Scope::handle_node_backprop_helper(int iter_index,
										int h_index,
										vector<BackwardContextLayer>& context,
										double& scale_factor_error,
										RunHelper& run_helper,
										ScopeHistory* history) {
	if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_ACTION) {
		ActionNodeHistory* action_node_history = (ActionNodeHistory*)history->node_histories[iter_index][h_index];
		ActionNode* action_node = (ActionNode*)action_node_history->node;

		if (action_node_history->experiment_history != NULL) {
			if (action_node->experiment->type == EXPERIMENT_TYPE_BRANCH) {
				BranchExperiment* branch_experiment = (BranchExperiment*)action_node->experiment;
				branch_experiment->backprop(context,
											scale_factor_error,
											run_helper,
											(BranchExperimentHistory*)action_node_history->experiment_history);
			} else {
				LoopExperiment* loop_experiment = (LoopExperiment*)action_node->experiment;
				loop_experiment->backprop(context,
										  scale_factor_error,
										  run_helper,
										  (LoopExperimentHistory*)action_node_history->experiment_history);
			}

			if (action_node->experiment->state == EXPERIMENT_STATE_DONE) {
				action_node->is_explore = false;

				delete action_node->experiment;
				action_node->experiment = NULL;
			}
		}

		action_node->backprop(context,
							  scale_factor_error,
							  run_helper,
							  action_node_history);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_SCOPE) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[iter_index][h_index];
		ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
		scope_node->backprop(context,
							 scale_factor_error,
							 run_helper,
							 scope_node_history);
	} else if (history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_BRANCH) {
		BranchNodeHistory* branch_node_history = (BranchNodeHistory*)history->node_histories[iter_index][h_index];
		BranchNode* branch_node = (BranchNode*)branch_node_history->node;
		branch_node->backprop(context,
							  run_helper,
							  branch_node_history);
	} else {
		// history->node_histories[iter_index][h_index]->node->type == NODE_TYPE_EXIT
		ExitNodeHistory* exit_node_history = (ExitNodeHistory*)history->node_histories[iter_index][h_index];
		ExitNode* exit_node = (ExitNode*)exit_node_history->node;
		exit_node->backprop(context,
							run_helper,
							exit_node_history);
	}
}

void Scope::add_state(bool initialized_locally,
					  int family_id,
					  int default_class_id) {
	this->num_states++;
	this->state_initialized_locally.push_back(initialized_locally);
	this->state_family_ids.push_back(family_id);
	this->state_default_class_ids.push_back(default_class_id);

	if (this->is_loop) {
		this->continue_score_network->add_state();
		this->continue_misguess_network->add_state();
		this->halt_score_network->add_state();
		this->halt_misguess_network->add_state();
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			action_node->score_network->add_state();
			action_node->misguess_network->add_state();
		} else if (this->nodes[n_index]->type == NODE_TYPE_BRANCH) {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			if (branch_node->branch_score_network != NULL) {
				branch_node->branch_score_network->add_state();
			}
			if (branch_node->branch_misguess_network != NULL) {
				branch_node->branch_misguess_network->add_state();
			}
			if (branch_node->original_score_network != NULL) {
				branch_node->original_score_network->add_state();
			}
			if (branch_node->original_misguess_network != NULL) {
				branch_node->original_misguess_network->add_state();
			}
		}
	}
}

void Scope::save(ofstream& output_file) {
	output_file << this->num_states << endl;
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		output_file << this->state_initialized_locally[s_index] << endl;
		output_file << this->state_family_ids[s_index] << endl;
		output_file << this->state_default_class_ids[s_index] << endl;
	}

	output_file << this->is_loop << endl;

	if (this->is_loop) {
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

		output_file << this->furthest_successful_halt << endl;
	}

	output_file << this->nodes.size() << endl;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		output_file << this->nodes[n_index]->type << endl;

		ofstream node_save_file;
		node_save_file.open("saves/node_" + to_string(this->id) + "_" + to_string(n_index) + ".txt");
		this->nodes[n_index]->save(node_save_file);
		node_save_file.close();
	}
}

void Scope::save_for_display(ofstream& output_file) {

}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;

	this->exceeded_depth = false;

	this->halt_score_network_history = NULL;
	this->halt_misguess_network_history = NULL;
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

	for (int i_index = 0; i_index < (int)original->node_histories.size(); i_index++) {
		this->node_histories.push_back(vector<AbstractNodeHistory*>());
		for (int h_index = 0; h_index < (int)original->node_histories[i_index].size(); h_index++) {
			if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ActionNodeHistory(action_node_history));
			} else if (original->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)original->node_histories[i_index][h_index];
				this->node_histories.back().push_back(new ScopeNodeHistory(scope_node_history));
			}
		}
	}
}

ScopeHistory::~ScopeHistory() {
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

	for (int iter_index = 0; iter_index < (int)this->node_histories.size(); iter_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[iter_index].size(); h_index++) {
			delete this->node_histories[iter_index][h_index];
		}
	}
}
