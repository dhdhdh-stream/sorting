#include "solution_node_empty.h"

#include <iostream>
#include <sstream>

#include "definitions.h"
#include "solution_node_utilities.h"

using namespace std;

SolutionNodeEmpty::SolutionNodeEmpty(vector<int> local_state_sizes) {
	this->node_type = NODE_TYPE_EMPTY;

	this->local_state_sizes = local_state_sizes;

	// state_networks will be copied in deep_copy() if needed

	// score_network initialized in initialize_local_state() if needed
	this->score_network = NULL;

	this->average_misguess = 2*solution->average_score*(1.0 - solution->average_score);

	this->is_temp_node = false;
}

SolutionNodeEmpty::SolutionNodeEmpty(vector<int>& scope_states,
									 vector<int>& scope_locations,
									 ifstream& save_file) {
	this->node_type = NODE_TYPE_EMPTY;

	ostringstream node_name_oss;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		node_name_oss << scope_states[l_index] << "_" << scope_locations[l_index] << "_";
	}
	string node_name = node_name_oss.str();

	string num_local_state_sizes_line;
	getline(save_file, num_local_state_sizes_line);
	int num_local_state_sizes = stoi(num_local_state_sizes_line);
	for (int l_index = 0; l_index < num_local_state_sizes; l_index++) {
		string state_size_line;
		getline(save_file, state_size_line);
		this->local_state_sizes.push_back(stoi(state_size_line));
	}

	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		if (this->local_state_sizes[l_index] > 0) {
			string state_network_name = "../saves/nns/" + node_name + "state_" \
				+ to_string(l_index) + "_" + to_string(solution->id) + ".txt";
			ifstream state_network_save_file;
			state_network_save_file.open(state_network_name);
			Network* state_network = new Network(state_network_save_file);
			state_network_save_file.close();
			this->state_networks.push_back(state_network);
		} else {
			this->state_networks.push_back(NULL);
		}
	}

	string score_network_name = "../saves/nns/" + node_name + "score_" + to_string(solution->id) + ".txt";
	ifstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network = new ScoreNetwork(score_network_save_file);
	score_network_save_file.close();

	string average_misguess_line;
	getline(save_file, average_misguess_line);
	this->average_misguess = stof(average_misguess_line);

	scope_locations.back()++;

	this->is_temp_node = false;
}

SolutionNodeEmpty::~SolutionNodeEmpty() {
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		if (this->state_networks[s_index] != NULL) {
			delete this->state_networks[s_index];
		}
	}

	if (this->score_network != NULL) {
		delete this->score_network;
	}
}

SolutionNode* SolutionNodeEmpty::re_eval(Problem& problem,
										 vector<vector<double>>& state_vals,
										 vector<SolutionNode*>& scopes,
										 vector<int>& scope_states,
										 vector<ReEvalStepHistory>& instance_history,
										 vector<AbstractNetworkHistory*>& network_historys) {
	activate_state_networks(problem,
							state_vals,
							network_historys);

	// only train if is_temp_node during explore

	instance_history.push_back(ReEvalStepHistory(this,
												 0.0,
												 -1));

	return this->next;
}

void SolutionNodeEmpty::re_eval_backprop(double score,
										 vector<vector<double>>& state_errors,
										 vector<ReEvalStepHistory>& instance_history,
										 vector<AbstractNetworkHistory*>& network_historys) {
	backprop_state_networks(state_errors,
							network_historys);

	instance_history.pop_back();
}

SolutionNode* SolutionNodeEmpty::explore(Problem& problem,
										 vector<vector<double>>& state_vals,
										 vector<SolutionNode*>& scopes,
										 vector<int>& scope_states,
										 vector<int>& scope_locations,
										 IterExplore*& iter_explore,
										 vector<ExploreStepHistory>& instance_history,
										 vector<AbstractNetworkHistory*>& network_historys,
										 bool& abandon_instance) {
	if (this->is_temp_node) {
		if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			activate_state_networks(problem,
									state_vals,
									network_historys);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
			activate_state_networks(problem,
									state_vals,
									network_historys);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			activate_state_networks(problem,
									state_vals);
		}
	} else {
		if (iter_explore == NULL) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_EXPLORE) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT) {
			bool is_after = false;
			if (iter_explore->explore_node->node_type != NODE_TYPE_START_SCOPE) {
				is_after = is_after_explore(scopes,
											scope_states,
											scope_locations,
											iter_explore->scopes,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_end_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < (int)iter_explore->scopes.size()) {
						if (iter_explore->scopes[l_index] == scopes[l_index]) {
							should_backprop = true;
						}
					}
				}

				if (should_backprop) {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index],
										   network_historys);
				} else {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FLAT) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH) {
			bool is_after = false;
			if (iter_explore->explore_node->node_type != NODE_TYPE_START_SCOPE) {
				is_after = is_after_explore(scopes,
											scope_states,
											scope_locations,
											iter_explore->scopes,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_end_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < (int)iter_explore->scopes.size()) {
						if (iter_explore->scopes[l_index] == scopes[l_index]) {
							should_backprop = true;
						}
					}
				}

				if (should_backprop) {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index],
										   network_historys);
				} else {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			bool is_after = false;
			if (iter_explore->explore_node->node_type != NODE_TYPE_START_SCOPE) {
				is_after = is_after_explore(scopes,
											scope_states,
											scope_locations,
											iter_explore->scopes,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_end_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < (int)iter_explore->scopes.size()) {
						if (iter_explore->scopes[l_index] == scopes[l_index]) {
							should_backprop = true;
						}
					}
				}

				if (should_backprop) {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index],
										   network_historys);
				} else {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			bool is_after = false;
			if (iter_explore->explore_node->node_type != NODE_TYPE_START_SCOPE) {
				is_after = is_after_explore(scopes,
											scope_states,
											scope_locations,
											iter_explore->scopes,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_end_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < (int)iter_explore->scopes.size()) {
						if (iter_explore->scopes[l_index] == scopes[l_index]) {
							should_backprop = true;
						}
					}
				}

				if (should_backprop) {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index],
										   network_historys);
				} else {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			bool is_after = false;
			if (iter_explore->explore_node->node_type != NODE_TYPE_START_SCOPE) {
				is_after = is_after_explore(scopes,
											scope_states,
											scope_locations,
											iter_explore->scopes,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_end_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < (int)iter_explore->scopes.size()) {
						if (iter_explore->scopes[l_index] == scopes[l_index]) {
							should_backprop = true;
						}
					}
				}

				if (should_backprop) {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index],
										   network_historys);
				} else {
					activate_state_network(problem,
										   l_index,
										   state_vals[l_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			activate_state_networks(problem,
									state_vals);
		}
	}

	if (this->is_temp_node) {
		if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT) {
			vector<double> obs;
			obs.push_back(problem.get_observation());

			this->score_network->mtx.lock();
			this->score_network->activate(state_vals,
										  obs,
										  network_historys);
			this->score_network->mtx.unlock();
		}
	}

	instance_history.push_back(ExploreStepHistory(this,
												  false,
												  0.0,
												  -1,
												  -1,
												  false));

	scope_locations.back()++;
	return this->next;
}

void SolutionNodeEmpty::explore_backprop(double score,
										 vector<vector<double>>& state_errors,
										 IterExplore*& iter_explore,
										 vector<ExploreStepHistory>& instance_history,
										 vector<AbstractNetworkHistory*>& network_historys) {
	if (this->is_temp_node) {
		if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT) {
			AbstractNetworkHistory* network_history = network_historys.back();

			this->score_network->mtx.lock();

			network_history->reset_weights();

			double misguess;
			vector<double> errors;
			if (score == 1.0) {
				if (this->score_network->output->acti_vals[0] < 1.0) {
					errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
					misguess = abs(1.0 - this->score_network->output->acti_vals[0]);
				} else {
					errors.push_back(0.0);
					misguess = 0.0;
				}
			} else {
				if (this->score_network->output->acti_vals[0] > 0.0) {
					errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
					misguess = abs(0.0 - this->score_network->output->acti_vals[0]);
				} else {
					errors.push_back(0.0);
					misguess = 0.0;
				}
			}
			this->score_network->backprop(errors);

			this->score_network->mtx.unlock();

			delete network_history;
			network_historys.pop_back();

			this->average_misguess = 0.9999*this->average_misguess + 0.0001*misguess;
		}
	}

	if (this->is_temp_node) {
		if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			backprop_state_networks(state_errors,
									network_historys);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			backprop_state_networks(state_errors,
									network_historys);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			// do nothing
		}
	} else {
		if (iter_explore == NULL) {
			// solution should not backprop if this is the case
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_EXPLORE) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.size() > 0 && network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		state_errors[l_index],
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FLAT) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.size() > 0 && network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		state_errors[l_index],
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.size() > 0 && network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		state_errors[l_index],
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.size() > 0 && network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		state_errors[l_index],
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.size() > 0 && network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		state_errors[l_index],
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			// do nothing
		}
	}

	instance_history.pop_back();
	return;
}

void SolutionNodeEmpty::explore_increment(double score,
										  IterExplore*& iter_explore) {
	// should not happen
}

void SolutionNodeEmpty::re_eval_increment() {
	// does not matter
}

SolutionNode* SolutionNodeEmpty::deep_copy(int inclusive_start_layer) {
	vector<int> copy_local_state_size(this->local_state_sizes.begin()+inclusive_start_layer,
		this->local_state_sizes.end());
	SolutionNodeEmpty* copy = new SolutionNodeEmpty(copy_local_state_size);

	for (int l_index = inclusive_start_layer; l_index < (int)this->local_state_sizes.size(); l_index++) {
		if (this->local_state_sizes[l_index] > 0) {
			copy->state_networks.push_back(new Network(this->state_networks[l_index]));
		} else {
			copy->state_networks.push_back(NULL);
		}
	}

	return copy;
}

void SolutionNodeEmpty::set_is_temp_node(bool is_temp_node) {
	this->is_temp_node = is_temp_node;

	// delete score_network after exploration
	if (is_temp_node == false) {
		if (this->score_network != NULL) {
			delete this->score_network;
			this->score_network = NULL;
		}
	}
}

void SolutionNodeEmpty::initialize_local_state(vector<int>& explore_node_local_state_sizes) {
	this->local_state_sizes.insert(this->local_state_sizes.begin(),
		explore_node_local_state_sizes.begin(), explore_node_local_state_sizes.end());

	for (int l_index = 0; l_index < (int)explore_node_local_state_sizes.size(); l_index++) {
		if (explore_node_local_state_sizes[l_index] != 0) {
			this->state_networks.insert(this->state_networks.begin()+l_index,
				new Network(explore_node_local_state_sizes[l_index],
							4*explore_node_local_state_sizes[l_index],
							1));
		} else {
			this->state_networks.insert(this->state_networks.begin()+l_index, NULL);
		}
	}

	this->score_network = new ScoreNetwork(this->local_state_sizes);
}

void SolutionNodeEmpty::setup_flat(vector<int>& loop_scope_counts,
								   int& curr_index,
								   SolutionNode* explore_node) {
	FoldHelper* fold_helper;

	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	if (it == this->fold_helpers.end()) {
		fold_helper = new FoldHelper(this,
									 (int)loop_scope_counts.size());
		this->fold_helpers[explore_node] = fold_helper;
	} else {
		// loop
		fold_helper = it->second;
	}

	fold_helper->set_index(loop_scope_counts,
						   curr_index);

	// don't increment curr_index
}

void SolutionNodeEmpty::setup_new_state(SolutionNode* explore_node,
										int new_state_size) {
	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	it->second->initialize_new_state_network(new_state_size);
}

void SolutionNodeEmpty::get_min_misguess(double& min_misguess) {
	if (this->average_misguess < min_misguess) {
		min_misguess = this->average_misguess;
	}
}

void SolutionNodeEmpty::cleanup_explore(SolutionNode* explore_node) {
	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	delete it->second;
	this->fold_helpers.erase(it);
}

void SolutionNodeEmpty::collect_new_state_networks(SolutionNode* explore_node,
												   vector<SolutionNode*>& existing_nodes,
												   vector<Network*>& new_state_networks) {
	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	existing_nodes.push_back(this);
	new_state_networks.push_back(it->second->new_state_network);
	it->second->new_state_network = NULL;
	delete it->second;
	this->fold_helpers.erase(it);
}

void SolutionNodeEmpty::insert_scope(int layer,
									 int new_state_size) {
	this->local_state_sizes.insert(this->local_state_sizes.begin() + layer, new_state_size);

	// state_networks will be set by candidate if needed
	this->state_networks.insert(this->state_networks.begin() + layer, NULL);
}

void SolutionNodeEmpty::reset_explore() {
	// do nothing
}

void SolutionNodeEmpty::save(vector<int>& scope_states,
							 vector<int>& scope_locations,
							 ofstream& save_file) {
	ostringstream node_name_oss;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		node_name_oss << scope_states[l_index] << "_" << scope_locations[l_index] << "_";
	}
	string node_name = node_name_oss.str();

	save_file << this->local_state_sizes.size() << endl;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		save_file << this->local_state_sizes[l_index] << endl;
	}

	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		if (this->local_state_sizes[l_index] > 0) {
			string state_network_name = "../saves/nns/" + node_name + "state_" \
				+ to_string(l_index) + "_" + to_string(solution->id) + ".txt";
			ofstream state_network_save_file;
			state_network_save_file.open(state_network_name);
			this->state_networks[l_index]->save(state_network_save_file);
			state_network_save_file.close();
		}
	}

	string score_network_name = "../saves/nns/" + node_name + "score_" + to_string(solution->id) + ".txt";
	ofstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	save_file << this->average_misguess << endl;

	scope_locations.back()++;
}

void SolutionNodeEmpty::save_for_display(ofstream& save_file) {

}

void SolutionNodeEmpty::activate_state_networks(Problem& problem,
												vector<vector<double>>& state_vals) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		if (this->local_state_sizes[l_index] > 0) {
			vector<double> inputs;
			inputs.reserve(1+this->local_state_sizes[l_index]);
			inputs.push_back(problem.get_observation());
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				inputs.push_back(state_vals[l_index][s_index]);
			}

			this->state_networks[l_index]->mtx.lock();
			this->state_networks[l_index]->activate(inputs);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->state_networks[l_index]->output->acti_vals[s_index];
			}
			this->state_networks[l_index]->mtx.unlock();
		}
	}
}

void SolutionNodeEmpty::activate_state_networks(Problem& problem,
												vector<vector<double>>& state_vals,
												vector<AbstractNetworkHistory*>& network_historys) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		if (this->local_state_sizes[l_index] > 0) {
			vector<double> inputs;
			inputs.reserve(1+this->local_state_sizes[l_index]);
			inputs.push_back(problem.get_observation());
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				inputs.push_back(state_vals[l_index][s_index]);
			}

			this->state_networks[l_index]->mtx.lock();
			this->state_networks[l_index]->activate(inputs, network_historys);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->state_networks[l_index]->output->acti_vals[s_index];
			}
			this->state_networks[l_index]->mtx.unlock();
		}
	}
}

void SolutionNodeEmpty::activate_state_network(Problem& problem,
											   int layer,
											   vector<double>& layer_state_vals) {
	if (this->local_state_sizes[layer] > 0) {
		vector<double> inputs;
		inputs.reserve(1+this->local_state_sizes[layer]);
		inputs.push_back(problem.get_observation());
		for (int s_index = 0; s_index < this->local_state_sizes[layer]; s_index++) {
			inputs.push_back(layer_state_vals[s_index]);
		}

		this->state_networks[layer]->mtx.lock();
		this->state_networks[layer]->activate(inputs);
		for (int s_index = 0; s_index < this->local_state_sizes[layer]; s_index++) {
			layer_state_vals[s_index] = this->state_networks[layer]->output->acti_vals[s_index];
		}
		this->state_networks[layer]->mtx.unlock();
	}
}

void SolutionNodeEmpty::activate_state_network(Problem& problem,
											   int layer,
											   vector<double>& layer_state_vals,
											   vector<AbstractNetworkHistory*>& network_historys) {
	if (this->local_state_sizes[layer] > 0) {
		vector<double> inputs;
		inputs.reserve(1+this->local_state_sizes[layer]);
		inputs.push_back(problem.get_observation());
		for (int s_index = 0; s_index < this->local_state_sizes[layer]; s_index++) {
			inputs.push_back(layer_state_vals[s_index]);
		}

		this->state_networks[layer]->mtx.lock();
		this->state_networks[layer]->activate(inputs, network_historys);
		for (int s_index = 0; s_index < this->local_state_sizes[layer]; s_index++) {
			layer_state_vals[s_index] = this->state_networks[layer]->output->acti_vals[s_index];
		}
		this->state_networks[layer]->mtx.unlock();
	}
}

void SolutionNodeEmpty::backprop_state_networks(vector<vector<double>>& state_errors,
												vector<AbstractNetworkHistory*>& network_historys) {
	for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
		if (this->local_state_sizes[l_index] > 0) {
			AbstractNetworkHistory* network_history = network_historys.back();

			this->state_networks[l_index]->mtx.lock();

			network_history->reset_weights();

			this->state_networks[l_index]->backprop(state_errors[l_index]);

			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_errors[l_index][s_index] = this->state_networks[l_index]->input->errors[1+s_index];
				this->state_networks[l_index]->input->errors[1+s_index] = 0.0;
			}

			this->state_networks[l_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}

void SolutionNodeEmpty::backprop_state_network_errors_with_no_weight_change(
		int layer,
		vector<double>& layer_state_errors,
		vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->state_networks[layer]->mtx.lock();

	network_history->reset_weights();

	this->state_networks[layer]->backprop_errors_with_no_weight_change(layer_state_errors);

	for (int s_index = 0; s_index < this->local_state_sizes[layer]; s_index++) {
		layer_state_errors[s_index] = this->state_networks[layer]->input->errors[1+s_index];
		this->state_networks[layer]->input->errors[1+s_index] = 0.0;
	}

	this->state_networks[layer]->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNodeEmpty::new_path_activate_state_networks(double observations,
														 vector<vector<double>>& state_vals,
														 vector<AbstractNetworkHistory*>& network_historys) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		if (this->local_state_sizes[l_index] > 0) {
			vector<double> inputs;
			inputs.reserve(1+this->local_state_sizes[l_index]);
			inputs.push_back(observations);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				inputs.push_back(state_vals[l_index][s_index]);
			}

			this->state_networks[l_index]->mtx.lock();
			this->state_networks[l_index]->activate(inputs, network_historys);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->state_networks[l_index]->output->acti_vals[s_index];
			}
			this->state_networks[l_index]->mtx.unlock();
		}
	}
}
