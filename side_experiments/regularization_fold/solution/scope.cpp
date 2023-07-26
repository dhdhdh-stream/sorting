#include "scope.h"

using namespace std;

Scope::Scope(int id,
			 int num_states,
			 vector<bool> state_initialized_locally,
			 vector<int> state_family_ids,
			 vector<int> state_default_class_ids,
			 bool is_loop,
			 vector<int> starting_target_indexes,
			 vector<StateNetwork*> starting_state_networks,
			 ScoreNetwork* continue_score_network,
			 ScoreNetwork* continue_misguess_network,
			 ScoreNetwork* halt_score_network,
			 ScoreNetwork* halt_misguess_network,
			 vector<AbstractNode*> nodes) {
	this->id = id;
	this->num_states = num_states;
	this->state_initialized_locally = state_initialized_locally;
	this->state_family_ids = state_family_ids;
	this->state_default_class_ids = state_default_class_ids;
	this->is_loop = is_loop;
	this->starting_target_indexes = starting_target_indexes;
	this->starting_state_networks = starting_state_networks;
	this->continue_score_network = continue_score_network;
	this->continue_misguess_network = continue_misguess_network;
	this->halt_score_network = halt_score_network;
	this->halt_misguess_network = halt_misguess_network;
	this->furthest_successful_halt = 5;	// simply initializing to 5
	this->nodes = nodes;
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
		string starting_state_networks_size_line;
		getline(input_file, starting_state_networks_size_line);
		int starting_state_networks_size = stoi(starting_state_networks_size_line);
		for (int s_index = 0; s_index < starting_state_networks_size; s_index++) {
			string starting_target_index_line;
			getline(input_file, starting_target_index_line);
			this->starting_target_indexes.push_back(stoi(starting_target_index_line))

			ifstream starting_state_network_save_file;
			starting_state_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_starting_state_" + to_string(s_index) + ".txt");
			this->starting_state_networks.push_back(new StateNetwork(starting_state_network_save_file));
			starting_state_network_save_file.close();
		}

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
	for (int s_index = 0; s_index < (int)this->starting_state_networks.size(); s_index++) {
		delete this->starting_state_networks[s_index];
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

		bool is_explore_iter_save = run_helper.is_explore_iter;
		int explore_iter;
		if (run_helper.is_explore_iter) {

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
			FamilyDefinition* family_definition = solution->families[this->state_default_class_ids[s_index]];
			run_helper.last_seen_vals[family_definition] = context.back().state_vals->at(s_index);
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

		if (action_node->is_explore
				&& run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			if (action_node->curr_experiment != NULL) {
				bool matches_context = true;
				if (action_node->branch_experiment->scope_context.size() > context.size()) {
					matches_context = false;
				} else {
					for (int c_index = 0; c_index < (int)action_node->branch_experiment->scope_context.size()-1; c_index++) {
						if (action_node->branch_experiment->scope_context[c_index] != 
									context[context.size()-action_node->branch_experiment->scope_context.size()+c_index].scope_id
								|| action_node->branch_experiment->node_context[c_index] !=
									context[context.size()-action_node->branch_experiment->scope_context.size()+c_index].node_id) {
							matches_context = false;
							break;
						}
					}
				}
				if (matches_context) {
					if (action_node->curr_experiment->type == EXPERIMENT_TYPE_BRANCH) {
						BranchExperiment* branch_experiment = (BranchExperiment*)action_node->curr_experiment;

						// explore_phase set in experiment
						BranchExperimentHistory* branch_experiment_history = new BranchExperimentHistory(branch_experiment);
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
						// action_node->curr_experiment->type == EXPERIMENT_TYPE_LOOP

					}
				} else {
					curr_node_id = action_node->next_node_id;
				}
			} else {
				// TODO: explore

			}
		} else {
			curr_node_id = action_node->next_node_id;
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
							  history);

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
		}

		int size_diff = (int)scope_history->scope->nodes.size() - (int)state_it->second.size();
		state_it->second.insert(state_it->second.end(), size_diff, vector<StateNetwork*>());
		score_it->second.insert(score_it->second.end(), size_diff, NULL);

		map<int, int>::iterator layer_seen_in_it = run_helper.experiment->scope_furthest_layer_seen_in.find(this->id);
		if (layer_seen_in_it == run_helper.experiment->scope_furthest_layer_seen_in.end()) {
			layer_seen_in_it = run_helper.experiment->scope_furthest_layer_seen_in.insert({this->id, run_helper.experiment_context_index}).first;
		} else {
			if (run_helper.experiment_context_index < layer_seen_in_it->second) {
				layer_seen_in_it->second = run_helper.experiment_context_index;

				int new_furthest_distance = run_helper.experiment->scope_context.size()+2 - run_helper.experiment_context_index;
				for (int n_index = 0; n_index < (int)state_it->second.size(); n_index++) {
					if (state_it->second[n_index].size() != 0) {
						for (int s_index = 0; s_index < NUM_NEW_STATES; s_index++) {
							state_it->second[n_index][s_index]->update_lasso_weights(new_furthest_distance);
						}
					}
				}
			}
		}

		if (run_helper.experiment_step_index != -1) {
			BranchExperiment* branch_experiment = (BranchExperiment*)run_helper.experiment;
			map<int, vector<bool>>::iterator steps_seen_in_it = branch_experiment->scope_steps_seen_in.find(this->id);
			if (steps_seen_in_it == run_helper.experiment->scope_steps_seen_in.end()) {
				branch_experiment->scope_steps_seen_in[this->id] = vector<bool>(branch_experiment->num_steps, false);
			}
			branch_experiment->scope_steps_seen_in[this->id] = run_helper.experiment_step_index;
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

		if ((run_helper.experiment->type == EXPERIMENT_TYPE_BRANCH
					&& run_helper.state == BRANCH_EXPERIMENT_STATE_SECOND_CLEAN)
				|| ) {
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

	} else {
		starting_node_ids.erase(starting_node_ids.begin());

		for (int h_index = (int)history->node_histories[0].size()-1; h_index >= 1; h_index--) {
			handle_node_backprop_helper(0,
										h_index,
										context,
										scale_factor_error,
										run_helper,
										history);
		}

		if (starting_state_errors.size() > 0) {
			ScopeNode* scope_node = (ScopeNode*)history->node_histories[0][0]->node;
			scope_node->halfway_backprop(starting_node_ids,
										 starting_state_errors,
										 context,
										 scale_factor_error,
										 run_helper,
										 history);
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
		action_node->backprop(context,
							  scale_factor_error,
							  run_helper,
							  action_node_history);

		if (action_node_history->experiment_history != NULL) {
			if ()
			action_node->curr_experiment->backprop();

			if (action_node->curr_experiment->type == EXPERIMENT_TYPE_BRANCH
					&& action_node->curr_experiment->state == BRANCH_EXPERIMENT_STATE_DONE) {

			} else if () {

			}
		}
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
							branch_node_history);
	}
}

void Scope::add_state(bool initialized_locally,
					  int family_id,
					  int default_class_id) {
	this->num_states++;
	this->state_initialized_locally.push_back(initialized_locally);
	this->state_family_ids.push_back(family_id);
	this->state_default_class_ids.push_back(default_class_id);

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			action_node->score_network->add_state();
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
		output_file << this->starting_state_networks.size() << endl;
		for (int s_index = 0; s_index < (int)this->starting_state_networks.size(); s_index++) {
			output_file << this->starting_target_indexes[s_index] << endl;

			ofstream starting_state_network_save_file;
			starting_state_network_save_file.open("saves/nns/scope_" + to_string(this->id) + "_starting_state_" + to_string(s_index) + ".txt");
			this->starting_state_networks[s_index]->save(starting_state_network_save_file);
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
}

ScopeHistory::ScopeHistory(ScopeHistory* original) {
	this->scope = original->scope;

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
	for (int s_index = 0; s_index < (int)this->starting_state_network_histories.size(); s_index++) {
		if (this->starting_state_network_histories[s_index] != NULL) {
			delete this->starting_state_network_histories[s_index];
		}
	}

	for (int iter_index = 0; iter_index < (int)this->node_histories.size(); iter_index++) {
		for (int h_index = 0; h_index < (int)this->node_histories[iter_index].size(); h_index++) {
			delete this->node_histories[iter_index][h_index];
		}
	}
}
