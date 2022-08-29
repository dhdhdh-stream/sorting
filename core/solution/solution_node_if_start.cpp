#include "solution_node_if_start.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeIfStart::SolutionNodeIfStart(SolutionNode* parent,
										 int node_index) {
	this->solution = parent->solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_IF_START;

	this->node_weight = 0.0;

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->children_networks_inputs_state_indexes = parent->explore_network_inputs_state_indexes;

	this->children_nodes.push_back(NULL);
	this->children_nodes.push_back(NULL);

	this->children_score_networks.push_back(parent->explore_no_jump_network);
	parent->explore_no_jump_network = NULL;
	this->children_score_networks.push_back(parent->explore_jump_network);
	parent->explore_jump_network = NULL;

	int input_size = 1 + (int)this->children_networks_inputs_state_indexes.size();
	this->children_certainty_networks.push_back(new Network(input_size,
															4*input_size,
															1));
	this->children_certainty_networks.push_back(new Network(input_size,
															4*input_size,
															1));

	this->children_on.push_back(false);
	this->children_on.push_back(false);

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_network = NULL;
	this->explore_no_jump_network = NULL;
	this->explore_halt_network = NULL;
	this->explore_no_halt_network = NULL;

	this->node_is_on = false;
}

SolutionNodeIfStart::SolutionNodeIfStart(Solution* solution,
										 int node_index,
										 ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_IF_START;

	string children_networks_inputs_state_indexes_size_line;
	getline(save_file, children_networks_inputs_state_indexes_size_line);
	int children_networks_inputs_state_indexes_size = stoi(child_networks_inputs_state_indexes_size_line);
	for (int s_index = 0; s_index < children_networks_inputs_state_indexes_size; s_index++) {
		string state_index_line;
		getline(save_file, state_index_line);
		this->network_inputs_state_indexes.push_back(stoi(state_index_line));
	}

	string num_children_line;
	getline(save_file, num_children_line);
	int num_children = stoi(num_children_line);
	for (int c_index = 0; c_index < num_children; c_index++) {
		this->children_nodes.push_back(NULL);

		string child_score_network_name = "../saves/nns/child_score_" + to_string(this->node_index) \
			+ "_" + to_string(c_index) + "_" + to_string(this->solution->id) + ".txt";
		ifstream child_score_network_save_file;
		child_score_network_save_file.open(child_score_network_name);
		Network* child_score_network = new Network(child_score_network_save_file);
		this->children_score_networks.push_back(child_score_network);
		child_score_network_save_file.close();

		string child_certainty_network_name = "../saves/nns/child_certainty_" + to_string(this->node_index) \
			+ "_" + to_string(c_index) + "_" + to_string(this->solution->id) + ".txt";
		child_certainty_network_save_file.open(child_certainty_network_name);
		Network* child_certainty_network = new Network(child_certainty_network_save_file);
		this->children_certainty_networks.push_back(child_certainty_network);
		child_certainty_network_save_file.close();

		this->children_on.push_back(false);
	}

	string node_weight_line;
	getline(save_file, node_weight_line);
	this->node_weight = stof(node_weight_line);

	this->temp_node_state = TEMP_NODE_STATE_NOT;

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_network = NULL;
	this->explore_no_jump_network = NULL;
	this->explore_halt_network = NULL;
	this->explore_no_halt_network = NULL;

	this->node_is_on = false;
}

SolutionNodeIfStart::~SolutionNodeIfStart() {
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		delete this->children_score_networks[c_index];
		delete this->children_certainty_networks[c_index];
	}
}

void SolutionNodeIfStart::reset() {
	this->node_is_on = false;

	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		this->children_on[c_index] = false;
	}
}

void SolutionNodeIfStart::add_potential_state(vector<int> potential_state_indexes,
											  SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->children_networks_inputs_potential_state_indexes.push_back(
			potential_state_indexes[ps_index]);

		for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
			// specific children will end up not supporting specific states, but that's OK
			this->children_score_networks[c_index]->add_potential();
			this->children_certainty_networks[c_index]->add_potential();
		}
	}

	// this can't be explore node
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (this->children_on[c_index]) {
			this->children_nodes[c_index]->add_potential_state(potential_state_indexes,
															   explore_node);
		}
	}
	this->end->add_potential_state(potential_state_indexes, explore_node);
}

void SolutionNodeIfStart::extend_with_potential_state(vector<int> potential_state_indexes,
													  vector<int> new_state_indexes,
													  SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->children_networks_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->children_networks_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->children_networks_inputs_state_indexes.push_back(new_state_indexes[ps_index]);

				for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
					this->children_score_networks[c_index]->extend_with_potential(pi_index);
					this->children_certainty_networks[c_index]->extend_with_potential(pi_index);
				}

				this->children_networks_inputs_potential_state_indexes.erase(
					this->children_networks_inputs_potential_state_indexes.begin() + pi_index);

				break;
			}
		}
	}

	// this can't be explore node
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (this->children_on[c_index]) {
			this->children_nodes[c_index]->extend_with_potential_state(potential_state_indexes,
																	   new_state_indexes,
																	   explore_node);
		}
	}
	this->end->extend_with_potential_state(potential_state_indexes,
										   new_state_indexes,
										   explore_node);
}

void SolutionNodeIfStart::delete_potential_state(vector<int> potential_state_indexes,
												 SolutionNode* explore_node) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->children_networks_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->children_network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				for (int c_index = 0; c_index < (int)this->children_score_networks.size(); c_index++) {
					this->children_score_networks[c_index]->delete_potential(pi_index);
					this->children_certainty_networks[c_index]->delete_potential(pi_index);
				}

				this->children_networks_inputs_potential_state_indexes.erase(
					this->children_networks_inputs_potential_state_indexes.begin() + pi_index);

				break;
			}
		}
	}

	// this can't be explore node
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (this->children_on[c_index]) {
			this->children_nodes[c_index]->delete_potential_state(potential_state_indexes,
																  explore_node);
		}
	}
	this->end->delete_potential_state(potential_state_indexes, explore_node);
}

SolutionNode* SolutionNodeIfStart::activate(Problem& problem,
											double* state_vals,
											bool* states_on,
											vector<SolutionNode*>& loop_scopes,
											vector<int>& loop_scope_counts,
											int& iter_explore_type,
											SolutionNode*& iter_explore_node,
											IterExplore*& iter_explore,
											double* potential_state_vals,
											bool* potential_states_on,
											vector<NetworkHistory*>& network_historys,
											vector<vector<double>>& guesses,
											vector<int>& explore_decisions,
											vector<bool>& explore_loop_decisions,
											bool save_for_display,
											ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
	}

	bool is_first_explore = false;
	if (iter_explore_type == EXPLORE_TYPE_NONE) {
		if (randuni() < this->node_weight) {
			if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
				vector<int> selectable_children;
				for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
					if (this->children_on[c_index]) {
						selectable_children.push_back(c_index);
					}
				}
				int iter_child_index;
				int random_index = -1 + rand()%((int)selectable_children.size()+1);
				if (random_index == -1) {
					iter_child_index = -1;
				} else {
					iter_child_index = selectable_children[random_index];
				}

				geometric_distribution<int> seq_length_dist(0.2);
				normal_distribution<double> write_val_dist(0.0, 2.0);
				vector<Action> try_path;
				if (iter_child_index == -1) {
					int seq_length = seq_length_dist(generator);
					for (int i = 0; i < seq_length; i++) {
						Action a(write_val_dist(generator), rand()%3);
						try_path.push_back(a);
					}

					iter_explore = new IterExplore(
						ITER_EXPLORE_TYPE_JUMP,
						try_path,
						this,
						NULL,
						NULL,
						this->end,
						this->children_network_inputs_state_indexes,
						-1);
				} else {
					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					potential_inclusive_jump_ends.push_back(this);
					potential_non_inclusive_jump_ends.push_back(this->children_nodes[iter_child_index]);
					find_potential_jumps(this->children_nodes[iter_child_index],
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);

					int random_index = rand()%(int)potential_inclusive_jump_ends.size();

					if (this == potential_inclusive_jump_ends[random_index]) {
						int seq_length = 1 + seq_length_dist(generator);
						for (int i = 0; i < seq_length; i++) {
							Action a(write_val_dist(generator), rand()%3);
							try_path.push_back(a);
						}

						iter_explore = new IterExplore(
							ITER_EXPLORE_TYPE_JUMP,
							try_path,
							this,
							NULL,
							NULL,
							this->children_nodes[iter_child_index],
							this->children_network_inputs_state_indexes,
							iter_child_index);
					} else {
						int seq_length = seq_length_dist(generator);
						for (int i = 0; i < seq_length; i++) {
							Action a(write_val_dist(generator), rand()%3);
							try_path.push_back(a);
						}

						iter_explore = new IterExplore(
							ITER_EXPLORE_TYPE_JUMP,
							try_path,
							this,
							this->children_nodes[iter_child_index],
							potential_inclusive_jump_ends[random_index],
							potential_non_inclusive_jump_ends[random_index],
							this->children_network_inputs_state_indexes,
							iter_child_index);
					}
				}

				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_EXPLORE;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_JUMP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_JUMP;
			}

			is_first_explore = true;
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
								   best_index,
								   save_for_display,
								   display_file);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index,
								   save_for_display,
								   display_file);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index,
								   save_for_display,
								   display_file);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   true,
								   network_historys,
								   best_score,
								   best_index,
								   save_for_display,
								   display_file);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index,
								   save_for_display,
								   display_file);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		activate_children_networks_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_state_indexes,
			true,
			network_historys,
			best_score,
			best_index,
			save_for_display,
			display_file);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		activate_children_networks_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_state_indexes,
			false,
			network_historys,
			best_score,
			best_index,
			save_for_display,
			display_file);
	}
	vector<double> new_guess_segment;
	new_guess_segment.push_back(best_score);
	guesses.push_back(new_guess_segment);

	SolutionNode* explore_node = NULL;
	if (iter_explore_node == this) {
		explore_node = explore_activate(problem,
										state_vals,
										states_on,
										loop_scopes,
										loop_scope_counts,
										iter_explore_type,
										iter_explore_node,
										iter_explore,
										is_first_explore,
										potential_state_vals,
										potential_state_indexes,
										network_historys,
										explore_decisions);
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
								   SolutionNode*& iter_explore_node,
								   double* potential_state_errors,
								   vector<int>& potential_state_indexes,
								   vector<NetworkHistory*>& network_historys,
								   vector<int>& explore_decisions,
								   vector<bool>& explore_loop_decisions) {
	explore_backprop(score,
					 misguess,
					 state_errors,
					 states_on,
					 iter_explore_node,
					 potential_state_errors,
					 network_historys,
					 explore_decisions);

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_children_networks(score,
								   state_errors,
								   states_on,
								   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		backprop_children_networks_errors_with_no_weight_change(
			score,
			state_errors,
			states_on,
			network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		backprop_children_networks_with_potential(
			score,
			potential_state_errors,
			network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		// do nothing
	}
}

void SolutionNodeIfStart::save(ofstream& save_file) {
	save_file << this->children_networks_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->children_networks_inputs_state_indexes.size(); i_index++) {
		save_file << this->children_networks_inputs_state_indexes[i_index] << endl;
	}

	save_file << this->children_nodes.size() << endl;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		string child_score_network_name = "../saves/nns/child_score_" + to_string(this->node_index) \
			+ "_" + to_string(c_index) + "_" + to_string(this->solution->id) + ".txt";
		ofstream child_score_network_save_file;
		child_score_network_save_file.open(child_score_network_name);
		this->children_score_networks[c_index]->save(child_score_network_save_file);
		child_score_network_save_file.close();

		string child_certainty_network_name = "../saves/nns/child_certainty_" + to_string(this->node_index) \
			+ "_" + to_string(c_index) + "_" + to_string(this->solution->id) + ".txt";
		ofstream child_certainty_network_save_file;
		child_certainty_network_save_file.open(child_certainty_network_name);
		this->children_certainty_networks[c_index]->save(child_certainty_network_save_file);
		child_certainty_network_save_file.close();
	}

	save_file << this->node_weight << endl;
}

void SolutionNodeIfStart::save_for_display(ofstream& save_file) {
	save_file << this->node_is_on << endl;
	if (this->node_is_on) {
		save_file << this->node_type << endl;
		save_file << this->children_nodes.size() << endl;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			save_file << this->children_on[c_index] << endl;
			if (this->children_on[c_index]) {
				save_file << this->children_nodes[c_index]->node_index << endl;
			}
		}
	}
}

void SolutionNodeIfStart::activate_children_networks(Problem& problem,
													 double* state_vals,
													 bool* states_on,
													 bool backprop,
													 vector<NetworkHistory*>& network_historys,
													 double& best_score,
													 int& best_index,
													 bool save_for_display,
													 ofstream& display_file) {
	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->children_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->children_network_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->children_network_inputs_state_indexes[i_index]]);
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

		best_index = selectable_children[rand()%selectable_children.size()];
		if (backprop) {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(inputs, network_historys);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();

			this->children_certainty_networks[best_index]->mtx.lock();
			this->children_certainty_networks[best_index]->activate(inputs, network_historys);
			this->children_certainty_networks[best_index]->mtx.unlock();
		} else {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(inputs);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();
		}

		if (save_for_display) {
			display_file << -1 << endl;
			display_file << best_index << endl;
			display_file << best_score << endl;
		}

		return;
	}

	best_index = -1;
	best_score = 0.0;
	double best_combined = numeric_limits<double>::lowest();

	if (save_for_display) {
		display_file << this->children_nodes.size() << endl;
	}

	if (backprop) {
		vector<NetworkHistory*> best_score_history;
		vector<NetworkHistory*> best_certainty_history;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				vector<NetworkHistory*> temp_score_history;
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(inputs, temp_score_history);
				double predicted_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				vector<NetworkHistory*> temp_certainty_history;
				this->children_certainty_networks[c_index]->mtx.lock();
				this->children_certainty_networks[c_index]->activate(inputs, temp_certainty_history);
				double predicted_certainty = this->children_certainty_networks[c_index]->output->acti_vals[0];
				this->children_certainty_networks[c_index]->mtx.unlock();

				double pinned_score = max(min(predicted_score, 1.0), 0.0);
				double pinned_certainty = max(predicted_certainty, 0.0);
				double curr_combined = pinned_score - pinned_certainty;

				if (curr_combined > best_combined) {
					best_index = c_index;
					best_score = predicted_score;
					best_combined = curr_combined;
					if (best_score_history.size() > 0) {
						delete best_score_history[0];
						best_score_history.pop_back();
					}
					best_score_history.push_back(temp_score_history[0]);
					if (best_certainty_history.size() > 0) {
						delete best_certainty_history[0];
						best_certainty_history.pop_back();
					}
					best_certainty_history.push_back(temp_certainty_history[0]);
				} else {
					delete temp_score_history[0];
					delete temp_certainty_history[0];
				}

				if (save_for_display) {
					display_file << "on" << endl;
					display_file << curr_combined << endl;
				}
			} else {
				if (save_for_display) {
					display_file << "off" << endl;
				}
			}
		}
		network_historys.push_back(best_score_history[0]);
		network_historys.push_back(best_certainty_history[0]);
	} else {
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(inputs);
				double predicted_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				this->children_certainty_networks[c_index]->mtx.lock();
				this->children_certainty_networks[c_index]->activate(inputs);
				double predicted_certainty = this->children_certainty_networks[c_index]->output->acti_vals[0];
				this->children_certainty_networks[c_index]->mtx.unlock();

				double pinned_score = max(min(predicted_score, 1.0), 0.0);
				double pinned_certainty = max(predicted_certainty, 0.0);
				double curr_combined = pinned_score - pinned_certainty;

				if (curr_combined > best_combined) {
					best_index = c_index;
					best_score = predicted_score;
					best_combined = curr_combined;
				}

				if (save_for_display) {
					display_file << "on" << endl;
					display_file << curr_combined << endl;
				}
			} else {
				if (save_for_display) {
					display_file << "off" << endl;
				}
			}
		}
	}

	if (save_for_display) {
		display_file << best_index << endl;
	}

	return;
}

void SolutionNodeIfStart::backprop_children_networks(double score,
													 double misguess,
													 double* state_errors,
													 bool* states_on,
													 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	int child_index;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (score_network_history->network == this->children_certainty_networks[c_index]) {
			child_index = c_index;
			break;
		}
	}

	this->children_certainty_networks[child_index]->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->children_certainty_networks[child_index]->output->acti_vals[0]);
	this->children_certainty_networks[child_index]->backprop(certainty_errors);

	for (int i_index = 0; i_index < (int)this->children_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->children_networks_inputs_state_indexes[i_index]]) {
			state_errors[this->children_networks_inputs_state_indexes[i_index]] += \
				this->children_certainty_networks[child_index]->input->errors[1+i_index];
		}
		this->children_certainty_networks[child_index]->input->errors[1+i_index] = 0.0;
	}

	this->children_certainty_networks[child_index]->mtx.unlock();

	delete certainty_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->children_score_networks[child_index]->mtx.lock();

	score_network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->children_score_networks[child_index]->backprop(score_errors);

	for (int i_index = 0; i_index < (int)this->children_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->children_networks_inputs_state_indexes[i_index]]) {
			state_errors[this->children_networks_inputs_state_indexes[i_index]] += \
				this->children_score_networks[child_index]->input->errors[1+i_index];
		}
		this->children_score_networks[child_index]->input->errors[1+i_index] = 0.0;
	}

	this->children_score_networks[child_index]->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}

void SolutionNodeIfStart::backprop_children_networks_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	int child_index;
	for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
		if (network_history->network == this->children_certainty_networks[c_index]) {
			child_index = c_index;
			break;
		}
	}

	this->children_certainty_networks[child_index]->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->children_certainty_networks[child_index]->output->acti_vals[0]);
	this->children_certainty_networks[child_index]->backprop_errors_with_no_weight_change(certainty_errors);

	for (int i_index = 0; i_index < (int)this->children_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->children_networks_inputs_state_indexes[i_index]]) {
			state_errors[this->children_networks_inputs_state_indexes[i_index]] += \
				this->children_certainty_networks[child_index]->input->errors[1+i_index];
		}
		this->children_certainty_networks[child_index]->input->errors[1+i_index] = 0.0;
	}

	this->children_certainty_networks[child_index]->mtx.unlock();

	delete children_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->children_score_networks[child_index]->mtx.lock();

	score_network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->children_score_networks[child_index]->backprop_errors_with_no_weight_change(score_errors);

	for (int i_index = 0; i_index < (int)this->children_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->children_networks_inputs_state_indexes[i_index]]) {
			state_errors[this->children_networks_inputs_state_indexes[i_index]] += \
				this->children_score_networks[child_index]->input->errors[1+i_index];
		}
		this->children_score_networks[child_index]->input->errors[1+i_index] = 0.0;
	}

	this->children_score_networks[child_index]->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}

void SolutionNodeIfStart::activate_children_networks_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		vector<int>& potential_state_indexes,
		bool backprop,
		vector<NetworkHistory*>& network_historys,
		double& best_score,
		int& best_index,
		bool save_for_display,
		ofstream& display_file) {
	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->children_networks_inputs_potential_state_indexes.size(); p_index++) {
		if (potential_state_indexes[0] == this->children_networks_inputs_potential_state_indexes[p_index]) {
			for (int i = 0; i < 2; i++) {
				potentials_on.push_back(p_index+i);
				potential_vals.push_back(potential_state_vals[i]);
			}
			break;
		}
	}

	if (potentials_on.size() == 0) {
		activate_children_networks(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   best_score,
								   best_index,
								   save_for_display,
								   display_file);
		return;
	}

	vector<double> inputs;
	double curr_observations = problem.get_observation();
	inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->children_networks_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->children_networks_inputs_state_indexes[i_index]]) {
			inputs.push_back(state_vals[this->children_networks_inputs_state_indexes[i_index]]);
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

		best_index = selectable_children[rand()%selectable_children.size()];
		if (backprop) {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(
				inputs,
				potentials_on,
				potential_vals,
				network_historys);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();

			this->children_certainty_networks[best_index]->mtx.lock();
			this->children_certainty_networks[best_index]->activate(inputs, network_historys);
			this->children_certainty_networks[best_index]->mtx.unlock();
		} else {
			this->children_score_networks[best_index]->mtx.lock();
			this->children_score_networks[best_index]->activate(
				inputs,
				potentials_on,
				potential_vals);
			best_score = this->children_score_networks[best_index]->output->acti_vals[0];
			this->children_score_networks[best_index]->mtx.unlock();
		}

		if (save_for_display) {
			display_file << -1 << endl;
			display_file << best_index << endl;
			display_file << best_score << endl;
		}

		return;
	}

	best_index = -1;
	best_score = 0.0;
	double best_combined = numeric_limits<double>::lowest();

	if (save_for_display) {
		display_file << this->children_nodes.size() << endl;
	}

	if (backprop) {
		vector<NetworkHistory*> best_score_history;
		vector<NetworkHistory*> best_certainty_history;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				vector<NetworkHistory*> temp_score_history;
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(
					inputs,
					potentials_on,
					potential_vals,
					temp_score_history);
				double predicted_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				vector<NetworkHistory*> temp_certainty_history;
				this->children_certainty_networks[c_index]->mtx.lock();
				this->children_certainty_networks[c_index]->activate(
					inputs,
					potentials_on,
					potential_vals,
					temp_certainty_history);
				double predicted_certainty = this->children_certainty_networks[c_index]->output->acti_vals[0];
				this->children_certainty_networks[c_index]->unlock();

				double pinned_score = max(min(predicted_score, 1.0), 0.0);
				double pinned_certainty = max(predicted_score, 0.0);
				double curr_combined = pinned_score - pinned_certainty;

				if (curr_combined > best_combined) {
					best_index = c_index;
					best_score = predicted_score;
					best_combined = curr_combined;
					if (best_score_history.size() > 0) {
						delete best_score_history[0];
						best_score_history.pop_back();
					}
					best_score_history.push_back(temp_score_history[0]);
					if (best_certainty_history.size() > 0) {
						delete best_certainty_history[0];
						best_certainty_history.pop_back();
					}
					best_certainty_history.push_back(temp_certainty_history[0]);
				} else {
					delete temp_score_history[0];
					delete temp_certainty_history[0];
				}

				if (save_for_display) {
					display_file << "on" << endl;
					display_file << curr_combined << endl;
				}
			} else {
				if (save_for_display) {
					display_file << "off" << endl;
				}
			}
		}
		network_historys.push_back(best_score_history[0]);
		network_historys.push_back(best_certainty_history[0]);
	} else {
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (this->children_on[c_index]) {
				this->children_score_networks[c_index]->mtx.lock();
				this->children_score_networks[c_index]->activate(
					inputs,
					potentials_on,
					potential_vals);
				double predicted_score = this->children_score_networks[c_index]->output->acti_vals[0];
				this->children_score_networks[c_index]->mtx.unlock();

				this->children_certainty_networks[c_index]->mtx.lock();
				this->children_certainty_networks[c_index]->activate(
					inputs,
					potentials_on,
					potential_vals);
				double predicted_certainty = this->children_certainty_networks[c_index]->output->acti_vals[0];
				this->children_certainty_networks[c_index]->mtx.unlock();

				double pinned_score = max(min(predicted_score, 1.0), 0.0);
				double pinned_certainty = max(predicted_certainty, 0.0);
				double curr_combined = pinned_score - pinned_certainty;

				if (curr_combined > best_combined) {
					best_index = c_index;
					best_score = predicted_score;
					best_combined = curr_combined;
				}

				if (save_for_display) {
					display_file << "on" << endl;
					display_file << curr_combined << endl;
				}
			} else {
				if (save_for_display) {
					display_file << "off" << endl;
				}
			}
		}
	}

	if (save_for_display) {
		display_file << best_index << endl;
	}

	return;
}

void SolutionNodeIfStart::backprop_children_networks_with_potential(
		double score,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	if (network_historys.size() > 0) {
		NetworkHistory* certainty_network_history = network_historys.back();

		int child_index = -1;
		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			if (network_history->network == this->children_certainty_networks[c_index]) {
				child_index = c_index;
				break;
			}
		}

		if (child_index != -1) {
			this->children_certainty_networks[child_index]->mtx.lock();

			vector<int> certainty_potentials_on = certainty_network_history->potentials_on;

			vector<double> certainty_errors;
			certainty_errors.push_back(misguess - this->children_certainty_networks[child_index]->output->acti_vals[0]);
			this->children_certainty_networks[child_index]->backprop(certainty_errors, certainty_potentials_on);

			for (int o_index = 0; o_index < (int)certainty_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->children_certainty_networks[child_index]->
					potential_inputs[certainty_potentials_on[o_index]]->errors[0];
				this->children_certainty_networks[child_index]->potential_inputs[certainty_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->children_certainty_networks[child_index]->mtx.unlock();

			delete certainty_network_history;
			network_historys.pop_back();

			NetworkHistory* score_network_history = network_historys.back();

			this->children_score_networks[child_index]->mtx.lock();

			score_network_history->reset_weights();

			vector<int> score_potentials_on = score_network_history->potentials_on;

			vector<double> score_errors;
			if (score == 1.0) {
				if (this->children_score_networks[child_index]->output->acti_vals[0] < 1.0) {
					score_errors.push_back(1.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (this->children_score_networks[child_index]->output->acti_vals[0] > 0.0) {
					score_errors.push_back(0.0 - this->children_score_networks[child_index]->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			}
			this->children_score_networks[child_index]->backprop(score_errors, score_potentials_on);

			for (int o_index = 0; o_index < (int)score_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->children_score_networks[child_index]->
					potential_inputs[score_potentials_on[o_index]]->errors[0];
				this->children_score_networks[child_index]->potential_inputs[score_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->children_score_networks[child_index]->mtx.unlock();

			delete score_network_history;
			network_historys.pop_back();
		}
	}
}
