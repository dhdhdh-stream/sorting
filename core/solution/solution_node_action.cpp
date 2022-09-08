#include "solution_node_action.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeAction::SolutionNodeAction(Action action) {
	this->score_network = NULL;
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

void SolutionNodeAction::re_eval_increment() {
	this->node_weight = 0.9999*this->node_weight;
}

SolutionNode* SolutionNodeAction::deep_copy(int inclusive_start_layer) {
	SolutionNodeAction* copy = new SolutionNodeAction(this->action);

	for (int l_index = inclusive_start_layer; l_index < (int)this->local_state_sizes.size(); l_index++) {
		copy->state_networks.push_back(this->state_networks[l_index]);
	}
}

void SolutionNodeAction::set_is_temp_node(bool is_temp_node) {
	this->is_temp_node = is_temp_node;
}

void SolutionNodeAction::initialize_local_state(vector<int>& explore_node_local_state_sizes) {
	this->local_state_sizes.insert(this->local_state_sizes.begin(),
		epxlore_node_local_state_sizes.begin(), explore_node_local_state_sizes.end());

	this->score_network = new ScoreNetwork(this->local_state_sizes);

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

	double score = activate_score_network(problem,
										  state_vals,
										  network_historys);
	// TODO: update misguess in re-eval backprop

	instance_history.push_back(ReEvalStepHistory(this,
												 score,
												 -1));

	return this->next;
}

SolutionNode* SolutionNodeAction::explore(Problem& problem,
										  vector<vector<double>>& state_vals,
										  vector<SolutionNode*>& scopes,
										  vector<int>& scope_states,
										  vector<int>& scope_locations,
										  IterExplore*& iter_explore,
										  vector<StepHistory>& instance_history,
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

		instance_history.push_back(StepHistory(this,
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
			bool is_after = is_after_explore(scope,
											 scope_states,
											 scope_locations,
											 iter_explore->scope,
											 iter_explore->scope_states,
											 iter_explore->scope_locations,
											 iter_explore->parent_jump_scope_start_non_inclusive_index);

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
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FLAT) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH) {
			bool is_after = is_after_explore(scope,
											 scope_states,
											 scope_locations,
											 iter_explore->scope,
											 iter_explore->scope_states,
											 iter_explore->scope_locations,
											 iter_explore->parent_jump_scope_start_non_inclusive_index);

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
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			bool is_after = is_after_explore(scope,
											 scope_states,
											 scope_locations,
											 iter_explore->scope,
											 iter_explore->scope_states,
											 iter_explore->scope_locations,
											 iter_explore->parent_jump_scope_start_non_inclusive_index);

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
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			activate_state_networks(problem,
									state_vals);
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			bool is_after = is_after_explore(scope,
											 scope_states,
											 scope_locations,
											 iter_explore->scope,
											 iter_explore->scope_states,
											 iter_explore->scope_locations,
											 iter_explore->parent_jump_scope_start_non_inclusive_index);

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
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			bool is_after = is_after_explore(scope,
											 scope_states,
											 scope_locations,
											 iter_explore->scope,
											 iter_explore->scope_states,
											 iter_explore->scope_locations,
											 iter_explore->parent_jump_scope_start_non_inclusive_index);

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
			activate_score_network(problem,
								   state_vals,
								   network_historys);
		}
	}

	if (iter_explore != NULL) {
		// push StepHistory early for new_state check
		instance_history.push_back(StepHistory(this,
											   true,
											   previous_observations,
											   -1,
											   -1,
											   false));
	}

	if (iter_explore != NULL
			&& iter_explore->explore_node == this) {
		return explore_activate_helper(problem,
									   scopes,
									   scope_states,
									   scope_locations,
									   iter_explore,
									   instance_history,
									   network_historys);
	}

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
			// backprop score_network
			// also update misguess
		}
	}

	if (this->is_temp_node) {
		if (iter_explore == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			// backprop state_networks
		} else if (iter_explore == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			// backprop state_networks
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
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FLAT) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
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
												 double* state_vals,
												 bool* states_on,
												 bool backprop,
												 vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[sn_index][i_index]]) {
					state_network_inputs.push_back(state_vals[this->state_network_inputs_state_indexes[sn_index][i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}

			if (backprop) {
				this->state_networks[sn_index]->mtx.lock();
				this->state_networks[sn_index]->activate(state_network_inputs, network_historys);
				state_vals[this->state_networks_target_states[sn_index]] = \
					this->state_networks[sn_index]->output->acti_vals[0];
				this->state_networks[sn_index]->mtx.unlock();
			} else {
				this->state_networks[sn_index]->mtx.lock();
				this->state_networks[sn_index]->activate(state_network_inputs);
				state_vals[this->state_networks_target_states[sn_index]] = \
					this->state_networks[sn_index]->output->acti_vals[0];
				this->state_networks[sn_index]->mtx.unlock();
			}
		}
	}
}

void SolutionNodeAction::backprop_state_networks(double* state_errors,
												 bool* states_on,
												 vector<NetworkHistory*>& network_historys) {
	for (int sn_index = (int)this->state_networks_target_states.size() - 1; sn_index >= 0; sn_index--) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[
				this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->backprop(state_network_errors);

			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[sn_index][i_index]]) {
					if (this->state_network_inputs_state_indexes[sn_index][i_index]
							== this->state_networks_target_states[sn_index]) {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] = \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					} else {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] += \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					}
				}
				this->state_networks[sn_index]->input->errors[1 + i_index] = 0.0;
			}

			this->state_networks[sn_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}

void SolutionNodeAction::backprop_state_networks_errors_with_no_weight_change(
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	for (int sn_index = (int)this->state_networks_target_states.size() - 1; sn_index >= 0; sn_index--) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[
				this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->backprop_errors_with_no_weight_change(state_network_errors);

			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[sn_index][i_index]]) {
					if (this->state_network_inputs_state_indexes[sn_index][i_index]
							== this->state_networks_target_states[sn_index]) {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] = \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					} else {
						state_errors[this->state_network_inputs_state_indexes[sn_index][i_index]] += \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					}
				}
				this->state_networks[sn_index]->input->errors[1 + i_index] = 0.0;
			}

			this->state_networks[sn_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}
