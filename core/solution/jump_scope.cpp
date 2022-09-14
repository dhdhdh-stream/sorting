#include "jump_scope.h"

#include <iostream>
#include <sstream>

#include "definitions.h"
#include "solution_node_action.h"
#include "solution_node_empty.h"
#include "solution_node_utilities.h"
#include "start_scope.h"
#include "utilities.h"

using namespace std;

JumpScope::JumpScope(vector<int> local_state_sizes,
					 int num_states) {
	this->node_type = NODE_TYPE_JUMP_SCOPE;

	this->local_state_sizes = local_state_sizes;
	this->num_states = num_states;

	this->node_weight = 0.0;

	this->explore_weight = 0.0;

	this->explore_state = EXPLORE_STATE_EXPLORE;

	this->explore_jump_score_network = NULL;
	this->explore_no_jump_score_network = NULL;

	this->explore_small_jump_score_network = NULL;
	this->explore_small_no_jump_score_network = NULL;

	this->is_temp_node = false;
}

JumpScope::JumpScope(std::vector<int>& scope_states,
					 std::vector<int>& scope_locations,
					 std::ifstream& save_file) {
	this->node_type = NODE_TYPE_JUMP_SCOPE;

	string num_local_state_sizes_line;
	getline(save_file, num_local_state_sizes_line);
	int num_local_state_sizes = stoi(num_local_state_sizes_line);
	for (int l_index = 0; l_index < num_local_state_sizes; l_index++) {
		string state_size_line;
		getline(save_file, state_size_line);
		this->local_state_sizes.push_back(stoi(state_size_line));
	}

	ostringstream node_name_oss;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		node_name_oss << scope_states[l_index] << "_" << scope_locations[l_index] << "_";
	}
	string node_name = node_name_oss.str();

	string num_states_line;
	getline(save_file, num_states_line);
	this->num_states = stoi(num_states_line);

	string num_child_line;
	getline(save_file, num_child_line);
	int num_child = stoi(num_child_line);
	for (int c_index = 0; c_index < num_child; c_index++) {
		string child_score_network_name = "../saves/nns/" + node_name + "child_" \
			+ to_string(c_index) + "_" + to_string(id) + ".txt";
		ifstream child_score_network_save_file;
		child_score_network_save_file.open(child_score_network_name);
		Network* child_score_network = new Network(child_score_network_save_file);
		this->child_score_networks.push_back(child_score_network);
		child_score_network_save_file.close();
	}

	string node_weight_line;
	getline(save_file, node_weight_line);
	this->node_weight = stof(node_weight_line);

	string explore_weight_line;
	getline(save_file, explore_weight_line);
	this->explore_weight = stof(explore_weight_line);

	string top_path_size_line;
	getline(save_file, top_path_size_line);
	int top_path_size = stoi(top_path_size_line);
	this->top_path.reserve(top_path_size);
	scope_states.push_back(-1);
	scope_locations.push_back(0);
	for (int n_index = 0; n_index < top_path_size; n_index++) {
		string node_type_line;
		getline(save_file, node_type_line);
		int node_type = stoi(node_type_line);
		SolutionNode* new_node;
		if (node_type == NODE_TYPE_EMPTY) {
			new_node = new SolutionNodeEmpty(scope_states,
											 scope_locations,
											 save_file);
		} else if (node_type == NODE_TYPE_ACTION) {
			new_node = new SolutionNodeAction(scope_states,
											  scope_locations,
											  save_file);
		} else {
			// node_type == NODE_TYPE_JUMP_SCOPE
			new_node = new JumpScope(scope_states,
									 scope_locations,
									 save_file);
		}
		this->top_path.push_back(new_node);
	}

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->parent_scope = this;
		this->top_path[n_index]->scope_location = SCOPE_LOCATION_TOP;
		this->top_path[n_index]->scope_child_index = -1;
		this->top_path[n_index]->scope_node_index = n_index;
	}
	for (int n_index = 0; n_index < (int)this->top_path.size()-1; n_index++) {
		this->top_path[n_index]->next = this->top_path[n_index+1];
	}
	if (this->top_path.size() > 0) {
		this->top_path[this->top_path.size()-1]->next = this;
	}

	for (int c_index = 0; c_index < num_child; c_index++) {
		string child_paths_size_line;
		getline(save_file, child_paths_size_line);
		int child_paths_size = stoi(child_paths_size_line);
		this->child_paths.push_back(vector<SolutionNode*>());
		this->child_paths[c_index].reserve(child_paths_size);
		scope_states.back() = c_index;
		scope_locations.back() = 0;
		for (int n_index = 0; n_index < child_paths_size; n_index++) {
			string node_type_line;
			getline(save_file, node_type_line);
			int node_type = stoi(node_type_line);
			SolutionNode* new_node;
			if (node_type == NODE_TYPE_EMPTY) {
				new_node = new SolutionNodeEmpty(scope_states,
												 scope_locations,
												 save_file);
			} else if (node_type == NODE_TYPE_ACTION) {
				new_node = new SolutionNodeAction(scope_states,
												  scope_locations,
												  save_file);
			} else {
				// node_type == NODE_TYPE_JUMP_SCOPE
				new_node = new JumpScope(scope_states,
										 scope_locations,
										 save_file);
			}
			this->child_paths[c_index].push_back(new_node);
		}

		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->parent_scope = this;
			this->child_paths[c_index][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
			this->child_paths[c_index][n_index]->scope_child_index = c_index;
			this->child_paths[c_index][n_index]->scope_node_index = n_index;
		}
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size()-1; n_index++) {
			this->child_paths[c_index][n_index]->next = this->child_paths[c_index][n_index+1];
		}
		if (this->child_paths[c_index].size() > 0) {
			this->child_paths[c_index][this->child_paths[c_index].size()-1]->next = this;
		}
	}

	scope_states.pop_back();
	scope_locations.pop_back();

	scope_locations.back()++;

	this->explore_state = EXPLORE_STATE_EXPLORE;

	this->explore_jump_score_network = NULL;
	this->explore_no_jump_score_network = NULL;

	this->explore_small_jump_score_network = NULL;
	this->explore_small_no_jump_score_network = NULL;

	this->is_temp_node = false;
}

JumpScope::~JumpScope() {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		delete this->top_path[n_index];
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			delete this->child_paths[c_index][n_index];
		}

		delete this->child_score_networks[c_index];
	}
}

SolutionNode* JumpScope::re_eval(Problem& problem,
								 double& predicted_score,
								 vector<vector<double>>& state_vals,
								 vector<SolutionNode*>& scopes,
								 vector<int>& scope_states,
								 vector<ReEvalStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys) {
	if (scopes.back() != this) {
		// entering scope
		vector<double> scope_state_vals;
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			scope_state_vals.push_back(0.0);
		}
		state_vals.push_back(scope_state_vals);

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
			int random_index = rand()%(int)this->child_paths.size();
			vector<double> inputs = state_vals.back();
			inputs.push_back(problem.get_observation());
			this->child_score_networks[random_index]->mtx.lock();
			this->child_score_networks[random_index]->activate(inputs, network_historys);
			this->child_score_networks[random_index]->mtx.unlock();

			state_vals.back().clear();

			scope_states.back() = random_index;

			instance_history.push_back(ReEvalStepHistory(this,
														 0.0,
														 JUMP_SCOPE_STATE_IF));

			if (this->child_paths[random_index].size() > 0) {
				return this->child_paths[random_index][0];
			}
		} else {
			int best_index;
			double best_score;
			activate_child_networks(problem,
									state_vals.back(),
									best_index,
									best_score,
									network_historys);

			state_vals.back().clear();

			scope_states.back() = best_index;

			instance_history.push_back(ReEvalStepHistory(this,
														 0.0,
														 JUMP_SCOPE_STATE_IF));

			if (this->child_paths[best_index].size() > 0) {
				return this->child_paths[best_index][0];
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
	if (instance_history.back().scope_state == JUMP_SCOPE_STATE_EXIT) {
		vector<double> empty_local_state_errors;
		state_errors.push_back(empty_local_state_errors);
	} else if (instance_history.back().scope_state == JUMP_SCOPE_STATE_IF) {
		state_errors.back().reserve(this->num_states);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			state_errors.back().push_back(0.0);
		}

		backprop_child_networks(score,
								state_errors.back(),
								network_historys);
	} else {
		// instance_history.back().scope_state == JUMP_SCOPE_STATE_EXIT
		state_errors.pop_back();
	}

	instance_history.pop_back();
	return;
}

SolutionNode* JumpScope::explore(Problem& problem,
								 double& predicted_score,
								 vector<vector<double>>& state_vals,
								 vector<SolutionNode*>& scopes,
								 vector<int>& scope_states,
								 vector<int>& scope_locations,
								 IterExplore*& iter_explore,
								 vector<ExploreStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys,
								 bool& abandon_instance) {
	if (iter_explore != NULL
			&& iter_explore->explore_node == this
			&& scopes.back() == NULL) {
		explore_callback_helper(problem,
								state_vals,
								scopes,
								scope_states,
								scope_locations,
								instance_history,
								network_historys);

		instance_history.push_back(ExploreStepHistory(this,
													  false,
													  0.0,
													  -1,
													  -1,
													  true));
		return get_jump_end(iter_explore, this);
	}

	if (scopes.back() != this) {
		// entering scope
		vector<double> scope_state_vals;
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			scope_state_vals.push_back(0.0);
		}
		state_vals.push_back(scope_state_vals);

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
		if (iter_explore != NULL) {
			if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT
					|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH
					|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH
					|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE
					|| iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
				should_backprop = true;
			}
		}

		if (state_affected) {
			if (rand()%2 == 0) {
				int random_index = rand()%(int)this->child_paths.size();
				vector<double> inputs = state_vals.back();
				inputs.push_back(problem.get_observation());
				if (should_backprop) {
					this->child_score_networks[random_index]->mtx.lock();
					this->child_score_networks[random_index]->activate(inputs, network_historys);
					this->child_score_networks[random_index]->mtx.unlock();
				} else {
					this->child_score_networks[random_index]->mtx.lock();
					this->child_score_networks[random_index]->activate(inputs);
					this->child_score_networks[random_index]->mtx.unlock();
				}
				// don't abandon even if predicted score > 1.0

				state_vals.back().clear();

				scope_states.back() = random_index;
				scope_locations.back() = 0;

				instance_history.push_back(ExploreStepHistory(this,
															  false,
															  0.0,
															  JUMP_SCOPE_STATE_IF,
															  -1,
															  false));

				if (this->child_paths[random_index].size() > 0) {
					return this->child_paths[random_index][0];
				}
			} else {
				int best_index;
				double best_score;
				if (should_backprop) {
					activate_child_networks(problem,
											state_vals.back(),
											best_index,
											best_score,
											network_historys);
				} else {
					activate_child_networks(problem,
											state_vals.back(),
											best_index,
											best_score);
				}
				// don't abandon even if predicted score > 1.0

				state_vals.back().clear();

				scope_states.back() = best_index;
				scope_locations.back() = 0;

				instance_history.push_back(ExploreStepHistory(this,
															  false,
															  0.0,
															  JUMP_SCOPE_STATE_IF,
															  -1,
															  false));

				if (this->child_paths[best_index].size() > 0) {
					return this->child_paths[best_index][0];
				}
			}
		} else {
			int best_index;
			double best_score;
			activate_child_networks(problem,
									state_vals.back(),
									best_index,
									best_score);
			// TODO: break if predicted score > 1.0

			state_vals.back().clear();

			scope_states.back() = best_index;
			scope_locations.back() = 0;

			instance_history.push_back(ExploreStepHistory(this,
														  false,
														  0.0,
														  JUMP_SCOPE_STATE_IF,
														  -1,
														  false));

			if (this->child_paths[best_index].size() > 0) {
				return this->child_paths[best_index][0];
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
		return explore_helper(is_first_explore,
							  problem,
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
								 vector<ExploreStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys) {
	if (instance_history.back().is_explore_callback) {
		// iter_explore->explore_node == this
		explore_callback_backprop_helper(state_errors,
										 instance_history,
										 network_historys);

		instance_history.pop_back();
		return;
	}

	if (instance_history.back().scope_state == JUMP_SCOPE_STATE_EXIT) {
		if (iter_explore != NULL
				&& iter_explore->explore_node == this) {
			explore_backprop_helper(score,
									instance_history,
									network_historys);
		}

		vector<double> empty_local_state_errors;
		state_errors.push_back(empty_local_state_errors);
	} else if (instance_history.back().scope_state == JUMP_SCOPE_STATE_IF) {
		state_errors.back().reserve(this->num_states);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			state_errors.back().push_back(0.0);
		}

		backprop_child_networks_errors_with_no_weight_change(score,
															 state_errors.back(),
															 network_historys);
	} else {
		// instance_history.back()->scope_state == JUMP_SCOPE_STATE_EXIT
		state_errors.pop_back();
	}

	instance_history.pop_back();
	return;
}

void JumpScope::explore_increment(double score,
								  IterExplore*& iter_explore) {
	explore_increment_helper(score,
							 iter_explore);
}

void JumpScope::re_eval_increment() {
	this->node_weight = 0.9999*this->node_weight;

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->re_eval_increment();
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->re_eval_increment();
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

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		copy->child_paths.push_back(vector<SolutionNode*>());
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			copy->child_paths[c_index].push_back(this->child_paths[c_index][n_index]->deep_copy(inclusive_start_layer));
		}
		for (int n_index = 0; n_index < (int)copy->child_paths[c_index].size(); n_index++) {
			copy->child_paths[c_index][n_index]->parent_scope = copy;
			copy->child_paths[c_index][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
			copy->child_paths[c_index][n_index]->scope_child_index = c_index;
			copy->child_paths[c_index][n_index]->scope_node_index = n_index;
		}
		for (int n_index = 0; n_index < (int)copy->child_paths[c_index].size()-1; n_index++) {
			copy->child_paths[c_index][n_index]->next = copy->child_paths[c_index][n_index+1];
		}
		if (copy->child_paths[c_index].size() > 0) {
			copy->child_paths[c_index][copy->child_paths[c_index].size()-1]->next = copy;
		}

		copy->child_score_networks.push_back(new Network(this->child_score_networks[c_index]));
	}

	return copy;
}

void JumpScope::set_is_temp_node(bool is_temp_node) {
	this->is_temp_node = is_temp_node;

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->set_is_temp_node(is_temp_node);
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->set_is_temp_node(is_temp_node);
		}
	}
}

void JumpScope::initialize_local_state(vector<int>& explore_node_local_state_sizes) {
	this->local_state_sizes.insert(this->local_state_sizes.begin(),
		explore_node_local_state_sizes.begin(), explore_node_local_state_sizes.end());

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->initialize_local_state(explore_node_local_state_sizes);
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->initialize_local_state(explore_node_local_state_sizes);
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

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->setup_flat(loop_scope_counts,
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

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->setup_new_state(explore_node,
																 new_state_size);
		}
	}
}

void JumpScope::get_min_misguess(double& min_misguess) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->get_min_misguess(min_misguess);
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->get_min_misguess(min_misguess);
		}
	}
}

void JumpScope::cleanup_explore(SolutionNode* explore_node) {
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->cleanup_explore(explore_node);
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->cleanup_explore(explore_node);
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

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->collect_new_state_networks(explore_node,
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

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->insert_scope(layer,
															  new_state_size);
		}
	}
}

void JumpScope::reset_explore() {
	this->explore_state = EXPLORE_STATE_EXPLORE;

	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		this->explore_path[n_index]->cleanup_explore(this);
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
		this->explore_jump_score_network = NULL;
	}
	if (this->explore_no_jump_score_network != NULL) {
		delete this->explore_no_jump_score_network;
		this->explore_no_jump_score_network = NULL;
	}

	if (this->explore_small_jump_score_network != NULL) {
		delete this->explore_small_jump_score_network;
		this->explore_small_jump_score_network = NULL;
	}
	if (this->explore_small_no_jump_score_network != NULL) {
		delete this->explore_small_no_jump_score_network;
		this->explore_small_no_jump_score_network = NULL;
	}

	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		this->top_path[n_index]->reset_explore();
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			this->child_paths[c_index][n_index]->reset_explore();
		}
	}
}

void JumpScope::save(std::vector<int>& scope_states,
					 std::vector<int>& scope_locations,
					 std::ofstream& save_file) {
	ostringstream node_name_oss;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		node_name_oss << scope_states[l_index] << "_" << scope_locations[l_index] << "_";
	}
	string node_name = node_name_oss.str();

	save_file << this->local_state_sizes.size() << endl;
	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		save_file << this->local_state_sizes[l_index] << endl;
	}
	save_file << this->num_states << endl;

	save_file << this->child_paths.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		string child_score_network_name = "../saves/nns/" + node_name + "child_" \
			+ to_string(c_index) + "_" + to_string(id) + ".txt";
		ofstream child_score_network_save_file;
		child_score_network_save_file.open(child_score_network_name);
		this->child_score_networks[c_index]->save(child_score_network_save_file);
		child_score_network_save_file.close();
	}

	save_file << this->node_weight << endl;

	save_file << this->explore_weight << endl;

	save_file << this->top_path.size() << endl;
	scope_states.push_back(-1);
	scope_locations.push_back(0);
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		save_file << this->top_path[n_index]->node_type << endl;
		this->top_path[n_index]->save(scope_states,
									  scope_locations,
									  save_file);
	}

	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		save_file << this->child_paths[c_index].size() << endl;
		scope_states.back() = c_index;
		scope_locations.back() = 0;
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			save_file << this->child_paths[c_index][n_index]->node_type << endl;
			this->child_paths[c_index][n_index]->save(scope_states,
													  scope_locations,
													  save_file);
		}
	}

	scope_states.pop_back();
	scope_locations.pop_back();

	scope_locations.back()++;
}

void JumpScope::save_for_display(std::ofstream& save_file) {
	save_file << this->explore_weight << endl;

	save_file << this->top_path.size() << endl;
	for (int n_index = 0; n_index < (int)this->top_path.size(); n_index++) {
		save_file << this->top_path[n_index]->node_type << endl;
		this->top_path[n_index]->save_for_display(save_file);
	}

	save_file << this->child_paths.size() << endl;
	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		save_file << this->child_paths[c_index].size() << endl;
		for (int n_index = 0; n_index < (int)this->child_paths[c_index].size(); n_index++) {
			save_file << this->child_paths[c_index][n_index]->node_type << endl;
			this->child_paths[c_index][n_index]->save_for_display(save_file);
		}
	}
}

void JumpScope::activate_child_networks(Problem& problem,
										vector<double>& layer_state_vals,
										int& best_index,
										double& best_score) {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		inputs.push_back(layer_state_vals[s_index]);
	}

	best_score = numeric_limits<double>::lowest();
	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		this->child_score_networks[c_index]->mtx.lock();
		this->child_score_networks[c_index]->activate(inputs);
		double predicted_score = this->child_score_networks[c_index]->output->acti_vals[0];
		this->child_score_networks[c_index]->mtx.unlock();

		if (predicted_score > best_score) {
			best_index = c_index;
			best_score = predicted_score;
		}
	}
}

void JumpScope::activate_child_networks(Problem& problem,
										vector<double>& layer_state_vals,
										int& best_index,
										double& best_score,
										vector<AbstractNetworkHistory*>& network_historys) {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		inputs.push_back(layer_state_vals[s_index]);
	}

	best_score = numeric_limits<double>::lowest();
	vector<AbstractNetworkHistory*> best_history;
	for (int c_index = 0; c_index < (int)this->child_paths.size(); c_index++) {
		vector<AbstractNetworkHistory*> temp_history;
		this->child_score_networks[c_index]->mtx.lock();
		this->child_score_networks[c_index]->activate(inputs, temp_history);
		double predicted_score = this->child_score_networks[c_index]->output->acti_vals[0];
		this->child_score_networks[c_index]->mtx.unlock();

		if (predicted_score > best_score) {
			best_index = c_index;
			best_score = predicted_score;
			if (best_history.size() > 0) {
				delete best_history[0];
				best_history.pop_back();
			}
			best_history.push_back(temp_history[0]);
		} else {
			delete temp_history[0];
		}
	}

	network_historys.push_back(best_history[0]);
}

void JumpScope::backprop_child_networks(double score,
										vector<double>& layer_state_errors,
										vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	int matching_index;	// has to be matching index
	for (int c_index = 0; c_index < (int)this->child_score_networks.size(); c_index++) {
		if (network_history->network == this->child_score_networks[c_index]) {
			matching_index = c_index;
			break;
		}
	}

	this->child_score_networks[matching_index]->mtx.lock();

	network_history->reset_weights();

	vector<double> errors;
	if (score == 1.0) {
		if (this->child_score_networks[matching_index]->output->acti_vals[0] < 1.0) {
			errors.push_back(1.0 - this->child_score_networks[matching_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	} else {
		if (this->child_score_networks[matching_index]->output->acti_vals[0] > 0.0) {
			errors.push_back(0.0 - this->child_score_networks[matching_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	}
	this->child_score_networks[matching_index]->backprop(errors);

	for (int s_index = 0; s_index < this->num_states; s_index++) {
		layer_state_errors[s_index] = this->child_score_networks[matching_index]->input->errors[1+s_index];
		this->child_score_networks[matching_index]->input->errors[1+s_index] = 0.0;
	}

	this->child_score_networks[matching_index]->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void JumpScope::backprop_child_networks_errors_with_no_weight_change(
		double score,
		vector<double>& layer_state_errors,
		vector<AbstractNetworkHistory*>& network_historys) {
	if (network_historys.size() == 0) {
		return;
	}

	AbstractNetworkHistory* network_history = network_historys.back();

	int matching_index = -1;
	for (int c_index = 0; c_index < (int)this->child_score_networks.size(); c_index++) {
		if (network_history->network == this->child_score_networks[c_index]) {
			matching_index = c_index;
			break;
		}
	}

	if (matching_index != -1) {
		this->child_score_networks[matching_index]->mtx.lock();

		network_history->reset_weights();

		vector<double> errors;
		if (score == 1.0) {
			if (this->child_score_networks[matching_index]->output->acti_vals[0] < 1.0) {
				errors.push_back(1.0 - this->child_score_networks[matching_index]->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		} else {
			if (this->child_score_networks[matching_index]->output->acti_vals[0] > 0.0) {
				errors.push_back(0.0 - this->child_score_networks[matching_index]->output->acti_vals[0]);
			} else {
				errors.push_back(0.0);
			}
		}
		this->child_score_networks[matching_index]->backprop_errors_with_no_weight_change(errors);

		for (int s_index = 0; s_index < this->num_states; s_index++) {
			layer_state_errors[s_index] = this->child_score_networks[matching_index]->input->errors[1+s_index];
			this->child_score_networks[matching_index]->input->errors[1+s_index] = 0.0;
		}

		this->child_score_networks[matching_index]->mtx.unlock();

		delete network_history;
		network_historys.pop_back();
	}
}
