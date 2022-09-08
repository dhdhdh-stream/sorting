#include "jump_scope.h"

using namespace std;

JumpScope::JumpScope() {

}

JumpScope::~JumpScope() {

}

SolutionNode* JumpScope::re_eval(Problem& problem,
								 vector<vector<double>>& state_vals,
								 vector<SolutionNode*>& scopes,
								 vector<int>& scope_states,
								 vector<ReEvalStepHistory>& instance_history,
								 vector<AbstractNetworkHistory*>& network_historys) {
	// TODO: handle case where top_path is empty
	if (scope_states.back() != this) {
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

		return this->top_path[0];
	} else if (scope_stack_counts.back() == -1) {
		// if condition
		int best_index;
		// add 10% random chance
		activate_children_networks(problem,
								   state_vals.back(),
								   best_index,
								   network_historys);

		// TODO: handle case where child path is empty

	} else {
		// exiting scope
	}
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
	// TODO: handle case where top_path is empty
	if (iter_explore->explore_node == this
			&& scope_stacks.back() == NULL) {
		// back from explore
		
	} else if (scope_stacks.back() != this) {
		// entering scope
		vector<double> scope_states;
		for (int s_index = 0; s_index < this->num_states; s_index++) {
			scope_states.push_back(0.0);
		}
		state_vals.push_back(scope_states);

		scopes.push_back(this);
		scope_states.push_back(-1);
		scope_locations.push_back(0);

		instance_history.push_back(StepHistory(this,
											   false,
											   0.0,
											   -1,
											   -1,
											   false));

		return this->top_path[0];
	} else if (scope_stack_counts.back() == -1) {
		// if condition
		bool state_affected = false;
		if (iter_explore != NULL) {
			for (int s_index = 0; s_index < (int)iter_explore->scopes.size(); s_index++) {
				if (iter_explore->scopes[s_index] == this) {
					if (iter_explore->scope_states[s_index] == -1) {
						state_affected = true;
					}
					break;
				}
			}
		}

		// TODO: handle case where child path is empty
		int best_index;
		activate_children_networks(problem,
								   state_vals.back(),
								   best_index);

		// set state_vals.back to 0

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
