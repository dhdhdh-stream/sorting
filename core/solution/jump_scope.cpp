#include "jump_scope.h"

using namespace std;

JumpScope::JumpScope() {

}

JumpScope::~JumpScope() {

}

SolutionNode* JumpScope::explore(Problem& problem,
								 vector<vector<double>>& state_vals,
								 vector<SolutionNode*>& scope_stacks,
								 vector<int>& scope_stack_counts,
								 vector<int>& scope_history,	// build up history for backprop
								 IterExplore* iter_explore,
								 vector<SolutionNode*>& nodes_visited,
								 vector<double>& observations,
								 vector<AbstractNetworkHistory*>& network_historys) {
	if (iter_explore->explore_node == this
			&& scope_stacks.back() == NULL) {
		// back from explore
		if (iter_explore->local_type == ITER_EXPLORE_LOCAL_TYPE_JUMP_IF) {
			if (this->if_explore_state == EXPLORE_STATE_EXPLORE) {
				if (iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND) {
					scope_stacks.pop_back();
					scope_stack_counts.pop_back();

					scope_stacks.pop_back();
					scope_stack_counts.pop_back();

					observations.push_back(0.0);	// NULL
					scope_history.push_back((int)this->children_nodes.size());
					return this->next;
				} else {
					// iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START
					scope_stacks.pop_back();
					scope_stack_counts.pop_back();

					if (iter_explore->jump_end_non_inclusive_index
							>= this->children_nodes[iter_explore->child_index].size()) {
						scope_stacks.pop_back();
						scope_stack_counts.pop_back();

						observations.push_back(0.0);	// NULL
						scope_history.push_back((int)this->children_nodes.size());
						return this->next;
					} else {
						observations.push_back(0.0);	// NULL
						scope_history.push_back((int)this->children_nodes.size());
						return this->children_nodes[iter_explore->child_index][
							iter_explore->jump_end_non_inclusive_index];
					}
				}
			} else if (this->if_explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
				if (iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND) {
					scope_stacks.pop_back();
					scope_stack_counts.pop_back();

					int input_start_index;
					for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
						if (nodes_visited[n_index] == this) {
							input_start_index = n_index;
							break;
						}
					}

					vector<int> fold_loop_scope_counts;
					fold_loop_scope_counts.push_back(1);
					for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
						if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
							nodes_visited[n_index]->fold_helpers[this]->process_and_rerun_target_state(
								fold_loop_scope_counts,
								observations[n_index],
								-1,
								flat_inputs,
								activated);
						}
					}

					vector<double> obs;
					obs.push_back(problem.get_observation());

					this->explore_jump_score_network->mtx.lock();
					this->explore_jump_score_network->activate(flat_inputs,
															   activated,
															   scope_state_vals,	// actually, all empties
															   obs,
															   network_historys);
					this->explore_jump_score_network->mtx.unlock();

					scope_stacks.pop_back();
					scope_stack_counts.pop_back();

					observations.push_back(0.0);	// NULL
					scope_history.push_back((int)this->children_nodes.size());
					return this->next;
				} else {
					// iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START

				}
			}
		}
	} else if (scope_stacks.back() != this) {
		// entering scope
		vector<double> scope_states;
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			scope_states.push_back(0.0);
		}
		state_vals.push_back(scope_states);

		scope_stacks.push_back(this);
		scope_stack_counts.push_back(-1);

		observations.push_back(0.0);	// NULL
		scope_history.push_back(-1);
		return this->start;
	} else if (scope_stack_counts.back() == -1) {
		// if condition
		bool is_first_explore = false;
		if (iter_explore == NULL) {
			if (randuni() < this->if_explore_weight) {
				if (this->if_explore_state == EXPLORE_STATE_EXPLORE) {
					if (rand()%2 == 0) {
						vector<SolutionNode*> explore_path;
						new_random_path(explore_path,
										true);
						explore_path[explore_path.size()-1]->next = this;

						iter_explore = new IterExplore(this,
													   ITER_EXPLORE_TYPE_EXPLORE);
						iter_explore->local_type = ITER_EXPLORE_LOCAL_TYPE_JUMP_IF;
						iter_explore->if_type = JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND;
						iter_explore->explore_path = explore_path;
					} else {
						int random_child_index = rand()%(int)this->children_nodes.size();
						int random_end_non_inclusive_index = rand()%((int)this->children_nodes[random_child_index].size()+1);
						vector<SolutionNode*> explore_path;
						if (random_end_non_inclusive_index == 0) {
							new_random_path(explore_path,
											false);
						} else {
							new_random_path(explore_path,
											true);
						}
						explore_path[explore_path.size()-1]->next = this;

						iter_explore = new IterExplore(this,
													   ITER_EXPLORE_TYPE_EXPLORE);
						iter_explore->local_type = ITER_EXPLORE_LOCAL_TYPE_JUMP_IF;
						iter_explore->if_type = JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START;
						iter_explore->explore_path = explore_path;
						iter_explore->child_index = random_child_index;
						iter_explore->jump_end_non_inclusive_index = random_end_non_inclusive_index;
					}
				} else if (this->if_explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
					if (this->if_explore_type == JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND) {
						// go back and rerun local state historys
					} else {
						// this->if_explore_type == JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START
					}

					iter_explore = new IterExplore(this,
												   ITER_EXPLORE_TYPE_LEARN);
					iter_explore->local_type = ITER_EXPLORE_LOCAL_TYPE_JUMP_IF;
					iter_explore->explore_scope_stacks = scope_stacks;
					iter_explore->explore_scope_stack_counts = scope_stack_counts;
				} 

				is_first_explore = true;
			}
		}

		if (iter_explore != NULL
				&& iter_explore->explore_node == this
				&& iter_explore->local_type == ITER_EXPLORE_LOCAL_TYPE_JUMP_IF) {
			if (this->if_explore_state == EXPLORE_STATE_EXPLORE) {
				if (iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND) {
					if (is_first_explore || rand()%5 == 0) {
						scope_stack_counts.back() = (int)this->children_nodes.size();

						scope_stacks.push_back(NULL);
						scope_stack_counts.push_back(0);	// NULL

						observations.push_back(0.0);	// NULL
						scope_history.push_back(-1);
						return iter_explore->explore_path[0];
					}
				} else {
					// iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START
					if (is_first_explore || rand()%5 == 0) {
						scope_stack_counts.back() = iter_explore->child_index;

						scope_stacks.push_back(NULL);
						scope_stack_counts.push_back(0);	// NULL

						observations.push_back(0.0);	// NULL
						scope_history.push_back(-1);
						return iter_explore->explore_path[0];
					}
				}
			} else if (this->if_explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
				if (iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND) {
					if (is_first_explore || rand()%5 == 0) {
						scope_stack_counts.back() = (int)this->children_nodes.size();

						int input_start_index;
						for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
							if (nodes_visited[n_index] == this->explore_start_inclusive) {
								input_start_index = n_index;
								break;
							}
						}



						observations.push_back(0.0);	// NULL
						scope_history.push_back(-1);
						return iter_explore->explore_path[0];
					}
				} else {
					// iter_explore->if_type == JUMP_SCOPE_IF_EXPLORE_TYPE_BRANCH_START
				}
			}

			if (this->if_explore_type == JUMP_SCOPE_IF_EXPLORE_TYPE_APPEND) {
				if (this->if_explore_state == EXPLORE_STATE_EXPLORE) {
					
				} else if (this->if_explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
					if (is_first_explore || rand()%5 == 0) {
						activate_if_explore();	// just train greedily
					}
				} else if (this->if_explore_state == EXPLORE_STATE_MEASURE_JUMP_FLAT) {
					activate_if_explore();

					// activate child networks and compare
				}
			} else if (iter_explore->local_type == ITER_EXPLORE_LOCAL_TYPE_BRANCH_START) {
				if (this->if_explore_state == EXPLORE_STATE_EXPLORE) {
					if (is_first_explore || rand()%5 == 0) {
						scope_stack_counts.back() = iter_explore->child_index;

						observations.push_back(0.0);	// NULL
						scope_history.push_back(iter_explore->child_index);
						return iter_explore->explore_path[0];
					}
				} else if (this->if_explore_state == EXPLORE_)
			}
		}

		if (iter_explore == NULL) {

		}
		bool state_affected = false;
		// not exactly correct
		// but this is roughly the condition where state is affected
		if (explore_target == this
				&& this->explore_type == JUMP_SCOPE_EXPLORE_TYPE_TOP) {
			state_affected = true;
		}

		int best_index;
		activate_children_networks(problem,
								   state_vals.back(),
								   best_index);
	} else {
		// exiting scope

		scope_stacks.pop_back();
		scope_stack_counts.pop_back();
	}
}

void JumpScope::explore_backprop(double score,
								 SolutionNode*& explore_target) {
	if (explore_target != this) {
		// do nothing
		// nodes_visited will cause sub-nodes to be hit
	}

	if (explore_target == this
			&& this->explore_type == JUMP_SCOPE_EXPLORE_TYPE_NEW_BRANCH) {
		if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
			NetworkHistory* full_network_history = network_historys.back();

			this->explore_jump_full_score_network->mtx.lock();

			full_network_history.reset();

			vector<double> full_errors;
			if (score == 1.0) {
				if (this->explore_jump_full_score_network->output->acti_vals[0] < 1.0) {

				}
			}

			this->explore_jump_full_score_network->mtx.unlock();
		}
	}
}

void JumpScope::explore_new_branch_pre_helper(Problem& problem,
											  vector<vector<double>>& state_vals,
											  vector<SolutionNode*>& nodes_visited,
											  vector<double>& observations,
											  vector<AbstractNetworkHistory*>& network_historys,
											  bool& has_explored) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {

	} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
		int input_start_index;
		for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
			if (nodes_visited[n_index] == this->explore_start_inclusive) {
				input_start_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_flat_size] = {};
		bool activated[this->explore_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
			if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
				nodes_visited[n_index]->fold_helpers[this]->process(
					fold_loop_scope_counts,
					observations[n_index],
					-1,
					flat_inputs,
					activated);
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		this->explore_jump_score_network->mtx.lock();
		this->explore_jump_score_network->activate(flat_inputs,
												   activated,
												   scope_state_vals,	// actually, all empties
												   obs,
												   network_historys);
		this->explore_jump_score_network->mtx.unlock();
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FLAT) {
		int input_start_index;
		for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
			if (nodes_visited[n_index] == this->explore_start_inclusive) {
				input_start_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_flat_size] = {};
		bool activated[this->explore_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
			if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
				nodes_visited[n_index]->fold_helpers[this]->process(
					fold_loop_scope_counts,
					observations[n_index],
					-1,
					flat_inputs,
					activated);
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		this->explore_jump_score_network->mtx.lock();
		this->explore_jump_score_network->activate(flat_inputs,
												   activated,
												   scope_state_vals,
												   obs);
		double jump_score = this->explore_jump_score_network->output->acti_vals[0];
		this->explore_jump_score_network->mtx.unlock();

		vector<double> inputs;
		double curr_observations = problem.get_observation();
		inputs.push_back(curr_observations);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			inputs.push_back(scope_state_vals[s_index]);
		}

		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			this->children_score_networks[c_index]->mtx.lock();
			this->children_score_networks[c_index]->activate(inputs);
			double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
			this->children_score_networks[c_index]->mtx.unlock();

			double diff = jump_score - child_score;
			if (diff > 0.0) {
				this->explore_new_branch_children_diffs_positive[c_index] += diff;
			} else {
				this->explore_new_branch_children_diffs_negative[c_index] += diff;
			}
		}

		// TODO: compare against global as well
	} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FOLD) {
		if (rand()%2 == 0) {
			// activate/backprop existing?
			// for now, just use dumb way?
			// go back, re-call them all, and add to backprop_nodes
		} else {
			int input_start_index;
			for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
				if (nodes_visited[n_index] == this->explore_start_inclusive) {
					input_start_index = n_index;
					break;
				}
			}

			double flat_inputs[this->explore_flat_size] = {};
			bool activated[this->explore_flat_size] = {};

			vector<int> fold_loop_scope_counts;
			fold_loop_scope_counts.push_back(1);
			for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
				if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
					nodes_visited[n_index]->fold_helpers[this]->process(
						fold_loop_scope_counts,
						observations[n_index],
						this->explore_fold_input_index_on,
						flat_inputs,
						activated);
				}
			}

			vector<double> obs;
			obs.push_back(problem.get_observation());

			this->explore_jump_score_network->mtx.lock();
			this->explore_jump_score_network->activate(flat_inputs,
													   activated,
													   scope_state_vals,
													   obs,
													   network_historys);
			this->explore_jump_score_network->mtx.unlock();
		}
	} else {
		// this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FOLD
		// don't experiment with number of states? or can add, retraining everything anyways
	}
}

void JumpScope::explore_new_branch_post_helper(Problem& problem,
											   vector<vector<double>>& state_vals,
											   vector<SolutionNode*>& nodes_visited,
											   vector<double>& observations,
											   vector<AbstractNetworkHistory*>& network_historys,
											   bool& has_explored) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {

	} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FLAT) {
		int input_start_index;
		for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
			if (nodes_visited[n_index] == this->explore_start_inclusive) {
				input_start_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_flat_size] = {};
		bool activated[this->explore_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
			if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
				nodes_visited[n_index]->fold_helpers[this]->process(
					fold_loop_scope_counts,
					observations[n_index],
					-1,
					flat_inputs,
					activated);
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		// reuse fold_helper for both topside and for new path
		this->explore_jump_full_score_network->mtx.lock();
		this->explore_jump_full_score_network->activate(flat_inputs,
														activated,
														scope_state_vals,
														obs,
														network_historys);
		this->explore_jump_full_score_network->mtx.unlock();

		int state_input_start_index;
		for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
			if (nodes_visited[n_index] == this) {
				state_input_start_index = n_index;
				break;
			}
		}

		double state_flat_inputs[this->explore_path_flat_size] = {};
		bool state_activated[this->explore_path_flat_size] = {};

		vector<int> state_fold_loop_scope_counts;
		state_fold_loop_scope_counts.push_back(1);
		for (int n_index = state_input_start_index; state_input_start_index < (int)nodes_visited.size(); n_index++) {
			if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
				// distinct from above
				// these will all be new/temp nodes
				nodes_visited[n_index]->fold_helpers[this]->process(
					state_fold_loop_scope_counts,
					observations[n_index],
					-1,
					state_flat_inputs,
					state_activated);
			}
		}

		for (int s_index = 0; s_index < (int)this->explore_state_networks.size(); s_index++) {
			this->explore_state_networks[s_index]->mtx.lock();
			this->explore_state_networks[s_index]->activate(state_flat_inputs,
															state_activated,
															state_vals[s_index],
															obs,
															network_historys);
			this->explore_state_networks[s_index]->mtx.unlock();
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FLAT) {
		int input_start_index;
		for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
			if (nodes_visited[n_index] == this->explore_start_inclusive) {
				input_start_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_flat_size] = {};
		bool activated[this->explore_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
			if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
				nodes_visited[n_index]->fold_helpers[this]->process(
					fold_loop_scope_counts,
					observations[n_index],
					-1,
					flat_inputs,
					activated);
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		this->explore_jump_score_network->mtx.lock();
		this->explore_jump_score_network->activate(flat_inputs,
												   activated,
												   scope_state_vals,
												   obs);
		double jump_score = this->explore_jump_score_network->output->acti_vals[0];
		this->explore_jump_score_network->mtx.unlock();

		for (int a_index = 0; a_index < (int)this->explore_jump_actions.size(); a_index++) {
			problem.perform_action(this->explore_jump_actions[a_index]);
			obs.push_back(problem.get_observation());
		}

		this->explore_jump_full_score_network->mtx.lock();
		this->explore_jump_full_score_network->activate(flat_inputs,
														activated,
														scope_state_vals,
														obs);
		double jump_full_score = this->explore_jump_full_score_network->output->acti_vals[0];
		this->explore_jump_full_score_network->mtx.unlock();
		guesses.push_back(jump_full_score);
		// explore_increment target_explore to process

		vector<double> inputs;
		double curr_observations = problem.get_observation();
		inputs.push_back(curr_observations);
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			inputs.push_back(scope_state_vals[s_index]);
		}

		for (int c_index = 0; c_index < (int)this->children_nodes.size(); c_index++) {
			this->children_score_networks[c_index]->mtx.lock();
			this->children_score_networks[c_index]->activate(inputs);
			double child_score = this->children_score_networks[c_index]->output->acti_vals[0];
			this->children_score_networks[c_index]->mtx.unlock();

			double diff = jump_score - child_score;
			if (diff > 0.0) {
				this->explore_new_branch_children_diffs_positive[c_index] += diff;
			} else {
				this->explore_new_branch_children_diffs_negative[c_index] += diff;
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_JUMP_FOLD) {
		// fold states for new nodes as well
	} else {
		// this->explore_state == EXPLORE_STATE_MEASURE_JUMP_FOLD
		// don't experiment with number of states? or can add, retraining everything anyways
	}
}

// actually, always include randomness because a new path wouldn't be setting state correctly
// maybe go 50/50 depend on state or not
// use whether part of top or bottom to know if should go 50/50
void JumpScope::activate_random(vector<AbstractNetworkHistory*>& network_historys) {
	int input_start_index;
	for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
		if (nodes_visited[n_index] == this->explore_start_inclusive) {
			input_start_index = n_index;
			break;
		}
	}

	double flat_inputs[this->explore_flat_size] = {};
	bool activated[this->explore_flat_size] = {};

	vector<int> fold_loop_scope_counts;
	fold_loop_scope_counts.push_back(1);
	for (int n_index = input_start_index; input_start_index < (int)nodes_visited.size(); n_index++) {
		if (nodes_visited[n_index]->node_type == NODE_TYPE_ACTION) {
			nodes_visited[n_index]->fold_helpers[this]->process(
				fold_loop_scope_counts,
				observations[n_index],
				flat_inputs,
				activated);
		}
	}

	vector<double> obs;
	obs.push_back(problem.get_observation());
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
