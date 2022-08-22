#include "solution_node_if_start.h"

using namespace std;

SolutionNodeIfStart::SolutionNodeIfStart(SolutionNode* parent,
										 int node_index,
										 Network* original_path_score_network) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_IF_START;

	this->network_inputs_state_indexes = parent->network_inputs_state_indexes;

	this->average_unique_future_nodes = 1;
	this->average_score = 0.0;
	this->average_misguess = 1.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->children_nodes.push_back(NULL);
	this->children_nodes.push_back(NULL);

	Network* original_path_network = new Network(original_path_score_network);
	this->children_score_networks.push_back(original_path_network);
	this->children_score_networks.push_back(parent->explore_if_network);
	parent->explore_if_network = NULL;

	string original_path_network_name = "../saves/nns/child_" + to_string(this->node_index) \
		+ "_0_" + to_string(time(NULL)) + ".txt";
	this->children_score_network_names.push_back(original_path_network_name);
	string new_path_network_name = "../saves/nns/child_" + to_string(this->node_index) \
		+ "_1_" + to_string(time(NULL)) + ".txt";
	this->children_score_network_names.push_back(new_path_network_name);

	this->children_on.push_back(false);
	this->children_on.push_back(false);

	for (int i = 0; i < 6; i++) {
		this->explore_state_scores.push_back(0.0);
		this->explore_state_misguesses.push_back(0.0);
	}
}

SolutionNodeIfStart::~SolutionNodeIfStart() {
	for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
		delete this->children_score_networks[c_index];
	}
}

void SolutionNodeIfStart::reset() override {
	this->scope_states_on.clear();
	this->scope_potential_states.clear();

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		this->children_on[c_index] = false;
	}
}

void SolutionNodeIfStart::add_potential_state(vector<int> potential_state_indexes,
											  SolutionNode* scope) override {
	if (scope != this) {
		for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
			this->network_inputs_potential_state_indexes.push_back(
				potential_state_indexes[ps_index]);

			for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
				// specific children will end up not supporting specific states, but that's OK
				this->children_score_networks[c_index]->add_potential();
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (this->children_on[c_index]) {
			this->children_nodes[c_index]->add_potential_state(potential_state_indexes,
															   scope);
		}
	}
	this->end->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeIfStart::extend_with_potential_state(vector<int> potential_state_indexes,
													  vector<int> new_state_indexes,
													  SolutionNode* scope) override {
	if (scope != this) {
		for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
			for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
				if (this->network_inputs_potential_state_indexes[pi_index]
						== potential_state_indexes[ps_index]) {
					this->network_inputs_state_indexes.push_back(new_state_indexes[ps_index]);

					for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
						this->children_score_networks[c_index]->extend_with_potential(pi_index);
					}

					break;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (this->children_on[c_index]) {
			this->children_nodes[c_index]->extend_with_potential_state(potential_state_indexes,
																	   scope);
		}
	}
	this->end->extend_with_potential_state(potential_state_indexes, scope);
}

void SolutionNodeIfStart::reset_potential_state(vector<int> potential_state_indexes,
												SolutionNode* scope) override {
	if (scope != this) {
		for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
			for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
				if (this->network_inputs_potential_state_indexes[pi_index]
						== potential_state_indexes[ps_index]) {
					for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
						this->children_score_networks[c_index]->reset_potential(pi_index);
					}
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (this->children_on[c_index]) {
			this->children_nodes[c_index]->reset_potential_state(potential_state_indexes,
																 scope);
		}
	}
	this->end->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeIfStart::clear_potential_state() override {
	this->network_inputs_potential_state_indexes.clear();

	for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
		this->children_score_networks[c_index]->remove_potentials();
	}
}

SolutionNode* SolutionNodeIfStart::activate(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											bool is_first_time,
											int& iter_explore_type,
											SolutionNode* iter_explore_node,
											double* potential_state_vals,
											bool* potential_states_on,
											vector<NetworkHistory*>& network_historys,
											vector<double>& guesses,
											vector<int>& explore_decisions,
											vector<bool>& explore_loop_decisions) override {
	if (iter_explore_type == EXPLORE_TYPE_NONE && is_first_time) {
		if (randuni() < (2.0/this->average_unique_future_nodes)) {
			if (rand()%2 == 0) {
				if (!this->has_explored_state) {
					if (this->explore_state_state == EXPLORE_STATE_STATE_LEARN) {
						int rand_states = 1 + rand()%5;
						for (int s_index = 0; s_index < rand_states; s_index++) {
							potential_states_on[this->scope_potential_states[s_index]] = true;
						}

						iter_explore_node = this;
						iter_explore_type = EXPLORE_TYPE_LEARN_STATE;
					} else if (this->explore_state_state == EXPLORE_STATE_STATE_MEASURE) {
						int rand_states = rand()%6;
						for (int s_index = 0; s_index < rand_states; s_index++) {
							potential_states_on[this->scope_potential_states[s_index]] = true;
						}

						iter_explore_node = this;
						iter_explore_type = EXPLORE_TYPE_MEASURE_STATE;
					}
				}
			} else {
				if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
					geometric_distribution<int> seq_length_dist(0.0, 0.2);
					int seq_length;

					vector<int> selectable_children;
					for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
						if (this->children_on[c_index]) {
							selectable_children.push_back(c_index);
						}
					}

					int random_index = -1 + rand()%((int)selectable_children.size()+1);
					if (rand_index == -1) {
						this->explore_child_index = -1;
					} else {
						this->explore_child_index = selectable_children[random_index];
					}
					if (this->explore_child_index == -1) {
						seq_length = seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = NULL;
						this->explore_end_inclusive = NULL;
						this->explore_end_non_inclusive = this->end;
					} else {
						vector<SolutionNode*> potential_inclusive_jump_ends;
						vector<SolutionNode*> potential_non_inclusive_jump_ends;
						potential_inclusive_jump_ends.push_back(this);
						potential_non_inclusive_jump_ends.push_back(this->children_nodes[this->explore_child_index]);
						find_potential_jumps(this->children_nodes[this->explore_child_index],
											 potential_inclusive_jump_ends,
											 potential_non_inclusive_jump_ends);

						int random_index = rand()%potential_inclusive_jump_ends.size();

						if (this == potential_inclusive_jump_ends[random_index]) {
							seq_length = 1 + seq_length_dist(generator);

							this->explore_start_non_inclusive = this;
							this->explore_start_inclusive = NULL;
							this->explore_end_inclusive = NULL;
							this->explore_end_non_inclusive = this->children_nodes[this->explore_child_index];
						} else {
							seq_length = seq_length_dist(generator);

							this->explore_start_non_inclusive = this;
							this->explore_start_inclusive = this->children_nodes[this->explore_child_index];
							this->explore_end_inclusive = potential_inclusive_jump_ends[random_index];
							this->explore_end_non_inclusive = potential_non_inclusive_jump_ends[random_index];
						}
					}

					this->path_explore_type = PATH_EXPLORE_TYPE_JUMP;

					try_path.clear();

					normal_distribution<double> write_val_dist(0.0, 2.0);
					for (int i = 0; i < seq_length; i++) {
						Action a(write_val_dist(generator), rand()%3);
						try_path.push_back(a);
					}

					if (rand()%2 == 0) {
						this->path_target_type = PATH_TARGET_TYPE_GOOD;
					} else {
						this->path_target_type = PATH_TARGET_TYPE_BAD;
					}

					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_EXPLORE;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_LEARN_PATH;
				} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE) {
					iter_explore_node = this;
					iter_explore_type = EXPLORE_TYPE_MEASURE_PATH;
				}
			}
		}
	}

	double best_score;
	int best_index;
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   true,
								   network_historys,
								   best_score,
								   best_index);
		guesses.push_back(best_score);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   true,
								   network_historys,
								   best_score,
								   best_index);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		activate_children_networks_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_states_on,
			true,
			network_historys,
			best_score,
			best_index);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index);
		if (iter_explore_node != this) {
			guesses.push_back(best_score);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		activate_children_networks_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_states_on,
			false,
			network_historys,
			best_score,
			best_index);
		guesses.push_back(best_score);
	}

	for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
		state_vals[this->scope_states_on[o_index]] = 0.0;
		states_on[this->scope_states_on[o_index]] = true;
	}

	SolutionNode* explore_node = NULL;
	if (iter_explore_node == this) {
		explore_node = explore(score,
							   problem,
							   state_vals,
							   states_on,
							   loop_scopes,
							   loop_scope_counts,
							   iter_explore_type,
							   iter_explore_node,
							   potential_state_errors,
							   potential_states_on,
							   network_historys,
							   guesses,
							   explore_decisions,
							   explore_diffs);
	}

	if (explore_node != NULL) {
		return explore_node;
	} else {
		return this->children_nodes[best_index];
	}
}

void SolutionNodeIfStart::backprop(double score,
								   double misguess,
								   double* state_errors,
								   bool* states_on,
								   int& iter_explore_type,
								   SolutionNode* iter_explore_node,
								   double* potential_state_errors,
								   bool* potential_states_on,
								   vector<NetworkHistory*>& network_historys,
								   vector<int>& explore_decisions,
								   vector<bool>& explore_loop_decisions) override {
	backprop_explore_and_score_network_helper(score,
											  misguess,
											  state_errors,
											  states_on,
											  iter_explore_type,
											  iter_explore_node,
											  potential_state_errors,
											  potential_states_on,
											  network_historys,
											  explore_decisions);

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		for (int o_index = 0; o_index < (int)this->start->scope_states_on.size(); o_index++) {
			states_on[this->start->scope_states_on[o_index]] = false;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
	}
}

void SolutionNodeIfStart::activate_children_networks(Problem& problem,
													 double* state_vals,
													 bool* states_on,
													 bool backprop,
													 vector<NetworkHistory*>& network_historys,
													 double& best_score,
													 int& best_index) {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			inputs.push_back(0.0);
		}
	}

	if (rand()%20 == 0) {
		vector<int> selectable_children;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				selectable_children.push_back(c_index);
			}
		}

		best_index = selectable_children[rand()%this->selectable_children.size()];
		if (backprop) {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(inputs, network_historys);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();
		} else {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(inputs);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();
		}
		return;
	}

	best_index = -1;
	best_score = numeric_limits<double>::lowest();
	
	if (backprop) {
		vector<NetworkHistory*> best_history;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				vector<NetworkHistory*> temp_history;
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(inputs, temp_history);
				double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				if (child_score > best_score) {
					best_index = c_index;
					best_score = child_score;
					if (best_history.size() > 0) {
						delete best_history[0];
						best_history.pop_back();
					}
					best_history.push_back(temp_history[0]);
				} else {
					delete temp_history[0];
				}
			}
		}
		network_historys.push_back(best_history[0]);
	} else {
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(inputs);
				double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				if (child_score > best_score) {
					best_index = c_index;
					best_score = child_score;
				}
			}
		}
	}

	return;
}

void SolutionNodeIfStart::backprop_children_networks(double score,
													 double* state_errors,
													 bool* states_on,
													 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	int child_index;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (network_history->network == this->children_score_networks[c_index]) {
			child_index = c_index;
			break;
		}
	}

	this->children_score_networks[child_index]->mtx.lock();

	network_history->reset_weights();

	vector<double> errors;
	if (score == 1.0) {
		if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
			errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	} else {
		if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
			errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	}
	this->children_score_networks[child_index]->backprop(errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->children_score_networks[child_index]->input->errors[1+i_index];
		}
		this->children_score_networks[child_index]->input->errors[1+i_index] = 0.0;
	}

	this->children_score_networks[child_index]->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNodeIfStart::backprop_children_networks_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	int child_index;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (network_history->network == this->children_score_networks[c_index]) {
			child_index = c_index;
			break;
		}
	}

	this->children_score_networks[child_index]->mtx.lock();

	network_history->reset_weights();

	vector<double> errors;
	if (score == 1.0) {
		if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
			errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	} else {
		if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
			errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	}
	this->children_score_networks[child_index]->backprop_errors_with_no_weight_change(errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->children_score_networks[child_index]->input->errors[1+i_index];
		}
		this->children_score_networks[child_index]->input->errors[1+i_index] = 0.0;
	}

	this->children_score_networks[child_index]->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNodeIfStart::activate_children_networks_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		bool* potential_states_on,
		bool backprop,
		vector<NetworkHistory*>& network_historys,
		double& best_score,
		int& best_index) {
	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->network_inputs_potential_state_indexes.size(); p_index++) {
		if (potential_states_on[this->network_inputs_potential_state_indexes[p_index]]) {
			potentials_on.push_back(p_index);
			potential_vals.push_back(potential_state_vals[this->>network_inputs_potential_state_indexes[p_index]]);
		}
	}

	if (potentials_on.size() == 0) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index);
		return;
	}

	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->halt_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->halt_networks_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->halt_networks_inputs_state_indexes[i_index]]);
		} else {
			inputs.push_back(0.0);
		}
	}

	if (rand()%20 == 0) {
		vector<int> selectable_children;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				selectable_children.push_back(c_index);
			}
		}

		best_index = selectable_children[rand()%this->selectable_children.size()];
		if (backprop) {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(
				inputs,
				potentials_on,
				potential_states_on,
				network_historys);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();
		} else {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(
				inputs,
				potentials_on,
				potential_states_on);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();
		}
	}

	best_index = -1;
	best_score = numeric_limits<double>::lowest();

	if (backprop) {
		vector<NetworkHistory*> best_history;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				vector<NetworkHistory*> temp_history;
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(
					inputs,
					potentials_on,
					potential_states_on,
					network_historys);
				double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				if (child_score > best_score) {
					best_index = c_index;
					best_score = child_score;
					if (best_history.size() > 0) {
						delete best_history[0];
						best_history.pop_back();
					}
					best_history.push_back(temp_history[0]);
				} else {
					delete temp_history[0];
				}
			}
		}
		network_historys.push_back(best_history[0]);
	} else {
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(
					inputs,
					potentials_on,
					potential_states_on);
				double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				if (child_score > best_score) {
					best_index = c_index;
					best_score = child_score;
				}
			}
		}
	}

	return;
}

void SolutionNodeIfStart::backprop_children_networks_with_potential(
		double score,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	int child_index;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (network_history->network == this->children_score_networks[c_index]) {
			child_index = c_index;
			break;
		}
	}

	this->children_score_networks[child_index]->mtx.lock();

	network_history->reset_weights();

	vector<int> potentials_on = network_history->potentials_on;

	vector<double> errors;
	if (score == 1.0) {
		if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
			errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	} else {
		if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
			errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			errors.push_back(0.0);
		}
	}
	this->children_score_networks[child_index]->backprop(errors, potentials_on);

	for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
		potential_state_errors[this->network_inputs_potential_state_indexes[potentials_on[o_index]]] += \
			this->children_score_networks[child_index]->potential_inputs[potentials_on[o_index]]->errors[0];
		this->children_score_networks[child_index]->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
	}

	this->children_score_networks[child_index]->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}
