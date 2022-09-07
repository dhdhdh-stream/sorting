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
		// back from explore
		scopes.pop_back();
		scope_states.pop_back();
		scope_locations.pop_back();
		// no loops for now, so always pop_back

		if (this->explore_state = EXPLORE_STATE_EXPLORE) {
			// do nothing
		} else if (this->explore_state = EXPLORE_STATE_LEARN_FLAT) {
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
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					flat_inputs[input_index] = instance_history[n_index]->previous_observations;
					activated[input_index] = true;
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
				this->explore_state_networks[l_index]->mtx.lock();
				this->explore_state_networks->activate(flat_inputs,
													   activated,
													   state_vals[l_index],
													   obs,
													   network_historys);
				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_vals[l_index][s_index] = this->explore_state_networks->output->acti_vals[s_index];
				}
				this->explore_state_networks[l_index]->mtx.unlock();
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
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
			for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
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
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
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
					node_empty->fold_helpers[this]->new_path_process(
						fold_loop_scope_counts,
						this->explore_new_path_fold_input_index_on_inclusive,
						state_vals,
						network_historys);
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					node_action->fold_helpers[this]->new_path_process(
						fold_loop_scope_counts,
						this->explore_new_path_fold_input_index_on_inclusive,
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
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
			// train state networks in new path, but do nothing here
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_REPLACE) {
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
					node_empty->fold_helpers[this]->new_path_process(
						fold_loop_scope_counts,
						this->explore_new_path_fold_input_index_on_inclusive,
						state_vals,
						network_historys);
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					// n_index can't be explore_node, so action_performed always true
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					node_action->fold_helpers[this]->new_path_process(
						fold_loop_scope_counts,
						this->explore_new_path_fold_input_index_on_inclusive,
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
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
			// train state networks in new path, but do nothing here
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
			// do nothing
		}

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
		if (randuni() < this->if_explore_weight) {
			if (this->explore_state == EXPLORE_STATE_EXPLORE) {
				int parent_jump_scope_start_non_inclusive_index;
				int parent_jump_end_non_inclusive_index;
				new_random_scope(this,
								 parent_jump_scope_start_non_inclusive_index,
								 parent_jump_end_non_inclusive_index);

				vector<SolutionNode*> explore_path;
				if (parent_jump_end_non_inclusive_index == this->scope_node_index + 1) {
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
			} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_FLAT);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_MEASURE_FLAT);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_REPLACE) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
				iter_explore = new IterExplore(this,
											   ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE);
				iter_explore->scopes = scopes;
				iter_explore->scope_states = scope_states;
				iter_explore->scope_locations = scope_locations;
				iter_explore->parent_jump_end_non_inclusive_index = this->parent_jump_end_non_inclusive_index;
			}

			is_first_explore = true;
		}
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

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
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

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
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

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
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

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
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

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
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
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			if (is_first_explore || rand()%5 == 0) {
				scopes.push_back(NULL);
				scope_state.push_back(-1);
				scope_locations.push_back(-1);
				return iter_explore->explore_path[0];
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_existing_path_flat_size] = {};
			bool activated[this->explore_existing_path_flat_size] = {};

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					flat_inputs[input_index] = instance_history[n_index]->previous_observations;
					activated[input_index] = true;
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			if (rand()%2 == 0) {
				this->explore_jump_score_network->mtx.lock();
				this->explore_jump_score_network->activate(flat_inputs,
														   activated,
														   obs,
														   network_historys);
				this->explore_jump_score_network->mtx.unlock();

				scopes.push_back(NULL);
				scope_state.push_back(-1);
				scope_locations.push_back(-1);
				return this->explore_path[0];
			} else {
				this->explore_no_jump_score_network->mtx.lock();
				this->explore_no_jump_score_network->activate(flat_inputs,
															  activated,
															  obs,
															  network_historys);
				this->explore_no_jump_score_network->mtx.unlock();

				return this->next;
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_existing_path_flat_size] = {};
			bool activated[this->explore_existing_path_flat_size] = {};

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					flat_inputs[input_index] = instance_history[n_index]->previous_observations;
					activated[input_index] = true;
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			this->explore_jump_score_network->mtx.lock();
			this->explore_jump_score_network->activate(flat_inputs,
													   activated,
													   obs);
			double jump_score = this->explore_jump_score_network->output->acti_vals[0];
			this->explore_jump_score_network->mtx.unlock();

			this->explore_no_jump_score_network->mtx.lock();
			this->explore_no_jump_score_network->activate(flat_inputs,
														  activated,
														  obs);
			double no_jump_score = this->explore_no_jump_score_network->output->acti_vals[0];
			this->explore_no_jump_score_network->mtx.unlock();

			if (jump_score > no_jump_score) {
				if (rand()%2 == 0) {
					scopes.push_back(NULL);
					scope_state.push_back(-1);
					scope_locations.push_back(-1);
					instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_EXPLORE_EXPLORE;
					return this->explore_path[0];
				} else {
					instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_EXPLORE_NO_EXPLORE;
					return this->next;
				}
			} else {
				if (rand()%2 == 0) {
					scopes.push_back(NULL);
					scope_state.push_back(-1);
					scope_locations.push_back(-1);
					instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_EXPLORE;
					return this->explore_path[0];
				} else {
					instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_NO_EXPLORE;
					return this->next;
				}
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_existing_path_flat_size] = {};
			bool activated[this->explore_existing_path_flat_size] = {};
			vector<double> new_state_vals(this->explore_new_state_size, 0.0);

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_non_inclusive_index+1; input_start_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					// also assign SolutionNodeEmpty* fold_indexes, but don't increment
					ndoe_empty->fold_helpers[this]->existing_path_process(
						fold_loop_scope_counts,
						this->explore_existing_path_fold_input_index_on_inclusive,
						new_state_vals,
						network_historys);
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					node_action->fold_helpers[this]->existing_path_process(
						fold_loop_scope_counts,
						this->explore_existing_path_fold_input_index_on_inclusive,
						instance_history[n_index]->previous_observations,
						flat_inputs,
						activated,
						new_state_vals,
						network_historys);
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			if (rand()%2 == 0) {
				this->explore_jump_score_network->mtx.lock();
				this->explore_jump_score_network->activate(flat_inputs,
														   activated,
														   new_state_vals,
														   obs,
														   network_historys);
				this->explore_jump_score_network->mtx.unlock();

				scopes.push_back(NULL);
				scope_state.push_back(-1);
				scope_locations.push_back(-1);
				return this->explore_path[0];
			} else {
				this->explore_no_jump_score_network->mtx.lock();
				this->explore_no_jump_score_network->activate(flat_inputs,
															  activated,
															  new_state_vals,
															  obs,
															  network_historys);
				this->explore_no_jump_score_network->mtx.unlock();

				return this->next;
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			vector<double> new_state_vals(this->explore_new_state_size, 0.0);

			for (int n_index = input_start_non_inclusive_index+1; input_start_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					node_empty->fold_helpers[this]->activate_new_state(new_state_vals);
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					node_action->fold_helpers[this]->activate_new_state(new_state_vals);
				}
			}

			// reuse new_state_vals as network inputs
			new_state_vals.push_back(problem.get_observation());

			if (rand()%2 == 0) {
				this->explore_small_jump_score_network->mtx.lock();
				this->explore_small_jump_score_network->activate(new_state_vals, network_historys);
				this->explore_small_jump_score_network->mtx.unlock();

				scopes.push_back(NULL);
				scope_state.push_back(-1);
				scope_locations.push_back(-1);
				return this->explore_path[0];
			} else {
				this->explore_small_no_jump_score_network->mtx.lock();
				this->explore_small_no_jump_score_network->activate(new_state_vals, network_historys);
				this->explore_small_no_jump_score_network->mtx.unlock();

				return this->next;
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			int input_start_non_inclusive_index;
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					input_start_non_inclusive_index = n_index;
					break;
				}
			}

			vector<double> new_state_vals(this->explore_new_state_size, 0.0);

			for (int n_index = input_start_non_inclusive_index+1; input_start_index < (int)instance_history.size(); n_index++) {
				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					node_empty->fold_helpers[this]->activate_new_state(new_state_vals);
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					node_action->fold_helpers[this]->activate_new_state(new_state_vals);
				}
			}

			// reuse new_state_vals as network inputs
			new_state_vals.push_back(problem.get_observation());

			this->explore_small_jump_score_network->mtx.lock();
			this->explore_small_jump_score_network->activate(new_state_vals);
			double jump_score = this->explore_small_jump_score_network->output->acti_vals[0];
			this->explore_small_jump_score_network->mtx.unlock();

			this->explore_small_no_jump_score_network->mtx.lock();
			this->explore_small_no_jump_score_network->activate(new_state_vals);
			double no_jump_score = this->explore_small_no_jump_score_network->output->acti_vals[0];
			this->explore_small_no_jump_score_network->mtx.unlock();

			if (jump_score > no_jump_score) {
				if (rand()%2 == 0) {
					scopes.push_back(NULL);
					scope_state.push_back(-1);
					scope_locations.push_back(-1);
					instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FOLD_EXPLORE;
					return this->explore_path[0];
				} else {
					instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FOLD_NO_EXPLORE;
					return this->next;
				}
			} else {
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FOLD_N_A;
				return this->next;
			}
		} else if (this->explore_state == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			scopes.push_back(NULL);
			scope_state.push_back(-1);
			scope_locations.push_back(-1);
			return this->explore_path[0];
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
			scopes.push_back(NULL);
			scope_state.push_back(-1);
			scope_locations.push_back(-1);
			return this->explore_path[0];
		} else if (this->explore_state == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			scopes.push_back(NULL);
			scope_state.push_back(-1);
			scope_locations.push_back(-1);
			return this->explore_path[0];
		}
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
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			// do nothing
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				this->explore_state_networks[s_index]->mtx.lock();

				network_historys.back()->reset_weights();

				this->explore_state_networks[l_index]->backprop(state_errors[l_index]);

				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_errors[l_index][s_index] = this->explore_state_networks[l_index]->state_input->acti_vals[s_index];
					this->explore_state_networks[l_index]->state_input->acti_vals[s_index] = 0.0;
				}

				this->explore_state_networks[s_index]->mtx.unlock();
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
			// do nothing
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				this->explore_state_networks[s_index]->mtx.lock();

				network_historys.back()->reset_weights();

				this->explore_state_networks[l_index]->backprop(state_errors[l_index]);

				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_errors[l_index][s_index] = this->explore_state_networks[l_index]->state_input->acti_vals[s_index];
					this->explore_state_networks[l_index]->state_input->acti_vals[s_index] = 0.0;
				}

				this->explore_state_networks[s_index]->mtx.unlock();
			}

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			// start from before this step_history
			for (int n_index = (int)instance_history.size()-2; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == this) {
					break;
				}

				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					int input_index = node_empty->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
						node_empty->new_path_backprop_state_networks(state_errors);
					} else {
						break;
					}
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
						node_action->new_path_backprop_state_networks(state_errors);
					} else {
						break;
					}
				}
				// TODO: when loops are added, update fold_loop_scope_counts
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
			// train state networks in new path, but do nothing here
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (this->explore_state == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				this->explore_state_networks[s_index]->mtx.lock();

				network_historys.back()->reset_weights();

				this->explore_state_networks[l_index]->backprop(state_errors[l_index]);

				for (int s_index = 0; s_index < this->local_state[l_index]; s_index++) {
					state_errors[l_index][s_index] = this->explore_state_networks[l_index]->state_input->acti_vals[s_index];
					this->explore_state_networks[l_index]->state_input->acti_vals[s_index] = 0.0;
				}

				this->explore_state_networks[s_index]->mtx.unlock();
			}

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			// start from before this step_history
			for (int n_index = (int)instance_history.size()-2; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == this) {
					break;
				}

				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					int input_index = node_empty->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
						node_empty->new_path_backprop_state_networks(state_errors);
					} else {
						break;
					}
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
						node_action->new_path_backprop_state_networks(state_errors);
					} else {
						break;
					}
				}
				// TODO: when loops are added, update fold_loop_scope_counts
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
			// train state networks in new path, but do nothing here
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
			// do nothing
		}

		instance_history.pop_back();
		return;
	}

	if (iter_explore != NULL
			&& iter_explore->explore_node == this) {
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			// do nothing
		} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
			if (network_historys.back()->network == this->explore_jump_score_network) {
				// backprop explore_jump_score_network
			} else {
				// backprop explore_no_jump_score_network
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FLAT) {
			if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_EXPLORE_EXPLORE) {
				this->explore_explore_explore_score += score;
				this->explore_explore_explore_count++;
			} else if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_EXPLORE_NO_EXPLORE) {
				this->explore_explore_no_explore_score += score;
				this->explore_explore_no_explore_count++;
			} else if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_EXPLORE) {
				this->explore_no_explore_explore_score += score;
				this->explore_no_explore_explore_count++;
			} else {
				// instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_NO_EXPLORE
				this->explore_no_explore_no_explore_score += score;
				this->explore_no_explore_no_explore_count++;
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FOLD_BRANCH) {
			vector<double> new_state_errors(this->explore_new_state_size, 0.0);
			if (network_historys.back()->network == this->explore_jump_score_network) {
				// backprop explore_jump_score_network
			} else {
				// backprop explore_no_jump_score_network
			}

			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					break;
				}

				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					int input_index = node_empty->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_existing_path_fold_input_index_on_inclusive) {
						node_empty->fold_helpers[this]->existing_path_backprop_new_state(new_state_errors);
					} else {
						break;
					}
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_existing_path_fold_input_index_on_inclusive) {
						node_action->fold_helpers[this]->existing_path_backprop_new_state(new_state_errors);
					} else {
						break;
					}
				}
				// TODO: when loops are added, update fold_loop_scope_counts
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
			vector<double> new_state_errors(this->explore_new_state_size, 0.0);
			if (network_historys.back()->network == this->explore_small_jump_score_network) {
				// backprop explore_small_jump_score_network
			} else {
				// backprop explore_small_no_jump_score_network
			}

			SolutionNode* jump_scope_start = get_jump_scope_start(this);
			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
				if (instance_history[n_index]->node_visited == jump_scope_start) {
					break;
				}

				if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_EMPTY) {
					SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index]->node_visited;
					int input_index = node_empty->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_existing_path_fold_input_index_on_inclusive) {
						node_empty->fold_helpers[this]->existing_path_backprop_new_state(new_state_errors);
					} else {
						break;
					}
				} else if (instance_history[n_index]->node_visited->node_type == NODE_TYPE_ACTION) {
					SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index]->node_visited;
					int input_index = node_action->fold_helpers[this]->get_input_index(fold_loop_scope_counts);
					if (input_index <= this->explore_existing_path_fold_input_index_on_inclusive) {
						node_action->fold_helpers[this]->existing_path_backprop_new_state(new_state_errors);
					} else {
						break;
					}
				}
				// TODO: when loops are added, update fold_loop_scope_counts
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
			if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FOLD_EXPLORE) {
				this->explore_fold_explore_score += score;
				this->explore_fold_explore_count++;
			} else if (instance_history.back().explore_decision == EXPLORE_DECISION_TYPE_FOLD_NO_EXPLORE) {
				this->explore_fold_no_explore_score += score;
				this->explore_fold_no_explore_count++;
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_REPLACE) {
			// do nothing
		} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
			// do nothing
		} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
			this->explore_fold_replace_score += score;
		}
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
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FLAT) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_BRANCH) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_BRANCH) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_BRANCH) {
			// do nothing
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_LEARN_SMALL_REPLACE) {
			for (int l_index = (int)this->local_state.size()-1; l_index >= 0; l_index--) {
				if (network_historys.back()->network == this->state_networks[l_index]) {
					// backprop state_networks[l_index]
				}
			}
		} else if (iter_explore->type == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE) {
			// do nothing
		}
	}
}

void SolutionNodeAction::explore_increment(double score,
										   IterExplore*& iter_explore) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		if (score == 1.0) {
			this->explore_path = iter_explore->explore_path;
			for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
				if (this->explore_path[n_index]->node_type != NODE_TYPE_ACTION
						&& this->explore_path[n_index]->node_type != NODE_TYPE_EMPTY) {
					SolutionNode* deep_copy = this->explore_path[n_index]->deep_copy(0);
					this->explore_path[n_index] = deep_copy;
				}
			}

			this->parent_jump_scope_start_non_inclusive_index = iter_explore->parent_jump_scope_start_non_inclusive_index;
			this->parent_jump_end_non_inclusive_index = iter_explore->parent_jump_end_non_inclusive_index;

			vector<SolutionNode*> existing_path;
			get_existing_path(this, existing_path);
			vector<int> existing_path_loop_scope_counts;
			existing_path_loop_scope_counts.push_back(1);
			int existing_path_curr_input_index = 0;
			for (int n_index = 0; n_index < (int)existing_path.size(); n_index++) {
				existing_path[n_index]->setup_flat(existing_path_loop_scope_counts,
												   existing_path_curr_input_index,
												   this);
			}
			this->explore_existing_path_flat_size = existing_path_curr_input_index;

			vector<int> new_path_loop_scope_counts;
			new_path_loop_scope_counts.push_back(1);
			int new_path_curr_input_index = 0;
			for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
				this->explore_path[n_index]->setup_flat(new_path_loop_scope_counts,
														new_path_curr_input_index,
														this);
			}
			this->explore_new_path_flat_size = new_path_curr_input_index;

			for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
				this->explore_state_networks.push_back(
					new FoldNetwork(this->explore_new_path_flat_size,
									this->local_state[l_index]));
			}

			this->explore_jump_score_network = new FoldNetwork(this->explore_existing_path_flat_size);
			this->explore_no_jump_score_network = new FoldNetwork(this->explore_existing_path_flat_size);

			this->explore_state = EXPLORE_STATE_LEARN_FLAT;
			this->explore_iter_index = 0;
		} else {
			for (int n_index = 0; n_index < (int)iter_explore->explore_path.size(); n_index++) {
				// non-recursive, only need to check top layer
				if (this->explore_path[n_index]->node_type == NODE_TYPE_ACTION
						|| this->explore_path[n_index]->node_type == NODE_TYPE_EMPTY) {
					delete this->explore_path[n_index];
				}
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 2000000) {
			this->explore_state = EXPLORE_STATE_MEASURE_FLAT;
			this->explore_iter_index = 0;

			this->explore_explore_explore_count = 0;
			this->explore_explore_explore_score = 0.0;
			this->explore_explore_no_explore_count = 0;
			this->explore_explore_no_explore_score = 0.0;
			this->explore_no_explore_explore_count = 0;
			this->explore_no_explore_explore_score = 0.0;
			this->explore_no_explore_no_explore_count = 0;
			this->explore_no_explore_no_explore_score = 0.0;
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 100000) {
			double explore_explore_score = this->explore_explore_explore_score/this->explore_explore_explore_count;
			double explore_no_explore_score = this->explore_explore_no_explore_score/this->explore_explore_no_explore_count;

			bool branch_better = false;
			bool branch_not_worse = false;
			if (explore_explore_score > (explore_no_explore_score + 0.03*(1.0 - solution->average_score))) {
				branch_better = true;
				branch_not_worse = true;
			} else if (explore_explore_score > 0.97*explore_no_explore_score) {
				branch_not_worse = true;
			}

			double no_explore_explore_score = this->explore_no_explore_explore_score/this->explore_no_explore_explore_count;
			double no_explore_no_explore_score = this->explore_no_explore_no_explore_score/this->explore_no_explore_no_explore_count;

			bool can_replace = false;
			if (no_explore_explore_score > 0.97*no_explore_no_explore_score) {
				can_replace = true;
			}

			if (branch_better) {
				if (can_replace) {
					// replace
					this->explore_replace_type = EXPLORE_REPLACE_TYPE_SCORE;
					this->explore_replace_info_gain = 0.0;

					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						this->explore_path[n_index]->initialize_local_scope(this->local_state);
					}

					this->explore_state = EXPLORE_STATE_LEARN_FOLD_REPLACE;
					this->explore_iter_index = 0;

					this->explore_new_path_fold_input_index_on_inclusive = 0;
				} else {
					// branch
					this->explore_new_state_size = 2;	// TODO: make not hardcoded

					vector<SolutionNode*> existing_path;
					get_existing_path(this, existing_path);
					for (int n_index = 0; n_index < (int)existing_path.size(); n_index++) {
						existing_path[n_index]->setup_new_state(this, this->explore_new_state_size);
					}
					this->explore_jump_score_network->set_state_size(this->explore_new_state_size);
					this->explore_no_jump_score_network->set_state_size(this->explore_new_state_size);

					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						this->explore_path[n_index]->initialize_local_scope(this->local_state);
					}

					this->explore_state = EXPLORE_STATE_LEARN_FOLD_BRANCH;
					this->explore_iter_index = 0;

					this->explore_existing_path_fold_input_index_on_inclusive = 0;
					this->explore_new_path_fold_input_index_on_inclusive = 0;
				}
			} else if (branch_not_worse && can_replace) {
				vector<SolutionNode*> replacement_path;
				replacement_path.push_back(this);	// always include self for something to compare against
				get_replacement_path(this, replacement_path);

				double min_replacement_path_misguess = numeric_limits<double>::max();
				for (int n_index = 0; n_index < (int)replacement_path.size(); n_index++) {
					double curr_misguess = replacement_path[n_index]->get_min_misguess();
					if (curr_misguess < min_replacement_path_misguess) {
						min_replacement_path_misguess = curr_misguess;
					}
				}

				double min_new_path_misguess = numeric_limits<double>::max();
				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					double curr_misguess = this->explore_path[n_index]->get_min_misguess();
					if (curr_misguess < min_new_path_misguess) {
						min_new_path_misguess = curr_misguess;
					}
				}

				if (min_new_path_misguess < 0.9*min_replacement_path_misguess) {
					// replace
					this->explore_replace_type = EXPLORE_REPLACE_TYPE_INFO;
					this->explore_replace_info_gain = 1.0 - min_new_path_misguess/min_replacement_path_misguess;

					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						this->explore_path[n_index]->initialize_local_scope(this->local_state);
					}

					this->explore_state = EXPLORE_STATE_LEARN_FOLD_REPLACE;
					this->explore_iter_index = 0;

					this->explore_new_path_fold_input_index_on_inclusive = 0;
				} else {
					// abandon
					// cleanup explore

					this->explore_state = EXPLORE_STATE_EXPLORE;
					this->explore_iter_index = 0;
				}
			} else {
				// abandon
				vector<SolutionNode*> existing_path;
				get_existing_path(this, existing_path);
				for (int n_index = 0; n_index < (int)existing_path.size(); n_index++) {
					existing_path[n_index]->cleanup_explore(this);
				}

				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->cleanup_explore(this);
					delete this->explore_path[n_index];
				}
				this->explore_path.clear();

				for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
					delete this->explore_state_networks[l_index];
				}
				this->explore_state_networks.clear();

				delete this->explore_jump_score_network;
				delete this->explore_no_jump_score_network;

				this->explore_state = EXPLORE_STATE_EXPLORE;
				this->explore_iter_index = 0;
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 300000) {
			if (this->explore_existing_path_fold_input_index_on_inclusive < this->explore_existing_path_flat_size-1
					|| this->explore_new_path_fold_input_index_on_inclusive < this->explore_new_path_flat_size-1) {
				this->explore_existing_path_fold_input_index_on_inclusive++;
				this->explore_new_path_fold_input_index_on_inclusive++;
				this->explore_iter_index = 0;
			} else {
				// TODO: think about abandoning here
				this->explore_small_jump_score_network = new Network(this->explore_new_state_size,
																	 4*this->explore_new_state_size,
																	 1);
				this->explore_small_no_jump_score_network = new Network(this->explore_new_state_size,
																		4*this->explore_new_state_size,
																		1);

				this->explore_state = EXPLORE_STATE_LEARN_SMALL_BRANCH;
				this->explore_iter_index = 0;
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 400000) {
			this->explore_state = EXPLORE_STATE_MEASURE_FOLD_BRANCH;
			this->explore_iter_index = 0;

			this->explore_fold_explore_count = 0;
			this->explore_fold_explore_score = 0.0;
			this->explore_fold_no_explore_count = 0;
			this->explore_fold_no_explore_score = 0.0;
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 100000) {
			double explore_score = this->explore_fold_explore_score/this->explore_fold_explore_count;
			double no_explore_score = this->explore_fold_no_explore_score/this->explore_fold_no_explore_count;

			bool branch_better = false;
			if (explore_score > (no_explore_score + 0.03*(1.0 - solution->average_score))) {
				branch_better = true;
			}

			if (branch_better) {
				double branch_percent = (this->explore_fold_explore_count+this->explore_fold_no_explore_count)/100000;
				double score_increase = explore_score - no_explore_score;

				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->cleanup_explore(this);
				}

				CandidateBranch* new_candidate = new CandidateBranch(this,
																	 branch_percent,
																	 score_increase,
																	 this->parent_jump_scope_start_non_inclusive_index,
																	 this->parent_jump_end_non_inclusive_index,
																	 this->explore_path,
																	 this->explore_small_jump_score_network,
																	 this->explore_small_no_jump_score_network);
				vector<SolutionNode*> existing_path;
				get_existing_path(this, existing_path);
				for (int n_index = 0; n_index < (int)existing_path.size(); n_index++) {
					existing_path[n_index]->collect_new_state_networks(this,
																	   new_candidate->existing_nodes,
																	   new_candidate->new_state_networks);
				}
				solution->candidates.push_back(new_candidate);

				this->explore_path.clear();

				for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
					delete this->explore_state_networks[l_index];
				}
				this->explore_state_networks.clear();

				delete this->explore_jump_score_network;
				this->explore_jump_score_network = NULL;
				delete this->explore_no_jump_score_network;
				this->explore_no_jump_score_network = NULL;

				this->explore_small_jump_score_network = NULL;
				this->explore_small_no_jump_score_network = NULL;
			} else {
				// cleanup
			}

			this->explore_state = EXPLORE_STATE_EXPLORE;
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_REPLACE) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 300000) {
			if (this->explore_new_path_fold_input_index_on_inclusive < this->explore_new_path_flat_size-1) {
				this->explore_new_path_fold_input_index_on_inclusive++;
				this->explore_iter_index = 0;
			} else {
				// TODO: think about abandoning here
				this->explore_state = EXPLORE_STATE_LEARN_SMALL_REPLACE;
				this->explore_iter_index = 0;
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 400000) {
			this->explore_state = EXPLORE_STATE_MEASURE_FOLD_REPLACE;
			this->explore_iter_index = 0;

			this->explore_fold_replace_score = 0.0;
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 100000) {
			double original_score = (this->explore_explore_no_explore_score+this->explore_no_explore_no_explore_score)
				/(this->explore_explore_no_explore_count+this->explore_no_explore_no_explore_count);
			double replace_score = this->explore_fold_replace_score/100000;

			bool should_replace = false;
			if (this->explore_replace_type == EXPLORE_REPLACE_TYPE_SCORE) {
				if (replace_score > (original_score + 0.03*(1.0 - solution->average_score))) {
					should_replace = true;
				}
			} else {
				// this->explore_replace_type == EXPLORE_REPLACE_TYPE_INFO
				if (replace_score > 0.97*original_score) {
					should_replace = true;
				}
			}

			if (should_replace) {
				double score_increase = replace_score - original_score;

				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->cleanup_explore(this);
				}

				CandidateReplace* new_candidate = new CandidateReplace(this,
																	   this->explore_replace_type,
																	   score_increase,
																	   this->explore_replace_info_gain,
																	   this->parent_jump_scope_start_non_inclusive_index,
																	   this->parent_jump_end_non_inclusive_index,
																	   this->explore_path);

				this->explore_path.clear();

				vector<SolutionNode*> existing_path;
				get_existing_path(this, existing_path);
				for (int n_index = 0; n_index < (int)existing_path.size(); n_index++) {
					existing_path[n_index]->cleanup_explore(this);
				}

				for (int l_index = 0; l_index < (int)this->local_state.size(); l_index++) {
					delete this->explore_state_networks[l_index];
				}
				this->explore_state_networks.clear();

				delete this->explore_jump_score_network;
				this->explore_jump_score_network = NULL;
				delete this->explore_no_jump_score_network;
				this->explore_no_jump_score_network = NULL;
			} else {
				// cleanup
			}

			this->explore_state = EXPLORE_STATE_EXPLORE;
		}
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
