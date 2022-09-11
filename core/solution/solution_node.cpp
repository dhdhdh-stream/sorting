#include "solution_node.h"

#include <cmath>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "candidate_branch.h"
#include "candidate_replace.h"
#include "definitions.h"
#include "solution_node_action.h"
#include "solution_node_empty.h"
#include "solution_node_utilities.h"
#include "utilities.h"

using namespace std;

SolutionNode::~SolutionNode() {
	// do nothing
}

void SolutionNode::explore_callback_helper(Problem& problem,
										   vector<vector<double>>& state_vals,
										   vector<SolutionNode*>& scopes,
										   vector<int>& scope_states,
										   vector<int>& scope_locations,
										   vector<ExploreStepHistory>& instance_history,
										   vector<AbstractNetworkHistory*>& network_historys) {
	scopes.pop_back();
	scope_states.pop_back();
	scope_locations.pop_back();
	// no loops for now, so always pop_back

	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		// do nothing
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == this) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_new_path_flat_size] = {};
		bool activated[this->explore_new_path_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				// n_index can't be explore_node, so action_performed always true
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				flat_inputs[input_index] = instance_history[n_index].previous_observations;
				activated[input_index] = true;
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
			this->explore_state_networks[l_index]->mtx.lock();
			this->explore_state_networks[l_index]->activate(flat_inputs,
															activated,
															state_vals[l_index],
															obs,
															network_historys);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->explore_state_networks[l_index]->output->acti_vals[s_index];
			}
			this->explore_state_networks[l_index]->mtx.unlock();
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == this) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_new_path_flat_size] = {};
		bool activated[this->explore_new_path_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				// n_index can't be explore_node, so action_performed always true
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				flat_inputs[input_index] = instance_history[n_index].previous_observations;
				activated[input_index] = true;
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
			this->explore_state_networks[l_index]->mtx.lock();
			this->explore_state_networks[l_index]->activate(flat_inputs,
															activated,
															state_vals[l_index],
															obs);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->explore_state_networks[l_index]->output->acti_vals[s_index];
			}
			this->explore_state_networks[l_index]->mtx.unlock();
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == this) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_new_path_flat_size] = {};
		bool activated[this->explore_new_path_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				node_empty->fold_helpers[this]->new_path_process(
					fold_loop_scope_counts,
					this->explore_new_path_fold_input_index_on_inclusive,
					instance_history[n_index].previous_observations,
					state_vals,
					network_historys);
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				// n_index can't be explore_node, so action_performed always true
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				node_action->fold_helpers[this]->new_path_process(
					fold_loop_scope_counts,
					this->explore_new_path_fold_input_index_on_inclusive,
					instance_history[n_index].previous_observations,
					flat_inputs,
					activated,
					state_vals,
					network_historys);
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
			this->explore_state_networks[l_index]->mtx.lock();
			this->explore_state_networks[l_index]->activate(flat_inputs,
															activated,
															state_vals[l_index],
															obs,
															network_historys);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->explore_state_networks[l_index]->output->acti_vals[s_index];
			}
			this->explore_state_networks[l_index]->mtx.unlock();
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
		// train state networks in new path, but do nothing here
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
		// do nothing
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_REPLACE) {
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == this) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_new_path_flat_size] = {};
		bool activated[this->explore_new_path_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				// also assign SolutionNodeEmpty* fold_indexes, but don't increment
				node_empty->fold_helpers[this]->new_path_process(
					fold_loop_scope_counts,
					this->explore_new_path_fold_input_index_on_inclusive,
					instance_history[n_index].previous_observations,
					state_vals,
					network_historys);
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				// n_index can't be explore_node, so action_performed always true
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				node_action->fold_helpers[this]->new_path_process(
					fold_loop_scope_counts,
					this->explore_new_path_fold_input_index_on_inclusive,
					instance_history[n_index].previous_observations,
					flat_inputs,
					activated,
					state_vals,
					network_historys);
			}
		}

		vector<double> obs;
		obs.push_back(problem.get_observation());

		for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
			this->explore_state_networks[l_index]->mtx.lock();
			this->explore_state_networks[l_index]->activate(flat_inputs,
															activated,
															state_vals[l_index],
															obs,
															network_historys);
			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_vals[l_index][s_index] = this->explore_state_networks[l_index]->output->acti_vals[s_index];
			}
			this->explore_state_networks[l_index]->mtx.unlock();
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
		// train state networks in new path, but do nothing here
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
		// do nothing
	}

	scope_locations.back() = this->parent_jump_end_non_inclusive_index;
}

void SolutionNode::is_explore_helper(vector<SolutionNode*>& scopes,
									 vector<int>& scope_states,
									 vector<int>& scope_locations,
									 IterExplore*& iter_explore,
									 bool& is_first_explore) {
	if (randuni() < this->explore_weight) {
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

SolutionNode* SolutionNode::explore_helper(bool is_first_explore,
										   Problem& problem,
										   vector<SolutionNode*>& scopes,
										   vector<int>& scope_states,
										   vector<int>& scope_locations,
										   IterExplore*& iter_explore,
										   vector<ExploreStepHistory>& instance_history,
										   vector<AbstractNetworkHistory*>& network_historys) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		if (is_first_explore || rand()%5 == 0) {
			scopes.push_back(NULL);
			scope_states.push_back(-1);
			scope_locations.push_back(-1);
			return iter_explore->explore_path[0];
		} else {
			scope_locations.back()++;
			return this->next;
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
		SolutionNode* jump_scope_start = get_jump_scope_start(this);
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == jump_scope_start) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_existing_path_flat_size] = {};
		bool activated[this->explore_existing_path_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				flat_inputs[input_index] = instance_history[n_index].previous_observations;
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
			scope_states.push_back(-1);
			scope_locations.push_back(-1);
			return this->explore_path[0];
		} else {
			this->explore_no_jump_score_network->mtx.lock();
			this->explore_no_jump_score_network->activate(flat_inputs,
														  activated,
														  obs,
														  network_historys);
			this->explore_no_jump_score_network->mtx.unlock();

			scope_locations.back()++;
			return this->next;
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
		SolutionNode* jump_scope_start = get_jump_scope_start(this);
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == jump_scope_start) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_existing_path_flat_size] = {};
		bool activated[this->explore_existing_path_flat_size] = {};

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				flat_inputs[input_index] = instance_history[n_index].previous_observations;
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
				scope_states.push_back(-1);
				scope_locations.push_back(-1);
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_EXPLORE_EXPLORE;
				return this->explore_path[0];
			} else {
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_EXPLORE_NO_EXPLORE;
				scope_locations.back()++;
				return this->next;
			}
		} else {
			if (rand()%2 == 0) {
				scopes.push_back(NULL);
				scope_states.push_back(-1);
				scope_locations.push_back(-1);
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_EXPLORE;
				return this->explore_path[0];
			} else {
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FLAT_NO_EXPLORE_NO_EXPLORE;
				scope_locations.back()++;
				return this->next;
			}
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
		SolutionNode* jump_scope_start = get_jump_scope_start(this);
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == jump_scope_start) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		double flat_inputs[this->explore_existing_path_flat_size] = {};
		bool activated[this->explore_existing_path_flat_size] = {};
		vector<double> new_state_vals(this->explore_new_state_size, 0.0);

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				// also assign SolutionNodeEmpty* fold_indexes, but don't increment
				node_empty->fold_helpers[this]->existing_path_process(
					fold_loop_scope_counts,
					this->explore_existing_path_fold_input_index_on_inclusive,
					instance_history[n_index].previous_observations,
					new_state_vals,
					network_historys);
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				node_action->fold_helpers[this]->existing_path_process(
					fold_loop_scope_counts,
					this->explore_existing_path_fold_input_index_on_inclusive,
					instance_history[n_index].previous_observations,
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
			scope_states.push_back(-1);
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

			scope_locations.back()++;
			return this->next;
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
		SolutionNode* jump_scope_start = get_jump_scope_start(this);
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == jump_scope_start) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		vector<double> new_state_vals(this->explore_new_state_size, 0.0);

		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				node_empty->fold_helpers[this]->activate_new_state_network(instance_history[n_index].previous_observations,
																		   new_state_vals);
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				node_action->fold_helpers[this]->activate_new_state_network(instance_history[n_index].previous_observations,
																			new_state_vals);
			}
		}

		// reuse new_state_vals as network inputs
		new_state_vals.push_back(problem.get_observation());

		if (rand()%2 == 0) {
			this->explore_small_jump_score_network->mtx.lock();
			this->explore_small_jump_score_network->activate(new_state_vals, network_historys);
			this->explore_small_jump_score_network->mtx.unlock();

			scopes.push_back(NULL);
			scope_states.push_back(-1);
			scope_locations.push_back(-1);
			return this->explore_path[0];
		} else {
			this->explore_small_no_jump_score_network->mtx.lock();
			this->explore_small_no_jump_score_network->activate(new_state_vals, network_historys);
			this->explore_small_no_jump_score_network->mtx.unlock();

			scope_locations.back()++;
			return this->next;
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
		SolutionNode* jump_scope_start = get_jump_scope_start(this);
		int input_start_non_inclusive_index;
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == jump_scope_start) {
				input_start_non_inclusive_index = n_index;
				break;
			}
		}

		vector<double> new_state_vals(this->explore_new_state_size, 0.0);

		for (int n_index = input_start_non_inclusive_index+1; n_index < (int)instance_history.size(); n_index++) {
			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				node_empty->fold_helpers[this]->activate_new_state_network(instance_history[n_index].previous_observations,
																		   new_state_vals);
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				node_action->fold_helpers[this]->activate_new_state_network(instance_history[n_index].previous_observations,
																			new_state_vals);
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
				scope_states.push_back(-1);
				scope_locations.push_back(-1);
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FOLD_EXPLORE;
				return this->explore_path[0];
			} else {
				instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FOLD_NO_EXPLORE;
				scope_locations.back()++;
				return this->next;
			}
		} else {
			instance_history.back().explore_decision = EXPLORE_DECISION_TYPE_FOLD_N_A;
			scope_locations.back()++;
			return this->next;
		}
	} else if (this->explore_state == ITER_EXPLORE_TYPE_LEARN_FOLD_REPLACE) {
		scopes.push_back(NULL);
		scope_states.push_back(-1);
		scope_locations.push_back(-1);
		return this->explore_path[0];
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
		scopes.push_back(NULL);
		scope_states.push_back(-1);
		scope_locations.push_back(-1);
		return this->explore_path[0];
	} else {
		// this->explore_state == ITER_EXPLORE_TYPE_MEASURE_FOLD_REPLACE
		scopes.push_back(NULL);
		scope_states.push_back(-1);
		scope_locations.push_back(-1);
		return this->explore_path[0];
	}
}

void SolutionNode::explore_callback_backprop_helper(vector<vector<double>>& state_errors,
													vector<ExploreStepHistory>& instance_history,
													vector<AbstractNetworkHistory*>& network_historys) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		// do nothing
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
		for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
			this->explore_state_networks[l_index]->mtx.lock();

			network_historys.back()->reset_weights();

			this->explore_state_networks[l_index]->full_backprop(state_errors[l_index]);

			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_errors[l_index][s_index] = this->explore_state_networks[l_index]->state_input->acti_vals[s_index];
				this->explore_state_networks[l_index]->state_input->acti_vals[s_index] = 0.0;
			}

			this->explore_state_networks[l_index]->mtx.unlock();
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
		// do nothing
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
		for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
			this->explore_state_networks[l_index]->mtx.lock();

			network_historys.back()->reset_weights();

			this->explore_state_networks[l_index]->state_backprop(state_errors[l_index]);

			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_errors[l_index][s_index] = this->explore_state_networks[l_index]->state_input->acti_vals[s_index];
				this->explore_state_networks[l_index]->state_input->acti_vals[s_index] = 0.0;
			}

			this->explore_state_networks[l_index]->mtx.unlock();
		}

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		// start from before this step_history
		for (int n_index = (int)instance_history.size()-2; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == this) {
				break;
			}

			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				int input_index = node_empty->fold_helpers[this]->get_index(fold_loop_scope_counts);
				if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
					node_empty->backprop_state_networks(state_errors,
														network_historys);
				}
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
					node_action->backprop_state_networks(state_errors,
														 network_historys);
				}
			}
			// TODO: when loops are added, update fold_loop_scope_counts
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
		// train state networks in new path, but do nothing here
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_BRANCH) {
		// do nothing
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_REPLACE) {
		for (int l_index = (int)this->local_state_sizes.size()-1; l_index >= 0; l_index--) {
			this->explore_state_networks[l_index]->mtx.lock();

			network_historys.back()->reset_weights();

			this->explore_state_networks[l_index]->state_backprop(state_errors[l_index]);

			for (int s_index = 0; s_index < this->local_state_sizes[l_index]; s_index++) {
				state_errors[l_index][s_index] = this->explore_state_networks[l_index]->state_input->acti_vals[s_index];
				this->explore_state_networks[l_index]->state_input->acti_vals[s_index] = 0.0;
			}

			this->explore_state_networks[l_index]->mtx.unlock();
		}

		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		// start from before this step_history
		for (int n_index = (int)instance_history.size()-2; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == this) {
				break;
			}

			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				int input_index = node_empty->fold_helpers[this]->get_index(fold_loop_scope_counts);
				if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
					node_empty->backprop_state_networks(state_errors,
														network_historys);
				}
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				if (input_index <= this->explore_new_path_fold_input_index_on_inclusive) {
					node_action->backprop_state_networks(state_errors,
														 network_historys);
				}
			}
			// TODO: when loops are added, update fold_loop_scope_counts
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_REPLACE) {
		// train state networks in new path, but do nothing here
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FOLD_REPLACE) {
		// do nothing
	}
}

void SolutionNode::explore_backprop_helper(double score,
										   vector<ExploreStepHistory>& instance_history,
										   vector<AbstractNetworkHistory*>& network_historys) {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		// do nothing
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FLAT) {
		if (network_historys.back()->network == this->explore_jump_score_network) {
			backprop_explore_jump_score_network(score, network_historys);
		} else {
			backprop_explore_no_jump_score_network(score, network_historys);
		}
	} else if (this->explore_state == EXPLORE_STATE_MEASURE_FLAT) {
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
	} else if (this->explore_state == EXPLORE_STATE_LEARN_FOLD_BRANCH) {
		vector<double> new_state_errors(this->explore_new_state_size, 0.0);
		if (network_historys.back()->network == this->explore_jump_score_network) {
			if (this->explore_iter_index < 150000) {
				state_backprop_explore_jump_score_network(score,
														  new_state_errors,
														  network_historys);
			} else {
				full_backprop_explore_jump_score_network(score,
														 new_state_errors,
														 network_historys);
			}
		} else {
			if (this->explore_iter_index < 150000) {
				state_backprop_explore_no_jump_score_network(score,
															 new_state_errors,
															 network_historys);
			} else {
				full_backprop_explore_no_jump_score_network(score,
															new_state_errors,
															network_historys);
			}
		}

		SolutionNode* jump_scope_start = get_jump_scope_start(this);
		vector<int> fold_loop_scope_counts;
		fold_loop_scope_counts.push_back(1);
		for (int n_index = (int)instance_history.size()-1; n_index >= 0; n_index--) {
			if (instance_history[n_index].node_visited == jump_scope_start) {
				break;
			}

			if (instance_history[n_index].node_visited->node_type == NODE_TYPE_EMPTY) {
				SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)instance_history[n_index].node_visited;
				int input_index = node_empty->fold_helpers[this]->get_index(fold_loop_scope_counts);
				if (input_index <= this->explore_existing_path_fold_input_index_on_inclusive) {
					node_empty->fold_helpers[this]->existing_path_backprop_new_state(new_state_errors,
																					 network_historys);
				}
			} else if (instance_history[n_index].node_visited->node_type == NODE_TYPE_ACTION) {
				SolutionNodeAction* node_action = (SolutionNodeAction*)instance_history[n_index].node_visited;
				int input_index = node_action->fold_helpers[this]->get_index(fold_loop_scope_counts);
				if (input_index <= this->explore_existing_path_fold_input_index_on_inclusive) {
					node_action->fold_helpers[this]->existing_path_backprop_new_state(new_state_errors,
																					  network_historys);
				}
			}
			// TODO: when loops are added, update fold_loop_scope_counts
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN_SMALL_BRANCH) {
		if (network_historys.back()->network == this->explore_small_jump_score_network) {
			backprop_explore_small_jump_score_network(score,
													  network_historys);
		} else {
			backprop_explore_small_no_jump_score_network(score,
														 network_historys);
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

void SolutionNode::explore_increment_helper(double score,
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

				this->explore_path[n_index]->initialize_local_state(this->local_state_sizes);
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

			for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
				this->explore_state_networks.push_back(
					new FoldNetwork(this->explore_new_path_flat_size,
									this->local_state_sizes[l_index],
									this->local_state_sizes[l_index]));
			}

			this->explore_jump_score_network = new FoldNetwork(this->explore_existing_path_flat_size, 1);
			this->explore_no_jump_score_network = new FoldNetwork(this->explore_existing_path_flat_size, 1);

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
					replacement_path[n_index]->get_min_misguess(min_replacement_path_misguess);
				}

				double min_new_path_misguess = numeric_limits<double>::max();
				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->get_min_misguess(min_new_path_misguess);
				}

				// TODO: add if equal, choose shorter path
				if (min_new_path_misguess < 0.9*min_replacement_path_misguess) {
					// replace
					this->explore_replace_type = EXPLORE_REPLACE_TYPE_INFO;
					this->explore_replace_info_gain = 1.0 - min_new_path_misguess/min_replacement_path_misguess;

					this->explore_state = EXPLORE_STATE_LEARN_FOLD_REPLACE;
					this->explore_iter_index = 0;

					this->explore_new_path_fold_input_index_on_inclusive = 0;
				} else {
					// abandon
					explore_abandon_helper();

					this->explore_state = EXPLORE_STATE_EXPLORE;
				}
			} else {
				// abandon
				explore_abandon_helper();

				this->explore_state = EXPLORE_STATE_EXPLORE;
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
																	 this->explore_new_state_size,
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

				for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
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
				explore_abandon_helper();
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
				solution->candidates.push_back(new_candidate);

				this->explore_path.clear();

				vector<SolutionNode*> existing_path;
				get_existing_path(this, existing_path);
				for (int n_index = 0; n_index < (int)existing_path.size(); n_index++) {
					existing_path[n_index]->cleanup_explore(this);
				}

				for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
					delete this->explore_state_networks[l_index];
				}
				this->explore_state_networks.clear();

				delete this->explore_jump_score_network;
				this->explore_jump_score_network = NULL;
				delete this->explore_no_jump_score_network;
				this->explore_no_jump_score_network = NULL;
			} else {
				explore_abandon_helper();
			}

			this->explore_state = EXPLORE_STATE_EXPLORE;
		}
	}
}

void SolutionNode::explore_abandon_helper() {
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

	for (int l_index = 0; l_index < (int)this->local_state_sizes.size(); l_index++) {
		delete this->explore_state_networks[l_index];
	}
	this->explore_state_networks.clear();

	delete this->explore_jump_score_network;
	this->explore_jump_score_network = NULL;
	delete this->explore_no_jump_score_network;
	this->explore_no_jump_score_network = NULL;

	if (this->explore_small_jump_score_network != NULL) {
		delete this->explore_small_jump_score_network;
		this->explore_small_jump_score_network = NULL;
	}
	if (this->explore_small_no_jump_score_network != NULL) {
		delete this->explore_small_no_jump_score_network;
		this->explore_small_no_jump_score_network = NULL;
	}
}

void SolutionNode::backprop_explore_jump_score_network(double score,
													   vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_jump_score_network->full_backprop(score_errors);	

	this->explore_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::backprop_explore_no_jump_score_network(double score,
														  vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_no_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_no_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_no_jump_score_network->full_backprop(score_errors);	

	this->explore_no_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::state_backprop_explore_jump_score_network(double score,
															 vector<double>& new_state_errors,
															 vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_jump_score_network->state_backprop(score_errors);

	for (int s_index = 0; s_index < this->explore_new_state_size; s_index++) {
		new_state_errors[s_index] = this->explore_jump_score_network->state_input->errors[s_index];
		this->explore_jump_score_network->state_input->errors[s_index] = 0.0;
	}

	this->explore_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::state_backprop_explore_no_jump_score_network(double score,
																vector<double>& new_state_errors,
																vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_no_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_no_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_no_jump_score_network->state_backprop(score_errors);

	for (int s_index = 0; s_index < this->explore_new_state_size; s_index++) {
		new_state_errors[s_index] = this->explore_no_jump_score_network->state_input->errors[s_index];
		this->explore_no_jump_score_network->state_input->errors[s_index] = 0.0;
	}

	this->explore_no_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::full_backprop_explore_jump_score_network(double score,
															vector<double>& new_state_errors,
															vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_jump_score_network->full_backprop(score_errors);

	for (int s_index = 0; s_index < this->explore_new_state_size; s_index++) {
		new_state_errors[s_index] = this->explore_jump_score_network->state_input->errors[s_index];
		this->explore_jump_score_network->state_input->errors[s_index] = 0.0;
	}

	this->explore_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::full_backprop_explore_no_jump_score_network(double score,
															   vector<double>& new_state_errors,
															   vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_no_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_no_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_no_jump_score_network->full_backprop(score_errors);

	for (int s_index = 0; s_index < this->explore_new_state_size; s_index++) {
		new_state_errors[s_index] = this->explore_no_jump_score_network->state_input->errors[s_index];
		this->explore_no_jump_score_network->state_input->errors[s_index] = 0.0;
	}

	this->explore_no_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::backprop_explore_small_jump_score_network(double score,
															 std::vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_small_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_small_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_small_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_small_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_small_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_small_jump_score_network->backprop(score_errors);

	this->explore_small_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::backprop_explore_small_no_jump_score_network(double score,
																std::vector<AbstractNetworkHistory*>& network_historys) {
	AbstractNetworkHistory* network_history = network_historys.back();

	this->explore_small_no_jump_score_network->mtx.lock();

	network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_small_no_jump_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_small_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_small_no_jump_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_small_no_jump_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_small_no_jump_score_network->backprop(score_errors);

	this->explore_small_no_jump_score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}
