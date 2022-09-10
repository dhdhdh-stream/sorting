#include "solution_node_action.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeAction::SolutionNodeAction(Action action
									   std::vector<int> local_state_sizes) {
	this->score_network = NULL;
	this->local_state_sizes = local_state_sizes;
}

SolutionNodeAction::SolutionNodeAction(ifstream& save_file) {
	this->score_network = NULL;
}

SolutionNodeAction::~SolutionNodeAction() {
	if (this->score_network != NULL) {
		delete this->score_network;
	}

	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		if (this->state_networks[s_index] != NULL) {
			delete this->state_networks[s_index];
		}
	}
}

SolutionNode* SolutionNodeAction::re_eval(Problem& problem,
										  vector<vector<double>>& state_vals,
										  vector<SolutionNode*>& scopes,
										  vector<int>& scope_states,
										  vector<ReEvalStepHistory>& instance_history,
										  vector<AbstractNetworkHistory*>& network_historys) {
	activate_state_networks(problem,
							state_vals,
							network_historys);

	problem.perform_action(this->action);

	vector<double> obs;
	obs.push_back(problem.get_observation());

	this->score_network->mtx.lock();
	this->score_network->activate(state_vals,
								  obs,
								  network_historys);
	double score = this->score_network->output->acti_vals[0];
	this->score_network->mtx.unlock();

	instance_history.push_back(ReEvalStepHistory(this,
												 score,
												 -1));

	return this->next;
}

void SolutionNodeAction::re_eval_backprop(double score,
										  vector<vector<double>>& state_errors,
										  vector<ReEvalStepHistory>& instance_history,
										  vector<AbstractNetworkHistory*>& network_historys) {
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

	this->average_misguess = 0.9999*this->average_misguess + 0.0001*misguess;

	backprop_state_networks(state_errors,
							network_historys);

	instance_history.pop_back();
}

SolutionNode* SolutionNodeAction::explore(Problem& problem,
										  vector<vector<double>>& state_vals,
										  vector<SolutionNode*>& scopes,
										  vector<int>& scope_states,
										  vector<int>& scope_locations,
										  IterExplore*& iter_explore,
										  vector<ExploreStepHistory>& instance_history,
										  vector<AbstractNetworkHistory*>& network_historys,
										  bool& abandon_instance) {
	if (iter_explore->explore_node == this
			&& scopes.back() == NULL) {
		explore_callback_helper(problem,
								state_vals,
								scopes,
								scope_states,
								scope_locations,
								network_historys);

		instance_history.push_back(ExploreStepHistory(this,
													  false,
													  0.0,
													  -1,
													  -1,
													  true));
		return get_jump_end(this);
	}

	bool is_first_explore = false;
	if (iter_explore == NULL) {
		is_explore_helper(scopes,
						  scope_states,
						  scope_locations,
						  iter_explore,
						  is_first_explore);
	}

	if (this->is_temp_node) {
		if (iter_explore == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			activate_state_networks(problem,
									state_vals,
									network_historys);
		} else if (iter_explore == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
			activate_state_networks(problem,
									state_vals,
									network_historys);
		} else if (iter_explore == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
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
				is_after = is_after_explore(scope,
											scope_states,
											scope_locations,
											iter_explore->scope,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_scope_start_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < iter_explore->scopes.size()) {
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
				is_after = is_after_explore(scope,
											scope_states,
											scope_locations,
											iter_explore->scope,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_scope_start_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < iter_explore->scopes.size()) {
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
				is_after = is_after_explore(scope,
											scope_states,
											scope_locations,
											iter_explore->scope,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_scope_start_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < iter_explore->scopes.size()) {
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
				is_after = is_after_explore(scope,
											scope_states,
											scope_locations,
											iter_explore->scope,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_scope_start_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (l_index < iter_explore->scopes.size()) {
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
										   s_index,
										   state_vals[s_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			bool is_after = false;
			if (iter_explore->explore_node->node_type != NODE_TYPE_START_SCOPE) {
				is_after = is_after_explore(scope,
											scope_states,
											scope_locations,
											iter_explore->scope,
											iter_explore->scope_states,
											iter_explore->scope_locations,
											iter_explore->parent_jump_scope_start_non_inclusive_index);
			}

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				bool should_backprop = false;
				if (is_after) {
					if (s_index < iter_explore->scopes.size()) {
						if (iter_explore->scopes[s_index] == scopes[s_index]) {
							should_backprop = true;
						}
					}
				}

				if (should_backprop) {
					activate_state_network(problem,
										   s_index,
										   state_vals[s_index],
										   network_historys);
				} else {
					activate_state_network(problem,
										   s_index,
										   state_vals[s_index]);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			activate_state_networks(problem,
									state_vals);
		}
	}

	double previous_observations = problem.get_observation();
	problem.perform_action(this->action);

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

	// push StepHistory early for new_state check
	instance_history.push_back(ExploreStepHistory(this,
												  true,
												  previous_observations,
												  -1,
												  -1,
												  false));

	if (iter_explore != NULL
			&& iter_explore->explore_node == this) {
		return explore_helper(problem,
							  scopes,
							  scope_states,
							  scope_locations,
							  iter_explore,
							  instance_history,
							  network_historys);
	}

	scope_locations.back()++;
	return this->next;
}

void SolutionNodeAction::explore_backprop(double score,
										  vector<vector<double>>& state_errors,
										  IterExplore*& iter_explore,
										  vector<StepHistory>& instance_history,
										  vector<NetworkHistory*>& network_historys) {
	if (instance_history->is_explore_callback) {
		// iter_explore->explore_node == this
		explore_callback_backprop_helper(state_errors,
										 instance_history,
										 network_historys);

		instance_history.pop_back();
		return;
	}

	if (iter_explore != NULL
			&& iter_explore->explore_node == this) {
		explore_backprop_helper(score,
								instance_history,
								network_historys);
	}

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

			this->average_misguess = 0.9999*this->average_misguess + 0.0001*misguess;
		}
	}

	if (this->is_temp_node) {
		if (iter_explore == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			backprop_state_networks(state_errors,
									network_historys);
		} else if (iter_explore == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			backprop_state_networks(state_errors,
									network_historys);
		} else if (iter_explore == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			// do nothing
		}
	} else {
		if (iter_explore == NULL) {
			// solution should not backprop if this is the case
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_EXPLORE) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		layer_state_errors,
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FLAT) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		layer_state_errors,
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		layer_state_errors,
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		layer_state_errors,
																		network_historys);
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					backprop_state_network_errors_with_no_weight_change(l_index,
																		layer_state_errors,
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

void SolutionNodeAction::explore_increment(double score,
										   IterExplore*& iter_explore) {
	explore_increment_helper(score,
							 iter_explore);
}

void SolutionNodeAction::re_eval_increment() {
	this->node_weight = 0.9999*this->node_weight;
}

SolutionNode* SolutionNodeAction::deep_copy(int inclusive_start_layer) {
	vector<int> copy_local_state_size(this->local_state_sizes.begin()+inclusive_start_layer,
		this->local_state_sizes.end());
	SolutionNodeAction* copy = new SolutionNodeAction(this->action,
													  copy_local_state_size);

	for (int l_index = inclusive_start_layer; l_index < (int)this->local_state_sizes.size(); l_index++) {
		copy->state_networks.push_back(new Network(this->state_networks[l_index]));
	}
}

void SolutionNodeAction::set_is_temp_node(bool is_temp_node) {
	this->is_temp_node = is_temp_node;
}

void SolutionNodeAction::initialize_local_state(vector<int>& explore_node_local_state_sizes) {
	this->local_state_sizes.insert(this->local_state_sizes.begin(),
		explore_node_local_state_sizes.begin(), explore_node_local_state_sizes.end());

	for (int l_index = 0; l_index < explore_node_local_state_sizes.size(); l_index++) {
		if (explore_node_local_state_sizes[l_index] != 0) {
			this->state_networks.insert(this->state_networks.begin(),
				new Network(explore_node_local_state_sizes[l_index],
							4*explore_node_local_state_sizes[l_index],
							1));
		} else {
			this->state_networks.insert(this->state_networks.begin(), NULL);
		}
	}

	this->score_network = new ScoreNetwork(this->local_state_sizes);
}

void SolutionNodeAction::setup_flat(vector<int>& loop_scope_counts,
									int& curr_index,
									SolutionNode* explore_node) {
	FoldHelper* fold_helper;

	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	if (it == this->fold_helpers.end()) {
		fold_helper = new FoldHelper(this,
									 loop_scope_counts.size());
		this->fold_helpers[explore_node] = fold_helper;
	} else {
		// loop
		fold_helper = it->second;
	}

	fold_helper->set_index(loop_scope_counts,
						   curr_index);

	curr_index++;
}

void SolutionNodeAction::setup_new_state(SolutionNode* explore_node,
										 int new_state_size) {
	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	it->second->initialize_new_state_network(new_state_size);
}

void SolutionNodeAction::get_min_misguess(double& min_misguess) {
	if (this->average_misguess < min_misguess) {
		min_misguess = this->average_misguess;
	}
}

void SolutionNodeAction::cleanup_explore(SolutionNode* explore_node) {
	this->fold_helpers.erase(explore_node);
}

void SolutionNodeAction::collect_new_state_networks(SolutionNode* explore_node,
													vector<SolutionNode*>& existing_nodes,
													vector<Network*>& new_state_networks) {
	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	existing_nodes.push_back(this);
	new_state_networks.push_back(it->new_state_network);
	it->new_state_network = NULL;
	this->fold_helpers.erase(it);
}

void SolutionNodeAction::insert_scope(int layer,
									  int new_state_size) {
	this->local_state_sizes.insert(this->local_state_sizes.begin() + layer, new_state_size);

	// state_networks will be set by candidate if needed
	this->state_networks.insert(this->state_networks.begin() + layer, NULL);

	this->score_network->insert_scope(layer, new_state_size);
}

void SolutionNodeAction::reset_explore() {
	for (map<SolutionNode*,FoldHelper*>::iterator it = this->fold_helpers.begin(); it != this->fold_helpers.end(); it++) {
		delete it->second;
	}
	this->fold_helpers.clear();

	this->explore_state = EXPLORE_STATE_EXPLORE;

	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}
	this->explore_path.clear();

	for (int s_index = 0; s_index < (int)this->explore_state_networks.size(); s_index++) {
		if (this->explore_state_networks[s_index] != NULL) {
			delete this->explore_state_networks[s_index];
		}
	}
	this->explore_state_networks.clear();

	if (this->explore_jump_score_network != NULL) {
		delete this->explore_jump_score_network;
	}
	if (this->explore_no_jump_score_network != NULL) {
		delete this->explore_no_jump_score_network;
	}

	if (this->explore_small_jump_score_network != NULL) {
		delete this->explore_small_jump_score_network;
	}
	if (this->explore_small_no_jump_score_network != NULL) {
		delete this->explore_small_no_jump_score_network;
	}
}

void SolutionNodeAction::save(ofstream& save_file) {
	save_score_network(save_file);

	this->action.save(save_file);

	save_file << this->state_networks.size() << endl;
	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		save_file << this->state_network_inputs_state_indexes[s_index].size() << endl;
		for (int si_index = 0; si_index < (int)this->state_network_inputs_state_indexes[s_index].size(); si_index++) {
			save_file << this->state_network_inputs_state_indexes[s_index][si_index] << endl;
		}
		save_file << this->state_networks_target_states[s_index] << endl;
	
		string state_network_name = "../saves/nns/state_" + to_string(this->node_index) \
			+ "_" + to_string(s_index) + "_" + to_string(this->solution->id) + ".txt";
		ofstream state_network_save_file;
		state_network_save_file.open(state_network_name);
		this->state_networks[s_index]->save(state_network_save_file);
		state_network_save_file.close();
	}
}

void SolutionNodeAction::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		this->action.save(save_file);
		save_file << this->next->node_index << endl;
	}
}

void SolutionNodeAction::activate_state_networks(Problem& problem,
												 vector<vector<double>>& state_vals) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes; l_index++) {
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

void SolutionNodeAction::activate_state_networks(Problem& problem,
												 vector<vector<double>>& state_vals,
												 vector<AbstractNetworkHistory*>& network_historys) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes; l_index++) {
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

void SolutionNodeAction::activate_state_network(Problem& problem,
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

void SolutionNodeAction::activate_state_network(Problem& problem,
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

void SolutionNodeAction::backprop_state_networks(vector<vector<double>>& state_errors,
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

void SolutionNodeAction::backprop_state_network_errors_with_no_weight_change(
		int layer,
		vector<double>& layer_state_errors,
		vector<AbstractNetworkHistory*>& network_historys) {
	if (this->local_state_sizes[layer] > 0) {
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
}

void SolutionNodeAction::new_path_activate_state_networks(double observations,
														  vector<vector<double>>& state_vals,
														  vector<AbstractNetworkHistory*>& network_historys) {
	for (int l_index = 0; l_index < (int)this->local_state_sizes; l_index++) {
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