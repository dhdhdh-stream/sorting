#include "solution_node.h"

#include <cmath>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"

using namespace std;

SolutionNode::~SolutionNode() {
	// do nothing
}

SolutionNode* SolutionNode::explore_activate(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		vector<SolutionNode*>& loop_scopes,
		vector<int>& loop_scope_counts,
		int& iter_explore_type,
		SolutionNode*& iter_explore_node,
		IterExplore*& iter_explore,
		bool is_first_explore,
		double* potential_state_vals,
		vector<int>& potential_state_indexes,
		vector<NetworkHistory*>& network_historys,
		vector<int>& explore_decisions) {
	if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
		if (iter_explore->iter_explore_type == ITER_EXPLORE_TYPE_JUMP) {
			if (is_first_explore || rand()%20 == 0) {
				for (int a_index = 0; a_index < (int)iter_explore->try_path.size(); a_index++) {
					problem.perform_action(iter_explore->try_path[a_index]);
				}

				return iter_explore->iter_end_non_inclusive;
			} else {
				return NULL;
			}
		} else {
			// iter_explore->iter_explore_type == ITER_EXPLORE_TYPE_LOOP
			if (loop_scopes.back() == NULL) {
				if ((loop_scope_counts.back() == 3 && rand()%3 == 0)
						|| (loop_scope_counts.back() == 4 && rand()%2 == 0)
						|| loop_scope_counts.back() == 5) {
					loop_scopes.pop_back();
					if (loop_scopes.size() == 0) {
						cout << "ERROR #1" << endl;
					}
					loop_scope_counts.pop_back();

					return NULL;
				} else {
					loop_scope_counts.back()++;

					return iter_explore->iter_start_inclusive;
				}
			} else {
				if (is_first_explore || rand()%20 == 0) {
					loop_scopes.push_back(NULL);
					loop_scope_counts.push_back(1);

					return iter_explore->iter_start_inclusive;
				} else {
					return NULL;
				}
			}
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
		if ((is_first_explore && rand()%20 != 0)
				|| rand()%20 == 0) {
			activate_explore_jump_network(problem,
										  state_vals,
										  states_on,
										  true,
										  network_historys);

			if (this->explore_path.size() > 0) {
				return this->explore_path[0];
			} else {
				return this->explore_end_non_inclusive;
			}
		} else {
			activate_explore_no_jump_network(problem,
											 state_vals,
											 states_on,
											 true,
											 network_historys);

			return NULL;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
		double explore_score = activate_explore_jump_network(
			problem,
			state_vals,
			states_on,
			false,
			network_historys);
		double no_explore_score = activate_explore_no_jump_network(
			problem,
			state_vals,
			states_on,
			false,
			network_historys);
		if (explore_score > no_explore_score) {
			if (rand()%2 == 0) {
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
				if (this->explore_path.size() > 0) {
					return this->explore_path[0];
				} else {
					return this->explore_end_non_inclusive;
				}
			} else {
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
				return NULL;
			}
		} else {
			explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);
			return NULL;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
		if (loop_scopes.back() == NULL) {
			int current_count = loop_scope_counts.back();
			if (rand()%(6-current_count) == 0) {
				activate_explore_halt_network(problem,
											  state_vals,
											  states_on,
											  potential_state_vals,
											  true,
											  network_historys);

				loop_scopes.pop_back();
				if (loop_scopes.size() == 0) {
					cout << "ERROR #5" << endl;
				}
				loop_scope_counts.pop_back();

				return NULL;
			} else {
				activate_explore_no_halt_network(problem,
												 state_vals,
												 states_on,
												 potential_state_vals,
												 true,
												 network_historys);

				loop_scope_counts.back()++;

				return this->explore_start_inclusive;
			}
		} else {
			if (is_first_explore || rand()%20 == 0) {
				activate_explore_no_halt_network(problem,
												 state_vals,
												 states_on,
												 potential_state_vals,
												 true,
												 network_historys);

				for (int ps_index = 0; ps_index < 2; ps_index++) {
					potential_state_vals[ps_index] = 0.0;
				}

				loop_scopes.push_back(NULL);
				loop_scope_counts.push_back(1);

				return this->explore_start_inclusive;
			} else {
				activate_explore_halt_network(problem,
											  state_vals,
											  states_on,
											  potential_state_vals,
											  true,
											  network_historys);

				return NULL;
			}
		}
	} else {
		// this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP
		if (loop_scopes.back() == NULL) {
			explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);

			if (loop_scope_counts.back() >= 20) {
				loop_scopes.pop_back();
				if (loop_scopes.size() == 0) {
					cout << "ERROR #4" << endl;
				}
				loop_scope_counts.pop_back();

				return NULL;
			}

			double halt_score = activate_explore_halt_network(
				problem,
				state_vals,
				states_on,
				potential_state_vals,
				false,
				network_historys);
			double no_halt_score = activate_explore_no_halt_network(
				problem,
				state_vals,
				states_on,
				potential_state_vals,
				false,
				network_historys);
			if (halt_score > no_halt_score) {
				loop_scopes.pop_back();
				if (loop_scopes.size() == 0) {
					cout << "ERROR #3" << endl;
				}
				loop_scope_counts.pop_back();

				return NULL;
			} else {
				loop_scope_counts.back()++;

				return this->explore_start_inclusive;
			}
		} else {
			double halt_score = activate_explore_halt_network(
				problem,
				state_vals,
				states_on,
				potential_state_vals,
				false,
				network_historys);
			double no_halt_score = activate_explore_no_halt_network(
				problem,
				state_vals,
				states_on,
				potential_state_vals,
				false,
				network_historys);

			if (halt_score > no_halt_score) {
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);

				return NULL;
			} else {
				if (rand()%2 == 0) {
					for (int ps_index = 0; ps_index < 2; ps_index++) {
						potential_state_vals[ps_index] = 0.0;
					}

					loop_scopes.push_back(NULL);
					loop_scope_counts.push_back(1);

					explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
					return this->explore_start_inclusive;
				} else {
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
					return NULL;
				}
			}
		}
	}
}

void SolutionNode::explore_backprop(double score,
									double misguess,
									double* state_errors,
									bool* states_on,
									SolutionNode*& iter_explore_node,
									double* potential_state_errors,
									vector<NetworkHistory*>& network_historys,
									vector<int>& explore_decisions) {
	if (iter_explore_node == this) {
		if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
			// do nothing
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
			NetworkHistory* network_history = network_historys.back();
			if (network_history->network == this->explore_jump_network) {
				backprop_explore_jump_network(score,
											  state_errors,
											  states_on,
											  network_historys);
			} else {
				backprop_explore_no_jump_network(score,
												 state_errors,
												 states_on,
												 network_historys);
			}
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
			if (explore_decisions.back() == EXPLORE_DECISION_TYPE_EXPLORE) {
				this->explore_path_measure_count++;
				if (score == 1.0) {
					this->explore_explore_is_good += 1;
				} else {
					this->explore_explore_is_bad += 1;
				}
				this->explore_explore_misguess += misguess;
			} else if (explore_decisions.back() == EXPLORE_DECISION_TYPE_NO_EXPLORE) {
				this->explore_path_measure_count++;
				if (score == 1.0) {
					this->explore_no_explore_is_good += 1;
				} else {
					this->explore_no_explore_is_bad += 1;
				}
				this->explore_no_explore_misguess += misguess;
			}

			explore_decisions.pop_back();
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
			NetworkHistory* network_history = network_historys.back();
			if (network_history->network == this->explore_halt_network) {
				for (int ps_index = 0; ps_index < 2; ps_index++) {
					potential_state_errors[ps_index] = 0.0;
				}

				backprop_explore_halt_network(score,
											  potential_state_errors,
											  network_historys);
			} else {
				backprop_explore_no_halt_network(score,
												 potential_state_errors,
												 network_historys);
			}
		} else {
			// this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP
			if (explore_decisions.back() == EXPLORE_DECISION_TYPE_EXPLORE) {
				this->explore_path_measure_count++;
				if (score == 1.0) {
					this->explore_explore_is_good += 1;
				} else {
					this->explore_explore_is_bad += 1;
				}
				this->explore_explore_misguess += misguess;
			} else if (explore_decisions.back() == EXPLORE_DECISION_TYPE_NO_EXPLORE) {
				this->explore_path_measure_count++;
				if (score == 1.0) {
					this->explore_no_explore_is_good += 1;
				} else {
					this->explore_no_explore_is_bad += 1;
				}
				this->explore_no_explore_misguess += misguess;
			}

			explore_decisions.pop_back();
		}
	}
}

void SolutionNode::explore_increment(double score,
									 IterExplore* iter_explore) {
	if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
		if (score == 1.0) {
			if (iter_explore->iter_explore_type == ITER_EXPLORE_TYPE_JUMP) {
				if (iter_explore->try_path.size() > 0) {
					for (int a_index = 0; a_index < (int)iter_explore->try_path.size(); a_index++) {
						cout << iter_explore->try_path[a_index].to_string() << endl;
					}
					cout << endl;

					SolutionNodeAction* curr_node = new SolutionNodeAction(
						this->solution,
						-1,
						iter_explore->try_path[0],
						iter_explore->available_state);
					curr_node->temp_node_state = TEMP_NODE_STATE_LEARN;
					this->explore_path.push_back(curr_node);
					for (int a_index = 1; a_index < (int)iter_explore->try_path.size(); a_index++) {
						SolutionNodeAction* next_node = new SolutionNodeAction(
							this->solution,
							-1,
							iter_explore->try_path[a_index],
							iter_explore->available_state);
						next_node->temp_node_state = TEMP_NODE_STATE_LEARN;
						this->explore_path.push_back(next_node);
						// no need to set previous
						curr_node->next = next_node;
						curr_node = next_node;
					}
					curr_node->next = iter_explore->iter_end_non_inclusive;
				}

				this->explore_start_non_inclusive = iter_explore->iter_start_non_inclusive;
				this->explore_start_inclusive = iter_explore->iter_start_inclusive;
				this->explore_end_inclusive = iter_explore->iter_end_inclusive;
				this->explore_end_non_inclusive = iter_explore->iter_end_non_inclusive;

				if (this->node_type == NODE_TYPE_IF_START) {
					SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
					this_if_start->explore_child_index = iter_explore->iter_child_index;
				}

				this->explore_network_inputs_state_indexes = iter_explore->available_state;
				int input_size = 1 + (int)iter_explore->available_state.size();
				this->explore_jump_network = new Network(input_size,
														 4*input_size,
														 1);
				this->explore_no_jump_network = new Network(input_size,
															4*input_size,
															1);

				this->explore_path_state = EXPLORE_PATH_STATE_LEARN_JUMP;
				this->explore_path_iter_index = 0;
			} else if (iter_explore->iter_explore_type == ITER_EXPLORE_TYPE_LOOP) {
				this->explore_start_non_inclusive = iter_explore->iter_start_non_inclusive;
				this->explore_start_inclusive = iter_explore->iter_start_inclusive;
				this->explore_end_inclusive = iter_explore->iter_end_inclusive;
				this->explore_end_non_inclusive = iter_explore->iter_end_non_inclusive;

				this->solution->potential_state_mtx.lock();
				for (int i = 0; i < 2; i++) {
					this->explore_loop_states.push_back(this->solution->current_potential_state_counter);
					this->solution->current_potential_state_counter++;
				}
				this->solution->potential_state_mtx.unlock();
				
				this->explore_start_inclusive->add_potential_state(this->explore_loop_states, this);

				this->explore_network_inputs_state_indexes = iter_explore->available_state;
				int input_size = 1 + (int)iter_explore->available_state.size() + 2;
				this->explore_halt_network = new Network(input_size,
														 4*input_size,
														 1);
				this->explore_no_halt_network = new Network(input_size,
															4*input_size,
															1);

				this->explore_path_state = EXPLORE_PATH_STATE_LEARN_LOOP;
				this->explore_path_iter_index = 0;
			}
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 2000000) {
		if (this->explore_path_iter_index > 10) {
			for (int e_index = 0; e_index < (int)this->explore_path.size(); e_index++) {
				((SolutionNodeAction*)this->explore_path[e_index])->temp_node_state = TEMP_NODE_STATE_MEASURE;
			}

			this->explore_path_state = EXPLORE_PATH_STATE_MEASURE_JUMP;
			this->explore_path_iter_index = 0;

			this->explore_path_measure_count = 0;
			this->explore_explore_is_good = 0.0;
			this->explore_explore_is_bad = 0.0;
			this->explore_explore_misguess = 0.0;
			this->explore_no_explore_is_good = 0.0;
			this->explore_no_explore_is_bad = 0.0;
			this->explore_no_explore_misguess = 0.0;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 100000) {
		if (this->explore_path_iter_index > 5) {
			double improvement = explore_explore_is_good - explore_no_explore_is_good;
			// TODO: add misguess
			if (improvement > 0.0) {
				if (this->node_type == NODE_TYPE_IF_START
						&& ((SolutionNodeIfStart*)this)->explore_child_index == -1) {
					this->solution->nodes_mtx.lock();

					vector<int> new_path_node_indexes;
					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						int new_index = (int)this->solution->nodes.size();

						((SolutionNodeAction*)this->explore_path[n_index])->temp_node_state = TEMP_NODE_STATE_NOT;
						this->explore_path[n_index]->node_index = new_index;

						this->solution->nodes.push_back(this->explore_path[n_index]);
						new_path_node_indexes.push_back(new_index);
					}
					this->explore_path.clear();

					this->solution->nodes_mtx.unlock();

					SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
					this_if_start->children_nodes.push_back(NULL);
					while (this->explore_jump_network->input->acti_vals.size()
							< this_if_start->children_networks_inputs_state_indexes.size()) {
						this->explore_jump_network->pad_input();
					}
					this_if_start->children_score_networks.push_back(this->explore_jump_network);
					this->explore_jump_network = NULL;
					delete this->explore_no_jump_network;
					this->explore_no_jump_network = NULL;
					int input_size = 1 + (int)this_if_start->children_networks_inputs_state_indexes.size();
					this_if_start->children_certainty_networks.push_back(new Network(input_size,
																					 4*input_size,
																					 1));
					this_if_start->children_on.push_back(false);

					ExploreNodeAppendJump* new_explore_node = new ExploreNodeAppendJump(
						this->solution->explore,
						this->node_index,
						(int)this_if_start->children_nodes.size()-1,
						new_path_node_indexes);
					this->solution->explore->mtx.lock();
					this->solution->explore->current_node->children.push_back(new_explore_node);
					this->solution->explore->mtx.unlock();
					cout << "new ExploreNodeAppendJump" << endl;
				} else {
					this->solution->nodes_mtx.lock();

					int new_start_node_index = (int)this->solution->nodes.size();
					SolutionNodeIfStart* new_start_node = new SolutionNodeIfStart(
						this,
						new_start_node_index);
					// takes and clears explore networks
					this->solution->nodes.push_back(new_start_node);

					vector<int> new_path_node_indexes;
					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						int new_index = (int)this->solution->nodes.size();

						((SolutionNodeAction*)this->explore_path[n_index])->temp_node_state = TEMP_NODE_STATE_NOT;
						this->explore_path[n_index]->node_index = new_index;

						this->solution->nodes.push_back(this->explore_path[n_index]);
						new_path_node_indexes.push_back(new_index);
					}
					this->explore_path.clear();

					int new_end_node_index = (int)this->solution->nodes.size();
					SolutionNodeIfEnd* new_end_node = new SolutionNodeIfEnd(this->solution, new_end_node_index);
					this->solution->nodes.push_back(new_end_node);

					this->solution->nodes_mtx.unlock();

					ExploreNodeNewJump* new_explore_node;
					if (this->explore_start_inclusive == NULL) {
						new_explore_node = new ExploreNodeNewJump(
							this->solution->explore,
							this->explore_start_non_inclusive->node_index,
							-1,
							-1,
							this->explore_end_non_inclusive->node_index,
							new_start_node_index,
							new_end_node_index,
							new_path_node_indexes);
						cout << "this: " << this->node_index << endl;
						cout << "start_non_inclusive: " << this->explore_start_non_inclusive->node_index << endl;
						cout << "start_inclusive: " << -1 << endl;
						cout << "end_inclusive: " << -1 << endl;
						cout << "end_non_inclusive: " << this->explore_end_non_inclusive->node_index << endl;
						cout << "new_start_node_index: " << new_start_node_index << endl;
						cout << "new_end_node_index: " << new_end_node_index << endl;
					} else {
						new_explore_node = new ExploreNodeNewJump(
							this->solution->explore,
							this->explore_start_non_inclusive->node_index,
							this->explore_start_inclusive->node_index,
							this->explore_end_inclusive->node_index,
							this->explore_end_non_inclusive->node_index,
							new_start_node_index,
							new_end_node_index,
							new_path_node_indexes);
						cout << "this: " << this->node_index << endl;
						cout << "start_non_inclusive: " << this->explore_start_non_inclusive->node_index << endl;
						cout << "start_inclusive: " << this->explore_start_inclusive->node_index << endl;
						cout << "end_inclusive: " << this->explore_end_inclusive->node_index << endl;
						cout << "end_non_inclusive: " << this->explore_end_non_inclusive->node_index << endl;
						cout << "new_start_node_index: " << new_start_node_index << endl;
						cout << "new_end_node_index: " << new_end_node_index << endl;
					}
					this->solution->explore->mtx.lock();
					this->solution->explore->current_node->children.push_back(new_explore_node);
					this->solution->explore->mtx.unlock();
					cout << "new ExploreNodeNewJump" << endl;
				}
			} else {
				delete this->explore_jump_network;
				this->explore_jump_network = NULL;
				delete this->explore_no_jump_network;
				this->explore_no_jump_network = NULL;

				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					delete this->explore_path[n_index];
				}
				this->explore_path.clear();
			}

			this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 2000000) {
		if (this->explore_path_iter_index > 10) {
			this->explore_path_state = EXPLORE_PATH_STATE_MEASURE_LOOP;
			this->explore_path_iter_index = 0;

			this->explore_path_measure_count = 0;
			this->explore_explore_is_good = 0.0;
			this->explore_explore_is_bad = 0.0;
			this->explore_explore_misguess = 0.0;
			this->explore_no_explore_is_good = 0.0;
			this->explore_no_explore_is_bad = 0.0;
			this->explore_no_explore_misguess = 0.0;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 100000) {
		if (this->explore_path_iter_index > 5) {
			double improvement = explore_explore_is_good - explore_no_explore_is_good;
			// TODO: add misguess
			if (improvement > 0.0) {
				vector<int> new_state_indexes;
				this->solution->state_mtx.lock();
				for (int p_index = 0; p_index < (int)this->explore_loop_states.size(); p_index++) {
					new_state_indexes.push_back(this->solution->current_state_counter);
					this->solution->current_state_counter++;
				}
				this->solution->state_mtx.unlock();
				this->explore_start_inclusive->extend_with_potential_state(
					this->explore_loop_states,
					new_state_indexes,
					this);
				this->explore_loop_states.clear();

				this->solution->nodes_mtx.lock();

				int new_start_node_index = (int)this->solution->nodes.size();
				SolutionNodeLoopStart* new_start_node = new SolutionNodeLoopStart(
					this->solution,
					new_start_node_index,
					new_state_indexes);
				this->solution->nodes.push_back(new_start_node);

				int new_end_node_index = (int)this->solution->nodes.size();
				SolutionNodeLoopEnd* new_end_node = new SolutionNodeLoopEnd(
					this, new_end_node_index);
				// takes and clears explore networks
				this->solution->nodes.push_back(new_end_node);

				this->solution->nodes_mtx.unlock();

				ExploreNodeLoop* new_explore_node;
				new_explore_node = new ExploreNodeLoop(
					this->solution->explore,
					this->explore_start_non_inclusive->node_index,
					this->explore_start_inclusive->node_index,
					this->explore_end_inclusive->node_index,
					this->explore_end_non_inclusive->node_index,
					new_start_node_index,
					new_end_node_index);
				this->solution->explore->mtx.lock();
				this->solution->explore->current_node->children.push_back(new_explore_node);
				this->solution->explore->mtx.unlock();
				cout << "new ExploreNodeLoop" << endl;
			} else {
				delete this->explore_halt_network;
				this->explore_halt_network = NULL;
				delete this->explore_no_halt_network;
				this->explore_no_halt_network = NULL;

				this->explore_start_inclusive->delete_potential_state(
					this->explore_loop_states,
					this);
				this->explore_loop_states.clear();
			}

			this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
		}
	}
}

void SolutionNode::clear_explore() {
	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;

	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}
	this->explore_path.clear();

	if (this->explore_jump_network != NULL) {
		delete this->explore_jump_network;
		this->explore_jump_network = NULL;
	}
	if (this->explore_no_jump_network != NULL) {
		delete this->explore_no_jump_network;
		this->explore_no_jump_network = NULL;
	}
	if (this->explore_halt_network != NULL) {
		delete this->explore_halt_network;
		this->explore_halt_network = NULL;
	}
	if (this->explore_no_halt_network != NULL) {
		delete this->explore_no_halt_network;
		this->explore_no_halt_network = NULL;
	}

	this->explore_loop_states.clear();
}

void SolutionNode::update_node_weight(double new_node_weight) {
	this->node_weight = 0.9999*this->node_weight + 0.0001*new_node_weight;
}

double SolutionNode::activate_explore_jump_network(Problem& problem,
												   double* state_vals,
												   bool* states_on,
												   bool backprop,
												   vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->explore_network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->explore_jump_network->mtx.lock();
		this->explore_jump_network->activate(explore_network_inputs, network_historys);
		score = this->explore_jump_network->output->acti_vals[0];
		this->explore_jump_network->mtx.unlock();
	} else {
		this->explore_jump_network->mtx.lock();
		this->explore_jump_network->activate(explore_network_inputs);
		score = this->explore_jump_network->output->acti_vals[0];
		this->explore_jump_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_jump_network(double score,
												 double* state_errors,
												 bool* states_on,
												 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_jump_network->mtx.lock();

	if (network_history->network != this->explore_jump_network) {
		cout << "ERROR: explore_jump_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_jump_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_jump_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_jump_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_jump_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_jump_network->backprop(explore_network_errors);

	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			state_errors[this->explore_network_inputs_state_indexes[i_index]] += \
				this->explore_jump_network->input->errors[1+i_index];
		}
		this->explore_jump_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_jump_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_no_jump_network(Problem& problem,
													  double* state_vals,
													  bool* states_on,
													  bool backprop,
													  vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->explore_network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->explore_no_jump_network->mtx.lock();
		this->explore_no_jump_network->activate(explore_network_inputs, network_historys);
		score = this->explore_no_jump_network->output->acti_vals[0];
		this->explore_no_jump_network->mtx.unlock();
	} else {
		this->explore_no_jump_network->mtx.lock();
		this->explore_no_jump_network->activate(explore_network_inputs);
		score = this->explore_no_jump_network->output->acti_vals[0];
		this->explore_no_jump_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_no_jump_network(double score,
													double* state_errors,
													bool* states_on,
													vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_no_jump_network->mtx.lock();

	if (network_history->network != this->explore_no_jump_network) {
		cout << "ERROR: explore_no_jump_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_no_jump_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_no_jump_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_jump_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_no_jump_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_no_jump_network->backprop(explore_network_errors);

	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			state_errors[this->explore_network_inputs_state_indexes[i_index]] += \
				this->explore_no_jump_network->input->errors[1+i_index];
		}
		this->explore_no_jump_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_no_jump_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_halt_network(Problem& problem,
												   double* state_vals,
												   bool* states_on,
												   double* potential_state_vals,
												   bool backprop,
												   vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->explore_network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
	}
	for (int ps_index = 0; ps_index < 2; ps_index++) {
		explore_network_inputs.push_back(potential_state_vals[ps_index]);
	}

	double score;
	if (backprop) {
		this->explore_halt_network->mtx.lock();
		this->explore_halt_network->activate(explore_network_inputs, network_historys);
		score = this->explore_halt_network->output->acti_vals[0];
		this->explore_halt_network->mtx.unlock();
	} else {
		this->explore_halt_network->mtx.lock();
		this->explore_halt_network->activate(explore_network_inputs);
		score = this->explore_halt_network->output->acti_vals[0];
		this->explore_halt_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_halt_network(double score,
												 double* potential_state_errors,
												 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_halt_network->mtx.lock();

	if (network_history->network != this->explore_halt_network) {
		cout << "ERROR: halt_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_halt_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_halt_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_halt_network->backprop(explore_network_errors);

	for (int ps_index = 0; ps_index < 2; ps_index++) {
		potential_state_errors[ps_index] += this->explore_halt_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index];
		this->explore_halt_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index] = 0.0;
	}

	this->explore_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_no_halt_network(Problem& problem,
													  double* state_vals,
													  bool* states_on,
													  double* potential_state_vals,
													  bool backprop,
													  vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->explore_network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
	}
	for (int ps_index = 0; ps_index < 2; ps_index++) {
		explore_network_inputs.push_back(potential_state_vals[ps_index]);
	}

	double score;
	if (backprop) {
		this->explore_no_halt_network->mtx.lock();
		this->explore_no_halt_network->activate(explore_network_inputs, network_historys);
		score = this->explore_no_halt_network->output->acti_vals[0];
		this->explore_no_halt_network->mtx.unlock();
	} else {
		this->explore_no_halt_network->mtx.lock();
		this->explore_no_halt_network->activate(explore_network_inputs);
		score = this->explore_no_halt_network->output->acti_vals[0];
		this->explore_no_halt_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_no_halt_network(double score,
													double* potential_state_errors,
													std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_no_halt_network->mtx.lock();

	if (network_history->network != this->explore_no_halt_network) {
		cout << "ERROR: explore_no_halt_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_no_halt_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_no_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_halt_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_no_halt_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_no_halt_network->backprop(explore_network_errors);

	for (int ps_index = 0; ps_index < 2; ps_index++) {
		potential_state_errors[ps_index] += this->explore_no_halt_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index];
		this->explore_no_halt_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index] = 0.0;
	}

	this->explore_no_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}
