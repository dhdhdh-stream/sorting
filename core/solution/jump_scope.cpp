#include "jump_scope.h"

using namespace std;

JumpScope::JumpScope() {

}

JumpScope::~JumpScope() {

}

void JumpScope::re_eval_increment() {
	this->node_weight = 0.9999*this->node_weight;

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->re_eval_increment();
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->re_eval_increment();
		}
	}
}

SolutionNode* JumpScope::deep_copy(int inclusive_start_layer) {
	vector<int> copy_local_state_size(this->local_state_sizes.begin()+inclusive_start_layer,
		this->local_state_sizes.end());
	JumpScope* copy = new JumpScope(copy_local_state_size,
									this->num_states);

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		copy->top_path.push_back(this->top_path[n_index]->deep_copy(inclusive_start_layer));
	}
	for (int n_index = 0; n_index < (int)copy->top_path.size(); n_index++) {
		copy->top_path[n_index]->parent_scope = copy;
		copy->top_path[n_index]->scope_location = SCOPE_LOCATION_TOP;
		copy->top_path[n_index]->scope_child_index = -1;
		copy->top_path[n_index]->scope_node_index = n_index;
	}
	for (int n_index = 0; n_index < (int)copy->top_path.size()-1; n_index++) {
		copy->top_path[n_index]->next = copy->top_path[n_index+1];
	}
	if (copy->top_path.size() > 0) {
		copy->top_path[copy->top_path.size()-1]->next = copy;
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		copy->children_nodes.push_back(vector<SolutionNode*>());
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			copy->children_nodes[c_index].push_back(this->children_nodes[c_index][n_index]->deep_copy(inclusive_start_layer));
		}
		for (int n_index = 0; n_index < (int)copy->children_nodes[c_index].size(); n_index++) {
			copy->children_nodes[c_index][n_index]->parent_scope = copy;
			copy->children_nodes[c_index][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
			copy->children_nodes[c_index][n_index]->scope_child_index = c_index;
			copy->children_nodes[c_index][n_index]->scope_node_index = n_index;
		}
		for (int n_index = 0; n_index < (int)copy->children_nodes[c_index].size()-1; n_index++) {
			copy->children_nodes[c_index][n_index]->next = copy->children_nodes[c_index][n_index+1];
		}
		if (copy->children_nodes[c_index].size() > 0) {
			copy->children_nodes[c_index][copy->children_nodes[c_index].size()-1]->next = copy;
		}
	}
}

void JumpScope::set_is_temp_node(bool is_temp_node) {
	this->is_temp_node = is_temp_node;

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->set_is_temp_node(is_temp_node);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->set_is_temp_node(is_temp_node);
		}
	}
}

void JumpScope::initialize_local_state(vector<int>& explore_node_local_state_sizes) {
	this->local_state_sizes.insert(this->local_state_sizes.begin(),
		explore_node_local_state_sizes.begin(), explore_node_local_state_sizes.end());

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->initialize_local_state(explore_node_local_state_sizes);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->initialize_local_state(explore_node_local_state_sizes);
		}
	}
}

void JumpScope::setup_flat(vector<int>& loop_scope_counts,
						   int& curr_index,
						   SolutionNode* explore_node) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->setup_flat(loop_scope_counts,
											curr_index,
											explore_node);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->setup_flat(loop_scope_counts,
															   curr_index,
															   explore_node);
		}
	}
}

void JumpScope::setup_new_state(SolutionNode* explore_node,
								int new_state_size) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->setup_new_state(explore_node,
												 new_state_size);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->setup_new_state(explore_node,
																	new_state_size);
		}
	}
}

void JumpScope::get_min_misguess(double& min_misguess) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->get_min_misguess(min_misguess);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->get_min_misguess(min_misguess);
		}
	}
}

void JumpScope::cleanup_explore(SolutionNode* explore_node) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->cleanup_explore(explore_node);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->cleanup_explore(explore_node);
		}
	}
}

void JumpScope::collect_new_state_networks(SolutionNode* explore_node,
										   vector<SolutionNode*>& existing_nodes,
										   vector<Network*>& new_state_networks) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->collect_new_state_networks(explore_node,
															existing_nodes,
															new_state_networks);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->collect_new_state_networks(explore_node,
																			   existing_nodes,
																			   new_state_networks);
		}
	}
}

void JumpScope::insert_scope(int layer,
							 int new_state_size) {
	this->local_state_sizes.insert(this->local_state_sizes.begin() + layer, new_state_size);

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->insert_scope(layer,
											  new_state_size);
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->insert_scope(layer,
																 new_state_size);
		}
	}
}

void JumpScope::reset_explore() {
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

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->reset_explore();
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->children_nodes[c_index].size(); n_index++) {
			this->children_nodes[c_index][n_index]->reset_explore();
		}
	}
}

SolutionNode* JumpScope::re_eval(Problem& problem,
								 vector<vector<double>>& state_vals,
								 vector<SolutionNode*>& scopes,
								 vector<int>& scope_states,
								 vector<ReEvalStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys) {
	if (scopes.back() != this) {
		// entering scope
		vector<double> scope_states;
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			scope_states.push_back(0.0);
		}
		state_vals.push_back(scope_states);

		scopes.push_back(this);
		scope_states.push_back(-1);

		instance_history.push_back(ReEvalStepHistory(this,
													 0.0,
													 JUMP_SCOPE_STATE_ENTER));

		if (this->top_path.size() > 0) {
			return this->top_path[0];
		}
	}

	if (scope_states.back() == -1) {
		// if condition
		if (rand()%10 == 0) {
			int random_index = rand()%this->children_nodes.size();
			vector<double> inputs = state_vals.back();
			inputs.push_back(problem.get_observation());
			this->children_score_networks[random_index]->mtx.lock();
			this->children_score_networks[random_index]->activate(inputs, network_historys);
			this->children_score_networks[random_index]->mtx.unlock();

			state_vals.back().clear();

			scope_states.back() = random_index;

			instance_history.push_back(ReEvalStepHistory(this,
														 0.0,
														 JUMP_SCOPE_STATE_IF));

			if (this->children_nodes[random_index].size() > 0) {
				return this->children_nodes[random_index][0];
			}
		} else {
			int best_index;
			activate_children_networks(problem,
									   state_vals.back(),
									   best_index,
									   network_historys);

			state_vals.back().clear();

			scope_states.back() = best_index;

			instance_history.push_back(ReEvalStepHistory(this,
														 0.0,
														 JUMP_SCOPE_STATE_IF));

			if (this->children_nodes[best_index].size() > 0) {
				return this->children_nodes[best_index][0];
			}
		}
	}

	// exiting scope
	state_vals.pop_back();

	scopes.pop_back();
	scope_states.pop_back();

	instance_history.push_back(ReEvalStepHistory(this,
												 0.0,
												 JUMP_SCOPE_STATE_EXIT));

	return this->next;
}

void JumpScope::re_eval_backprop(double score,
								 vector<vector<double>>& state_errors,
								 vector<ReEvalStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys) {
	if (instance_history.back()->scope_state == JUMP_SCOPE_STATE_EXIT) {
		vector<double> empty_local_state_errors;
		state_errors.push_back(empty_local_state_errors);
	} else if (instance_history.back()->scope_state == JUMP_SCOPE_STATE_IF) {
		state_errors.back().reserve(this->num_states);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			state_errors.back().push_back(0.0);
		}

		// search for and backprop right child network
	} else {
		// instance_history.back()->scope_state == JUMP_SCOPE_STATE_EXIT
		state_errors.pop_back();
	}

	instance_history.pop_back();
	return;
}

SolutionNode* JumpScope::explore(Problem& problem,
								 vector<vector<double>>& state_vals,
								 vector<SolutionNode*>& scopes,
								 vector<int>& scope_states,
								 vector<int>& scope_locations,
								 IterExplore*& iter_explore,
								 vector<ExploreStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys,
								 bool& abandon_instance) {	// TODO: in jump, if has score >1.0, abandon explore
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

	if (scopes.back() != this) {
		// entering scope
		vector<double> scope_states;
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			scope_states.push_back(0.0);
		}
		state_vals.push_back(scope_states);

		scopes.push_back(this);
		scope_states.push_back(-1);
		scope_locations.push_back(0);

		instance_history.push_back(ExploreStepHistory(this,
													  false,
													  0.0,
													  JUMP_SCOPE_STATE_ENTER,
													  -1,
													  false));

		if (this->top_path.size() > 0) {
			return this->top_path[0];
		}
	}

	if (scope_states.back() == -1) {
		// if condition
		bool state_affected = false;
		if (iter_explore != NULL) {
			if (this->local_state_sizes.size() < iter_explore->scopes.size()) {
				if (iter_explore->scopes[this->local_state_sizes.size()] == this) {
					if (iter_explore->scope_states[this->local_state_sizes.size()] == -1) {
						state_affected = true;
					}
				}
			}
		}

		bool should_backprop = false;
		if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT
				|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH
				|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH
				|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE
				|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			should_backprop = true;
		}

		if (state_affected) {
			if (rand()%2 == 0) {
				int random_index = rand()%this->children_nodes.size();
				vector<double> inputs = state_vals.back();
				inputs.push_back(problem.get_observation());
				if (should_backprop) {
					this->children_score_networks[random_index]->mtx.lock();
					this->children_score_networks[random_index]->activate(inputs, network_historys);
					this->children_score_networks[random_index]->mtx.unlock();
				} else {
					this->children_score_networks[random_index]->mtx.lock();
					this->children_score_networks[random_index]->activate(inputs);
					this->children_score_networks[random_index]->mtx.unlock();
				}

				state_vals.back().clear();

				scope_states.back() = random_index;
				scope_locations.back() = 0;

				instance_history.push_back(ExploreStepHistory(this,
															  false,
															  0.0,
															  JUMP_SCOPE_STATE_IF,
															  -1,
															  false));

				if (this->children_nodes[random_index].size() > 0) {
					return this->children_nodes[random_index][0];
				}
			} else {
				int best_index;
				if (should_backprop) {
					activate_children_networks(problem,
											   state_vals.back(),
											   best_index,
											   network_historys);
				} else {
					activate_children_networks(problem,
											   state_vals.back(),
											   best_index);
				}

				state_vals.back().clear();

				scope_states.back() = best_index;
				scope_locations.back() = 0;

				instance_history.push_back(ExploreStepHistory(this,
															  false,
															  0.0,
															  JUMP_SCOPE_STATE_IF,
															  -1,
															  false));

				if (this->children_nodes[best_index].size() > 0) {
					return this->children_nodes[best_index][0];
				}
			}
		} else {
			int best_index;
			activate_children_networks(problem,
									   state_vals.back(),
									   best_index);

			state_vals.back().clear();

			scope_states.back() = best_index;
			scope_locations.back() = 0;

			instance_history.push_back(ExploreStepHistory(this,
														  false,
														  0.0,
														  JUMP_SCOPE_STATE_IF,
														  -1,
														  false));

			if (this->children_nodes[best_index].size() > 0) {
				return this->children_nodes[best_index][0];
			}
		}
	}

	// exiting scope
	state_vals.pop_back();

	scopes.pop_back();
	scope_states.pop_back();
	scope_locations.pop_back();

	bool is_first_explore = false;
	if (iter_explore == NULL) {
		is_explore_helper(scopes,
						  scope_states,
						  scope_locations,
						  iter_explore,
						  is_first_explore);
	}

	// push StepHistory early for new_state check
	instance_history.push_back(ExploreStepHistory(this,
												  false,
												  0.0,
												  JUMP_SCOPE_STATE_EXIT,
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

void JumpScope::explore_backprop(double score,
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

	if (instance_history.back()->scope_state == JUMP_SCOPE_STATE_EXIT) {
		if (iter_explore != NULL
				&& iter_explore->explore_node == this) {
			explore_backprop_helper(score,
									instance_history,
									network_historys);
		}

		vector<double> empty_local_state_errors;
		state_errors.push_back(empty_local_state_errors);
	} else if (instance_history.back()->scope_state == JUMP_SCOPE_STATE_IF) {
		state_errors.back().reserve(this->num_states);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			state_errors.back().push_back(0.0);
		}

		// search for and backprop right child network
	} else {
		// instance_history.back()->scope_state == JUMP_SCOPE_STATE_EXIT
		state_errors.pop_back();
	}

	instance_history.pop_back();
	return;
}

void JumpScope::activate_children_networks(Problem& problem,
										   vector<double>& scope_state_vals,
										   int& best_index) {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		inputs.push_back(scope_state_vals[s_index]);
	}

	double best_score = numeric_limits<double>::lowest();
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		this->children_score_networks[c_index]->mtx.lock();
		this->children_score_networks[c_index]->activate(inputs);
		double predicted_score = this->children_score_networks[c_index]->output->acti_vals[0];
		this->children_score_networks[c_index]->mtx.unlock();

		if (predicted_score > best_score) {
			best_index = c_index;
			best_score = predicted_score;
		}
	}
}
