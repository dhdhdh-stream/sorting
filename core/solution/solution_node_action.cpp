#include "solution_node_action.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/trim.hpp>

#include "definitions.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNodeAction::SolutionNodeAction(Solution* solution,
									   int node_index,
									   Action action,
									   vector<int> available_state) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_ACTION;

	this->network_inputs_state_indexes = available_state;

	int input_size = 1 + (int)this->network_inputs_state_indexes.size();
	this->score_network = new Network(input_size,
									  4*input_size,
									  1);
	this->certainty_network = new Network(input_size,
										  4*input_size,
										  1);

	this->node_weight = 0.0;

	this->action = action;

	for (int s_index = 0; s_index < (int)available_state.size(); s_index++) {
		this->state_network_inputs_state_indexes.push_back(available_state);
		Network* new_state_network = new Network(input_size,
												 4*input_size,
												 1);
		this->state_networks.push_back(new_state_network);
		this->state_networks_target_states.push_back(available_state[s_index]);
	}

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_score_network = NULL;
	this->explore_jump_certainty_network = NULL;
	this->explore_halt_score_network = NULL;
	this->explore_halt_certainty_network = NULL;
	this->explore_no_halt_score_network = NULL;
	this->explore_no_halt_certainty_network = NULL;

	this->node_is_on = false;
}

SolutionNodeAction::SolutionNodeAction(Solution* solution,
									   int node_index,
									   ifstream& save_file) {
	this->solution = solution;

	this->node_index = node_index;
	this->node_type = NODE_TYPE_ACTION;

	load_score_network(save_file);

	Action action(save_file);
	this->action = action;

	string num_state_networks_line;
	getline(save_file, num_state_networks_line);
	int num_state_networks = stoi(num_state_networks_line);
	for (int s_index = 0; s_index < num_state_networks; s_index++) {
		string num_inputs_line;
		getline(save_file, num_inputs_line);
		int num_inputs = stoi(num_inputs_line);
		vector<int> input_state_indexes;
		for (int si_index = 0; si_index < num_inputs; si_index++) {
			string state_index_line;
			getline(save_file, state_index_line);
			int state_index = stoi(state_index_line);
			input_state_indexes.push_back(state_index);
		}
		this->state_network_inputs_state_indexes.push_back(input_state_indexes);

		string target_state_line;
		getline(save_file, target_state_line);
		int target_state = stoi(target_state_line);
		this->state_networks_target_states.push_back(target_state);

		string state_network_name = "../saves/nns/state_" + to_string(this->node_index) \
			+ "_" + to_string(s_index) + "_" + to_string(this->solution->id) + ".txt";
		ifstream state_network_save_file;
		state_network_save_file.open(state_network_name);
		Network* state_network = new Network(state_network_save_file);
		state_network_save_file.close();
		this->state_networks.push_back(state_network);
	}

	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	this->explore_jump_score_network = NULL;
	this->explore_jump_certainty_network = NULL;
	this->explore_halt_score_network = NULL;
	this->explore_halt_certainty_network = NULL;
	this->explore_no_halt_score_network = NULL;
	this->explore_no_halt_certainty_network = NULL;

	this->node_is_on = false;
}

SolutionNodeAction::~SolutionNodeAction() {
	delete this->score_network;
	delete this->certainty_network;

	for (int s_index = 0; s_index < (int)this->state_networks.size(); s_index++) {
		delete this->state_networks[s_index];
	}
}

void SolutionNodeAction::construct_fold_inputs(vector<int>& loop_scope_counts,
											   int& curr_index,
											   SolutionNode* explore_node) {
	FoldHelper* fold_helper;

	map<SolutionNode*, FoldHelper*>::iterator it = this->fold_helpers.find(explore_node);
	if (it == this->fold_helpers.end()) {
		fold_helper = new FoldHelper(loop_scope_counts.size());
		this->fold_helpers[explore_node] = fold_helper;
	} else {
		fold_helper = it->second;
	}

	fold_helper.set_index(loop_scope_counts,
						  curr_index);

	curr_index++;

	if (this == explore_node) {
		if (loop_scope_counts.back() == 6) {
			return;
		} else {
			loop_scope_counts.back()++;
			this->explore_start_inclusive->construct_fold_inputs(
				loop_scope_counts,
				curr_index,
				explore_node);
		}
	} else {
		this->next->construct_fold_inputs(loop_scope_counts,
										  curr_index,
										  explore_node);
	}
}

SolutionNode* SolutionNodeAction::explore(Problem& problem,
										  vector<vector<double>>& state_vals,
										  vector<SolutionNode*>& scopes,
										  vector<int>& scope_states,
										  vector<int>& scope_locations,
										  IterExplore& iter_explore,
										  vector<StepHistory>& instance_history,
										  vector<AbstractNetworkHistory*>& network_historys) {
	if (iter_explore->explore_node == this
			&& scope_stacks.back() == NULL) {
		// back from explore
		scope_stacks.pop_back();
		scope_stack_counts.pop_back();
		// no loops for now, so always pop_back

		if (this->explore_state = EXPLORE_STATE_EXPLORE) {
			// do nothing
		} else if (this->explore_state = EXPLORE_STATE_LEARN_JUMP_FLAT) {
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == this) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_new_path_flat_size] = {};
			bool activated[this->explore_new_path_flat_size] = {};

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_non_inclusive_index+1; input_start_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					// run state network during LEARN_JUMP_FOLD but do nothing here
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = instance_history[n_index]->node_visited->fold_helpers[this] \
						->get_input_index(fold_loop_scope_counts);
					flat_inputs[input_index] = instance_history[n_index]->previous_observations;
					activated[input_index] = true;
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
				this->explore_state_networks[s_index]->mtx.lock();
				this->explore_state_networks->activate(flat_inputs,
													   activated,
													   state_vals[l_index],
													   obs,
													   network_historys);
				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_vals[l_index][s_index] = this->explore_state_networks->output->acti_vals[s_index];
				}
				this->explore_state_networks[s_index]->mtx.unlock();
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FLAT) {
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == this) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_new_path_flat_size] = {};
			bool activated[this->explore_new_path_flat_size] = {};

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_non_inclusive_index+1; input_start_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					// run state network during LEARN_JUMP_FOLD but do nothing here
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = instance_history[n_index]->node_visited->fold_helpers[this] \
						->get_input_index(fold_loop_scope_counts);
					flat_inputs[input_index] = instance_history[n_index]->previous_observations;
					activated[input_index] = true;
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
				this->explore_state_networks[s_index]->mtx.lock();
				this->explore_state_networks->activate(flat_inputs,
													   activated,
													   state_vals[l_index],
													   obs);
				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_vals[l_index][s_index] = this->explore_state_networks->output->acti_vals[s_index];
				}
				this->explore_state_networks[s_index]->mtx.unlock();
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FOLD) {
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == this) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_new_path_flat_size] = {};
			bool activated[this->explore_new_path_flat_size] = {};

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_non_inclusive_index+1; input_start_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					// also assign SolutionNodeEmpty* fold_indexes, but don't increment
					instance_history[n_index]->node_visited->fold_helpers[this]->new_path_process(
						fold_loop_scope_counts,
						this->explore_new_path_fold_input_index_on,
						state_vals,
						network_historys);
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					instance_history[n_index]->node_visited->fold_helpers[this]->new_path_process(
						fold_loop_scope_counts,
						this->explore_new_path_fold_input_index_on,
						instance_history[n_index]->previous_observations,
						flat_inputs,
						activated,
						state_vals,
						network_historys);
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
				this->explore_state_networks[s_index]->mtx.lock();
				this->explore_state_networks->activate(flat_inputs,
													   activated,
													   state_vals[l_index],
													   obs,
													   network_historys);
				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_vals[l_index][s_index] = this->explore_state_networks->output->acti_vals[s_index];
				}
				this->explore_state_networks[s_index]->mtx.unlock();
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FOLD) {
			// do nothing
		}

		instance_history.push_back(StepHistory(this,
											   false,
											   0.0,
											   -1,
											   true));
		return get_jump_end(this);
	}

	bool is_first_explore = false;
	if (iter_explore == NULL) {
		if (randuni() < this->if_explore_weight) {
			if (this->explore_state == EXPLORE_STATE_EXPLORE) {
				int parent_jump_scope_start_non_inclusive_index;
				int parent_jump_end_non_inclusive_index;
				new_random_scope(this,
								 parent_jump_scope_start_non_inclusive_index,
								 parent_jump_end_non_inclusive_index);

				vector<SolutionNode*> explore_path;
				if (parent_jump_end_non_inclusive_index
						== this->scope_node_index + 1) {
					new_random_path(explore_path,
									false);
				} else {
					new_random_path(explore_path,
									true);
				}
				explore_path[explore_path.size()-1]->next = this;

				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_EXPLORE);
				iter_explore->explore_path = explore_path;
				iter_explore->parent_jump_scope_start_non_inclusive_index = parent_jump_scope_start_non_inclusive_index;
				iter_explore->parent_jump_end_non_inclusive_index = parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
				// go back and rerun local state historys

				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_FLAT);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_scope_start_non_inclusive_index = this->parent_jump_scope_start_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FLAT) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_MEASURE_FLAT);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_scope_start_non_inclusive_index = this->parent_jump_scope_start_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FOLD) {
				// go back and rerun local state historys

				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_FOLD);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_scope_start_non_inclusive_index = this->parent_jump_scope_start_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FOLD) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_MEASURE_FOLD);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_scope_start_non_inclusive_index = this->parent_jump_scope_start_non_inclusive_index;
			}

			is_first_explore = true;
		}
	}

	if (iter_explore == NULL) {
		activate_state_networks(problem,
								state_vals,
								false,
								network_historys);
	} else if (iter_explore->type == ITER_EXPLORE_TYPE_EXPLORE) {
		activate_state_networks(problem,
								state_vals,
								false,
								network_historys);
	} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FLAT) {
		for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
			bool should_backprop = false;
			if (s_index < iter_explore->scopes.size()) {
				if (iter_explore->scopes[s_index] == scopes[s_index]) {
					// no loops for now, so always check equality
					if (iter_explore->scope_states[s_index] == scopes[s_index]) {
						if (s_index == iter_explore->scopes.size()) {
							if (iter_explore->parent_jump_scope_start_non_inclusive_index
									< scope_locations[s_index]) {
								should_backprop = true;
							}
						} else {
							if (iter_explore->scope_location[s_index] < scope_locations[s_index]) {
								should_backprop = true;
							}
						}
					}
				}
			}
			// TODO: reexamine


		}
	}
}

SolutionNode* SolutionNodeAction::activate(Problem& problem,
										   double* state_vals,
										   bool* states_on,
										   vector<SolutionNode*>& loop_scopes,
										   vector<int>& loop_scope_counts,
										   vector<bool>& loop_decisions,
										   int& iter_explore_type,
										   SolutionNode*& iter_explore_node,
										   IterExplore*& iter_explore,
										   double* potential_state_vals,
										   vector<int>& potential_state_indexes,
										   vector<NetworkHistory*>& network_historys,
										   vector<vector<double>>& guesses,
										   vector<int>& explore_decisions,
										   bool save_for_display,
										   ofstream& display_file) {
	if (save_for_display) {
		display_file << this->node_index << endl;
		this->action.save(display_file);
	}

	bool is_first_explore = false;
	if (iter_explore_type == EXPLORE_TYPE_NONE) {
		if (randuni() < this->node_weight) {
			if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
				vector<int> available_state;
				for (int s_index = 0; s_index < this->solution->current_state_counter; s_index++) {
					if (states_on[s_index]) {
						available_state.push_back(s_index);
					}
				}

				int rand_index = rand()%3;
				if (rand_index == 0) {
					SolutionNode* inclusive_end;
					SolutionNode* non_inclusive_end;
					find_scope_end(this, inclusive_end, non_inclusive_end);
					
					geometric_distribution<int> seq_length_dist(0.2);
					normal_distribution<double> write_val_dist(0.0, 2.0);
					vector<Action> try_path;
					if (this == inclusive_end) {
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
							this->next,
							available_state,
							-1);
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
							this->next,
							inclusive_end,
							non_inclusive_end,
							available_state,
							-1);
					}
				} else if (rand_index == 1) {
					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					find_potential_jumps(this,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);
					int random_index = rand()%(int)potential_inclusive_jump_ends.size();

					geometric_distribution<int> seq_length_dist(0.2);
					normal_distribution<double> write_val_dist(0.0, 2.0);
					vector<Action> try_path;
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
							this->next,
							available_state,
							-1);
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
							this->next,
							potential_inclusive_jump_ends[random_index],
							potential_non_inclusive_jump_ends[random_index],
							available_state,
							-1);
					}
				} else {
					vector<SolutionNode*> potential_non_inclusive_loop_starts;
					vector<SolutionNode*> potential_inclusive_loop_starts;
					find_potential_loops(this,
										 potential_non_inclusive_loop_starts,
										 potential_inclusive_loop_starts);
					int random_index = rand()%(int)potential_non_inclusive_loop_starts.size();

					vector<Action> empty_try_path;
					iter_explore = new IterExplore(
						ITER_EXPLORE_TYPE_LOOP,
						empty_try_path,
						potential_non_inclusive_loop_starts[random_index],
						potential_inclusive_loop_starts[random_index],
						this,
						this->next,
						available_state,
						-1);
				}

				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_EXPLORE;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_JUMP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_JUMP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
				potential_state_indexes = this->explore_loop_states;
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_LOOP;
			} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP) {
				potential_state_indexes = this->explore_loop_states;
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE_LOOP;
			}

			is_first_explore = true;
		}
	}

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								true,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								true,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		activate_state_networks(problem,
								state_vals,
								states_on,
								false,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		activate_state_networks_with_potential(problem,
											   state_vals,
											   states_on,
											   potential_state_vals,
											   potential_state_indexes,
											   true,
											   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		activate_state_networks_with_potential(problem,
											   state_vals,
											   states_on,
											   potential_state_vals,
											   potential_state_indexes,
											   false,
											   network_historys);
	}

	problem.perform_action(this->action);

	activate_helper(problem,
					state_vals,
					states_on,
					iter_explore_type,
					iter_explore_node,
					potential_state_vals,
					potential_state_indexes,
					network_historys,
					guesses);

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
										guesses,
										explore_decisions);
	}

	if (explore_node != NULL) {
		return explore_node;
	} else {
		return this->next;
	}
}

void SolutionNodeAction::backprop(double score,
								  double misguess,
								  double* state_errors,
								  bool* states_on,
								  vector<bool>& loop_decisions,
								  int& iter_explore_type,
								  SolutionNode*& iter_explore_node,
								  double* potential_state_errors,
								  vector<int>& potential_state_indexes,
								  vector<NetworkHistory*>& network_historys,
								  vector<int>& explore_decisions) {
	explore_backprop(score,
					 misguess,
					 state_errors,
					 states_on,
					 iter_explore_node,
					 potential_state_errors,
					 network_historys,
					 explore_decisions);

	backprop_helper(score,
					misguess,
					state_errors,
					states_on,
					iter_explore_type,
					iter_explore_node,
					potential_state_errors,
					network_historys);

	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_state_networks(state_errors,
								states_on,
								network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		if (this->node_index == -1) {
			backprop_state_networks(state_errors,
									states_on,
									network_historys);
		} else {
			backprop_state_networks_errors_with_no_weight_change(
				state_errors,
				states_on,
				network_historys);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		backprop_state_networks_with_potential(potential_state_errors,
											   potential_state_indexes,
											   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		// do nothing
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

void SolutionNodeAction::activate_state_networks_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		vector<int>& potential_state_indexes,
		bool backprop,
		vector<NetworkHistory*>& network_historys) {
	activate_state_networks(problem,
							state_vals,
							states_on,
							false,
							network_historys);

	for (int p_index = 0; p_index < (int)this->potential_state_networks_target_states.size(); p_index++) {
		for (int i_index = 0; i_index < 2; i_index++) {
			if (potential_state_indexes[i_index] == this->potential_state_networks_target_states[p_index]) {
				vector<double> state_network_inputs;
				double curr_observations = problem.get_observation();
				state_network_inputs.push_back(curr_observations);
				for (int i_index = 0; i_index < (int)this->potential_inputs_state_indexes[p_index].size(); i_index++) {
					if (states_on[this->potential_inputs_state_indexes[p_index][i_index]]) {
						state_network_inputs.push_back(state_vals[this->potential_inputs_state_indexes[p_index][i_index]]);
					} else {
						state_network_inputs.push_back(0.0);
					}
				}
				for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes[p_index].size(); pi_index++) {
					state_network_inputs.push_back(potential_state_vals[pi_index]);
				}

				if (backprop) {
					this->potential_state_networks[p_index]->mtx.lock();
					this->potential_state_networks[p_index]->activate(state_network_inputs, network_historys);
					potential_state_vals[i_index] = this->potential_state_networks[p_index]->output->acti_vals[0];
					this->potential_state_networks[p_index]->mtx.unlock();
				} else {
					this->potential_state_networks[p_index]->mtx.lock();
					this->potential_state_networks[p_index]->activate(state_network_inputs);
					potential_state_vals[i_index] = this->potential_state_networks[p_index]->output->acti_vals[0];
					this->potential_state_networks[p_index]->mtx.unlock();
				}
			}
		}
	}
}

void SolutionNodeAction::backprop_state_networks_with_potential(
		double* potential_state_errors,
		vector<int>& potential_state_indexes,
		vector<NetworkHistory*>& network_historys) {
	for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
		for (int i_index = 0; i_index < 2; i_index++) {
			if (potential_state_indexes[i_index] == this->potential_state_networks_target_states[p_index]) {
				NetworkHistory* network_history = network_historys.back();

				this->potential_state_networks[p_index]->mtx.lock();

				network_history->reset_weights();

				vector<double> state_network_errors;
				state_network_errors.push_back(potential_state_errors[
					this->potential_state_networks_target_states[p_index]]);
				this->potential_state_networks[p_index]->backprop(state_network_errors);

				for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes[p_index].size(); pi_index++) {
					if (this->potential_potential_inputs_state_indexes[p_index][pi_index]
							== this->potential_state_networks_target_states[p_index]) {
						potential_state_errors[pi_index] = this->potential_state_networks[p_index]->input->errors[
							1 + this->potential_inputs_state_indexes[p_index].size() + pi_index];
					} else {
						potential_state_errors[pi_index] += this->potential_state_networks[p_index]->input->errors[
							1 + this->potential_inputs_state_indexes[p_index].size() + pi_index];
					}

					this->potential_state_networks[p_index]->input->errors[
						1 + this->potential_inputs_state_indexes[p_index].size() + pi_index] = 0.0;
				}

				this->potential_state_networks[p_index]->mtx.unlock();

				delete network_history;
				network_historys.pop_back();
			}
		}
	}
}
