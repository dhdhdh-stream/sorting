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

void SolutionNode::activate_helper(Problem& problem,
								   double* state_vals,
								   bool* states_on,
								   int& iter_explore_type,
								   SolutionNode*& iter_explore_node,
								   double* potential_state_vals,
								   vector<int>& potential_state_indexes,
								   vector<NetworkHistory*>& network_historys,
								   vector<vector<double>>& guesses) {
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		double predicted_score;
		double predicted_misguess;
		activate_score_network(problem,
							   state_vals,
							   states_on,
							   true,
							   network_historys,
							   predicted_score,
							   predicted_misguess);
		if (this->node_type == NODE_TYPE_ACTION
				|| this->node_type == NODE_TYPE_START) {
			guesses.back().push_back(predicted_score);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		// do nothing
	} else {
		// do nothing
	}
}

void SolutionNode::backprop_helper(double score,
								   double misguess,
								   double* state_errors,
								   bool* states_on,
								   int& iter_explore_type,
								   SolutionNode*& iter_explore_node,
								   double* potential_state_errors,
								   vector<NetworkHistory*>& network_historys) {
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_score_network(score,
							   misguess,
							   state_errors,
							   states_on,
							   network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_JUMP) {
		if (this->node_index == -1) {
			backprop_score_network(score,
								   misguess,
								   state_errors,
								   states_on,
								   network_historys);
		} else {
			if (iter_explore_node != this) {
				backprop_score_network_errors_with_no_weight_change(
					score,
					misguess,
					state_errors,
					states_on,
					network_historys);
			}
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_JUMP) {
		// do nothing
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_LOOP) {
		if (iter_explore_node != this) {
			backprop_score_network_with_potential(score,
												  misguess,
												  potential_state_errors,
												  network_historys);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_LOOP) {
		// do nothing
	}
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
		vector<vector<double>>& guesses,
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
		// fetch initial observation

		vector<double> additional_observations;
		for (int a_index = 0; a_index < (int)iter_explore->try_path.size(); a_index++) {
			problem.perform_action(iter_explore->try_path[a_index]);
			additional_observations.push_back(problem.get_observation());
		}

		if (is_first_explore || rand()%20 == 0) {
			double predicted_score;
			double predicted_misguess;
			activate_explore_jump_network(problem,
										  state_vals,
										  states_on,
										  true,
										  network_historys,
										  predicted_score,
										  predicted_misguess);
			if (this->node_type == NODE_TYPE_ACTION
					|| this->node_type == NODE_TYPE_START) {
				guesses.back().push_back(predicted_score);
			}

			if (this->explore_path.size() > 0) {
				return this->explore_path[0];
			} else {
				return this->explore_end_non_inclusive;
			}
		} else {
			double predicted_score;
			double predicted_misguess;
			activate_score_network(problem,
								   state_vals,
								   states_on,
								   true,
								   network_historys,
								   predicted_score,
								   predicted_misguess);
			if (this->node_type == NODE_TYPE_ACTION
					|| this->node_type == NODE_TYPE_START) {
				guesses.back().push_back(predicted_score);
			}

			return NULL;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
		double jump_score;
		double jump_misguess;
		activate_explore_jump_network(problem,
									  state_vals,
									  states_on,
									  false,
									  network_historys,
									  jump_score,
									  jump_misguess);
		double pinned_jump_score = max(min(jump_score, 1.0), 0.0);
		double jump_combined = pinned_jump_score - jump_misguess;

		double no_jump_score;
		double no_jump_misguess;
		activate_score_network(problem,
							   state_vals,
							   states_on,
							   false,
							   network_historys,
							   no_jump_score,
							   no_jump_misguess);
		double pinned_no_jump_score = max(min(no_jump_score, 1.0), 0.0);
		double no_jump_combined = pinned_no_jump_score - no_jump_misguess;

		if (jump_combined > no_jump_combined) {
			if (rand()%2 == 0) {
				if (this->node_type == NODE_TYPE_ACTION
						|| this->node_type == NODE_TYPE_START) {
					guesses.back().push_back(jump_score);
				}
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
				if (this->explore_path.size() > 0) {
					return this->explore_path[0];
				} else {
					return this->explore_end_non_inclusive;
				}
			} else {
				if (this->node_type == NODE_TYPE_ACTION
						|| this->node_type == NODE_TYPE_START) {
					guesses.back().push_back(no_jump_score);
				}
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
				return NULL;
			}
		} else {
			if (this->node_type == NODE_TYPE_ACTION
					|| this->node_type == NODE_TYPE_START) {
				guesses.back().push_back(no_jump_score);
			}
			explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);
			return NULL;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
		// if loop_scopes.back() == NULL, scale by number of loops
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
			nodes_visited[n_index]->fold_helpers[this]->process(
				flat_inputs,
				activated,
				fold_loop_scope_counts);
		}

		// call potential state networks of nodes if needed

		

		if (loop_scopes.back() == NULL) {
			int current_count = loop_scope_counts.back();
			if (rand()%(6-current_count) == 0) {
				double predicted_score;
				double predicted_misguess;
				activate_explore_halt_network(problem,
											  state_vals,
											  states_on,
											  potential_state_vals,
											  true,
											  network_historys,
											  predicted_score,
											  predicted_misguess);

				loop_scopes.pop_back();
				loop_scope_counts.pop_back();

				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(predicted_score);
				}
				return NULL;
			} else {
				double predicted_score;
				double predicted_misguess;
				activate_explore_no_halt_network(problem,
												 state_vals,
												 states_on,
												 potential_state_vals,
												 true,
												 network_historys,
												 predicted_score,
												 predicted_misguess);

				loop_scope_counts.back()++;

				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(predicted_score);
				}
				return this->explore_start_inclusive;
			}
		} else {
			if (is_first_explore || rand()%20 == 0) {
				double predicted_score;
				double predicted_misguess;
				activate_explore_no_halt_network(problem,
												 state_vals,
												 states_on,
												 potential_state_vals,
												 true,
												 network_historys,
												 predicted_score,
												 predicted_misguess);

				for (int ps_index = 0; ps_index < 2; ps_index++) {
					potential_state_vals[ps_index] = 0.0;
				}

				loop_scopes.push_back(NULL);
				loop_scope_counts.push_back(1);

				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(predicted_score);
				}
				return this->explore_start_inclusive;
			} else {
				double predicted_score;
				double predicted_misguess;
				activate_explore_halt_network(problem,
											  state_vals,
											  states_on,
											  potential_state_vals,
											  true,
											  network_historys,
											  predicted_score,
											  predicted_misguess);
				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(predicted_score);
				}
				return NULL;
			}
		}
	} else {
		// this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP
		if (loop_scopes.back() == NULL) {
			explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);

			if (loop_scope_counts.back() >= 20) {
				double predicted_score;
				double predicted_misguess;
				activate_explore_halt_network(problem,
											  state_vals,
											  states_on,
											  potential_state_vals,
											  false,
											  network_historys,
											  predicted_score,
											  predicted_misguess);
				loop_scopes.pop_back();
				loop_scope_counts.pop_back();

				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(predicted_score);
				}
				return NULL;
			}

			double halt_score;
			double halt_misguess;
			activate_explore_halt_network(problem,
										  state_vals,
										  states_on,
										  potential_state_vals,
										  false,
										  network_historys,
										  halt_score,
										  halt_misguess);
			double pinned_halt_score = max(min(halt_score, 1.0), 0.0);
			double halt_combined = pinned_halt_score - halt_misguess;

			double no_halt_score;
			double no_halt_misguess;
			activate_explore_no_halt_network(problem,
											 state_vals,
											 states_on,
											 potential_state_vals,
											 false,
											 network_historys,
											 no_halt_score,
											 no_halt_misguess);
			double pinned_no_halt_score = max(min(no_halt_score, 1.0), 0.0);
			double no_halt_combined = pinned_no_halt_score - no_halt_misguess;

			if (halt_combined > no_halt_combined) {
				loop_scopes.pop_back();
				loop_scope_counts.pop_back();

				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(halt_score);
				}
				return NULL;
			} else {
				loop_scope_counts.back()++;

				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(no_halt_score);
				}
				return this->explore_start_inclusive;
			}
		} else {
			double explore_score;
			double explore_misguess;
			activate_explore_no_halt_network(problem,
											 state_vals,
											 states_on,
											 potential_state_vals,
											 false,
											 network_historys,
											 explore_score,
											 explore_misguess);
			double pinned_explore_score = max(min(explore_score, 1.0), 0.0);
			double explore_combined = pinned_explore_score - explore_misguess;

			double no_explore_score;
			double no_explore_misguess;
			activate_score_network(problem,
								   state_vals,
								   states_on,
								   false,
								   network_historys,
								   no_explore_score,
								   no_explore_misguess);
			double pinned_no_explore_score = max(min(no_explore_score, 1.0), 0.0);
			double no_explore_combined = pinned_no_explore_score - no_explore_misguess;

			if (no_explore_combined > explore_combined) {
				if (this->node_type == NODE_TYPE_ACTION) {
					guesses.back().push_back(no_explore_score);
				}
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);
				return NULL;
			} else {
				if (rand()%2 == 0) {
					for (int ps_index = 0; ps_index < 2; ps_index++) {
						potential_state_vals[ps_index] = 0.0;
					}

					loop_scopes.push_back(NULL);
					loop_scope_counts.push_back(1);

					if (this->node_type == NODE_TYPE_ACTION) {
						guesses.back().push_back(explore_score);
					}
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
					return this->explore_start_inclusive;
				} else {
					if (this->node_type == NODE_TYPE_ACTION) {
						guesses.back().push_back(no_explore_score);
					}
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
			if (network_history->network == this->explore_jump_certainty_network) {
				backprop_explore_jump_network(score,
											  misguess,
											  state_errors,
											  states_on,
											  network_historys);
			} else {
				backprop_score_network_errors_with_no_weight_change(
					score,
					misguess,
					state_errors,
					states_on,
					network_historys);
			}
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
			if (explore_decisions.back() == EXPLORE_DECISION_TYPE_EXPLORE) {
				this->explore_explore_measure_count++;
				if (score == 1.0) {
					this->explore_explore_is_good += 1;
				} else {
					this->explore_explore_is_bad += 1;
				}
				this->explore_explore_misguess += misguess;
			} else if (explore_decisions.back() == EXPLORE_DECISION_TYPE_NO_EXPLORE) {
				this->explore_no_explore_measure_count++;
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
			if (network_history->network == this->explore_halt_certainty_network) {
				for (int ps_index = 0; ps_index < 2; ps_index++) {
					potential_state_errors[ps_index] = 0.0;
				}

				backprop_explore_halt_network(score,
											  misguess,
											  potential_state_errors,
											  network_historys);
			} else {
				backprop_explore_no_halt_network(score,
												 misguess,
												 potential_state_errors,
												 network_historys);
			}
		} else {
			// this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP
			if (explore_decisions.back() == EXPLORE_DECISION_TYPE_EXPLORE) {
				this->explore_explore_measure_count++;
				if (score == 1.0) {
					this->explore_explore_is_good += 1;
				} else {
					this->explore_explore_is_bad += 1;
				}
				this->explore_explore_misguess += misguess;
			} else if (explore_decisions.back() == EXPLORE_DECISION_TYPE_NO_EXPLORE) {
				this->explore_no_explore_measure_count++;
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
					this->explore_path.push_back(curr_node);
					for (int a_index = 1; a_index < (int)iter_explore->try_path.size(); a_index++) {
						SolutionNodeAction* next_node = new SolutionNodeAction(
							this->solution,
							-1,
							iter_explore->try_path[a_index],
							iter_explore->available_state);
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
				this->explore_jump_score_network = new Network(input_size,
														 4*input_size,
														 1);
				this->explore_jump_certainty_network = new Network(input_size,
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
				this->explore_halt_score_network = new Network(input_size,
															   4*input_size,
															   1);
				this->explore_halt_certainty_network = new Network(input_size,
																   4*input_size,
																   1);
				this->explore_no_halt_score_network = new Network(input_size,
																  4*input_size,
																  1);
				this->explore_no_halt_certainty_network = new Network(input_size,
																	  4*input_size,
																	  1);

				this->explore_path_state = EXPLORE_PATH_STATE_LEARN_LOOP;
				this->explore_path_iter_index = 0;
			}
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_JUMP) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 2000000) {
		if (this->explore_path_iter_index > 10000000) {
			this->explore_path_state = EXPLORE_PATH_STATE_MEASURE_JUMP;
			this->explore_path_iter_index = 0;

			this->explore_explore_measure_count = 0;
			this->explore_explore_is_good = 0.0;
			this->explore_explore_is_bad = 0.0;
			this->explore_explore_misguess = 0.0;
			this->explore_no_explore_measure_count = 0;
			this->explore_no_explore_is_good = 0.0;
			this->explore_no_explore_is_bad = 0.0;
			this->explore_no_explore_misguess = 0.0;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_JUMP) {
		this->explore_path_iter_index++;

		if (this->explore_path_iter_index > 100000) {
			// double average_explore_is_good = (double)this->explore_explore_is_good/this->explore_explore_measure_count;
			// double average_explore_is_bad = (double)this->explore_explore_is_bad/this->explore_explore_measure_count;
			// double average_explore_misguess = this->explore_explore_misguess/this->explore_explore_measure_count;
			// double average_no_explore_is_good = (double)this->explore_no_explore_is_good/this->explore_no_explore_measure_count;
			// double average_no_explore_is_bad = (double)this->explore_no_explore_is_bad/this->explore_no_explore_measure_count;
			// double average_no_explore_misguess = this->explore_no_explore_misguess/this->explore_no_explore_measure_count;

			double improvement = explore_explore_is_good - explore_no_explore_is_good;
			// TODO: add misguess
			if (improvement > 0.0) {
				if (this->node_type == NODE_TYPE_IF_START
						&& ((SolutionNodeIfStart*)this)->explore_child_index == -1) {
					this->solution->nodes_mtx.lock();

					vector<int> new_path_node_indexes;
					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						int new_index = (int)this->solution->nodes.size();

						this->explore_path[n_index]->node_index = new_index;

						this->solution->nodes.push_back(this->explore_path[n_index]);
						new_path_node_indexes.push_back(new_index);
					}
					this->explore_path.clear();

					this->solution->nodes_mtx.unlock();

					SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
					this_if_start->children_nodes.push_back(NULL);
					while (this->explore_jump_score_network->input->acti_vals.size()
							< 1 + this->network_inputs_state_indexes.size()) {
						this->explore_jump_score_network->pad_input();
						this->explore_jump_certainty_network->pad_input();
					}
					this_if_start->children_score_networks.push_back(this->explore_jump_score_network);
					this->explore_jump_score_network = NULL;
					this_if_start->children_certainty_networks.push_back(this->explore_jump_certainty_network);
					this->explore_jump_certainty_network = NULL;
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

						this->explore_path[n_index]->node_index = new_index;

						this->solution->nodes.push_back(this->explore_path[n_index]);
						new_path_node_indexes.push_back(new_index);
					}
					this->explore_path.clear();

					int new_end_node_index = (int)this->solution->nodes.size();
					SolutionNodeIfEnd* new_end_node = new SolutionNodeIfEnd(
						this,
						new_end_node_index);
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
					}
					this->solution->explore->mtx.lock();
					this->solution->explore->current_node->children.push_back(new_explore_node);
					this->solution->explore->mtx.unlock();
					cout << "new ExploreNodeNewJump" << endl;
				}
			} else {
				delete this->explore_jump_score_network;
				this->explore_jump_score_network = NULL;
				delete this->explore_jump_certainty_network;
				this->explore_jump_certainty_network = NULL;

				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					delete this->explore_path[n_index];
				}
				this->explore_path.clear();
			}

			this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN_LOOP) {
		this->explore_path_iter_index++;

		if (this->explore_path_iter_index > 2000000) {
			this->explore_path_state = EXPLORE_PATH_STATE_MEASURE_LOOP;
			this->explore_path_iter_index = 0;

			this->explore_explore_measure_count = 0;
			this->explore_explore_is_good = 0.0;
			this->explore_explore_is_bad = 0.0;
			this->explore_explore_misguess = 0.0;
			this->explore_no_explore_measure_count = 0;
			this->explore_no_explore_is_good = 0.0;
			this->explore_no_explore_is_bad = 0.0;
			this->explore_no_explore_misguess = 0.0;
		}
	} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE_LOOP) {
		this->explore_path_iter_index++;

		if (this->explore_path_iter_index > 100000) {
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

				this->solution->nodes_mtx.lock();

				int new_start_node_index = (int)this->solution->nodes.size();
				SolutionNodeLoopStart* new_start_node = new SolutionNodeLoopStart(
					this,
					new_start_node_index,
					new_state_indexes);
				this->solution->nodes.push_back(new_start_node);

				int new_end_node_index = (int)this->solution->nodes.size();
				SolutionNodeLoopEnd* new_end_node = new SolutionNodeLoopEnd(
					this,
					new_end_node_index,
					new_state_indexes);
				// takes and clears explore networks
				this->solution->nodes.push_back(new_end_node);

				this->solution->nodes_mtx.unlock();

				this->explore_start_inclusive->extend_with_potential_state(
					this->explore_loop_states,
					new_state_indexes,
					this);
				this->explore_loop_states.clear();

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
				delete this->explore_halt_score_network;
				this->explore_halt_score_network = NULL;
				delete this->explore_halt_certainty_network;
				this->explore_halt_certainty_network = NULL;
				delete this->explore_no_halt_score_network;
				this->explore_no_halt_score_network = NULL;
				delete this->explore_no_halt_certainty_network;
				this->explore_no_halt_certainty_network = NULL;

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

	if (this->explore_jump_score_network != NULL) {
		delete this->explore_jump_score_network;
		this->explore_jump_score_network = NULL;
	}
	if (this->explore_jump_certainty_network != NULL) {
		delete this->explore_jump_certainty_network;
		this->explore_jump_certainty_network = NULL;
	}
	if (this->explore_halt_score_network != NULL) {
		delete this->explore_halt_score_network;
		this->explore_halt_score_network = NULL;
	}
	if (this->explore_halt_certainty_network != NULL) {
		delete this->explore_halt_certainty_network;
		this->explore_halt_certainty_network = NULL;
	}
	if (this->explore_no_halt_score_network != NULL) {
		delete this->explore_no_halt_score_network;
		this->explore_no_halt_score_network = NULL;
	}
	if (this->explore_no_halt_certainty_network != NULL) {
		delete this->explore_no_halt_certainty_network;
		this->explore_no_halt_certainty_network = NULL;
	}

	this->explore_loop_states.clear();
}

void SolutionNode::update_node_weight(double new_node_weight) {
	this->node_weight = 0.9999*this->node_weight + 0.0001*new_node_weight;
}

void SolutionNode::score_network_add_potential_state(vector<int> potential_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->network_inputs_potential_state_indexes.push_back(
			potential_state_indexes[ps_index]);
		this->score_network->add_potential();
		this->certainty_network->add_potential();
	}
}

void SolutionNode::score_network_extend_with_potential_state(vector<int> potential_state_indexes,
															 vector<int> new_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->network_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				this->score_network->extend_with_potential(pi_index);
				this->certainty_network->extend_with_potential(pi_index);
				this->network_inputs_potential_state_indexes.erase(
					this->network_inputs_potential_state_indexes.begin() + pi_index);
				break;
			}
		}
	}
}

void SolutionNode::score_network_delete_potential_state(vector<int> potential_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->score_network->delete_potential(pi_index);
				this->certainty_network->delete_potential(pi_index);
				this->network_inputs_potential_state_indexes.erase(
					this->network_inputs_potential_state_indexes.begin() + pi_index);
				break;
			}
		}
	}
}

void SolutionNode::score_network_clear_potential_state() {
	this->network_inputs_potential_state_indexes.clear();
	this->score_network->remove_potentials();
	this->certainty_network->remove_potentials();
}

void SolutionNode::load_score_network(ifstream& save_file) {
	string network_inputs_state_indexes_size_line;
	getline(save_file, network_inputs_state_indexes_size_line);
	int network_inputs_state_indexes_size = stoi(network_inputs_state_indexes_size_line);
	for (int s_index = 0; s_index < network_inputs_state_indexes_size; s_index++) {
		string state_index_line;
		getline(save_file, state_index_line);
		this->network_inputs_state_indexes.push_back(stoi(state_index_line));
	}

	string score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network = new Network(score_network_save_file);
	score_network_save_file.close();

	string certainty_network_name = "../saves/nns/certainty_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ifstream certainty_network_save_file;
	certainty_network_save_file.open(certainty_network_name);
	this->certainty_network = new Network(certainty_network_save_file);
	certainty_network_save_file.close();

	string node_weight_line;
	getline(save_file, node_weight_line);
	this->node_weight = stof(node_weight_line);
}

void SolutionNode::save_score_network(ofstream& save_file) {
	save_file << this->network_inputs_state_indexes.size() << endl;
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		save_file << this->network_inputs_state_indexes[i_index] << endl;
	}

	string score_network_name = "../saves/nns/score_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream score_network_save_file;
	score_network_save_file.open(score_network_name);
	this->score_network->save(score_network_save_file);
	score_network_save_file.close();

	string certainty_network_name = "../saves/nns/certainty_" + to_string(this->node_index) \
		+ "_" + to_string(this->solution->id) + ".txt";
	ofstream certainty_network_save_file;
	certainty_network_save_file.open(certainty_network_name);
	this->certainty_network->save(certainty_network_save_file);
	certainty_network_save_file.close();

	save_file << this->node_weight << endl;
}

void SolutionNode::activate_score_network(Problem& problem,
										  double* state_vals,
										  bool* states_on,
										  bool backprop,
										  vector<NetworkHistory*>& network_historys,
										  double& predicted_score,
										  double& predicted_misguess) {
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

	if (backprop) {
		this->score_network->mtx.lock();
		this->score_network->activate(inputs, network_historys);
		predicted_score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();

		this->certainty_network->mtx.lock();
		this->certainty_network->activate(inputs, network_historys);
		predicted_misguess = this->certainty_network->output->acti_vals[0];
		this->certainty_network->mtx.unlock();
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(inputs);
		predicted_score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();

		this->certainty_network->mtx.lock();
		this->certainty_network->activate(inputs);
		predicted_misguess = this->certainty_network->output->acti_vals[0];
		this->certainty_network->mtx.unlock();
	}
}

void SolutionNode::backprop_score_network(double score,
										  double misguess,
										  double* state_errors,
										  bool* states_on,
										  vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	this->certainty_network->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->certainty_network->output->acti_vals[0]);
	this->certainty_network->backprop(certainty_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->certainty_network->input->errors[1+i_index];
		}
		this->certainty_network->input->errors[1+i_index] = 0.0;
	}

	this->certainty_network->mtx.unlock();

	delete certainty_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->score_network->mtx.lock();

	score_network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->score_network->backprop(score_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}

void SolutionNode::backprop_score_network_errors_with_no_weight_change(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	this->certainty_network->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->certainty_network->output->acti_vals[0]);
	this->certainty_network->backprop_errors_with_no_weight_change(certainty_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->certainty_network->input->errors[1+i_index];
		}
		this->certainty_network->input->errors[1+i_index] = 0.0;
	}

	this->certainty_network->mtx.unlock();

	delete certainty_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->score_network->mtx.lock();

	score_network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->score_network->backprop_errors_with_no_weight_change(score_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}

void SolutionNode::activate_score_network_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		vector<int>& potential_state_indexes,
		bool backprop,
		vector<NetworkHistory*>& network_historys,
		double& predicted_score,
		double& predicted_misguess) {
	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->network_inputs_potential_state_indexes.size(); p_index++) {
		if (potential_state_indexes[0] == this->network_inputs_potential_state_indexes[p_index]) {
			for (int i = 0; i < 2; i++) {
				potentials_on.push_back(p_index+i);
				potential_vals.push_back(potential_state_vals[i]);
			}
			break;
		}
	}

	if (potentials_on.size() == 0) {
		activate_score_network(problem,
							   state_vals,
							   states_on,
							   false,
							   network_historys,
							   predicted_score,
							   predicted_misguess);
		return;
	}

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

	if (backprop) {
		this->score_network->mtx.lock();
		this->score_network->activate(inputs,
									  potentials_on,
									  potential_vals,
									  network_historys);
		predicted_score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();

		this->certainty_network->mtx.lock();
		this->certainty_network->activate(inputs,
										  potentials_on,
										  potential_vals,
										  network_historys);
		predicted_misguess = this->certainty_network->output->acti_vals[0];
		this->certainty_network->mtx.unlock();
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(inputs,
									  potentials_on,
									  potential_vals);
		predicted_score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();

		this->certainty_network->mtx.lock();
		this->certainty_network->activate(inputs,
										  potentials_on,
										  potential_vals);
		predicted_misguess = this->certainty_network->output->acti_vals[0];
		this->certainty_network->mtx.unlock();
	}
}

void SolutionNode::backprop_score_network_with_potential(
		double score,
		double misguess,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	if (network_historys.size() > 0) {
		NetworkHistory* certainty_network_history = network_historys.back();
		if (certainty_network_history->network == this->certainty_network) {
			this->certainty_network->mtx.lock();

			certainty_network_history->reset_weights();

			vector<int> certainty_potentials_on = certainty_network_history->potentials_on;

			vector<double> certainty_errors;
			certainty_errors.push_back(misguess - this->certainty_network->output->acti_vals[0]);
			this->certainty_network->backprop(certainty_errors, certainty_potentials_on);

			for (int o_index = 0; o_index < (int)certainty_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->certainty_network->potential_inputs[
					certainty_potentials_on[o_index]]->errors[0];
				this->certainty_network->potential_inputs[certainty_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->certainty_network->mtx.unlock();

			delete certainty_network_history;
			network_historys.pop_back();

			NetworkHistory* score_network_history = network_historys.back();

			this->score_network->mtx.lock();

			score_network_history->reset_weights();

			vector<int> score_potentials_on = score_network_history->potentials_on;

			vector<double> score_errors;
			if (score == 1.0) {
				if (this->score_network->output->acti_vals[0] < 1.0) {
					score_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			} else {
				if (this->score_network->output->acti_vals[0] > 0.0) {
					score_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
				} else {
					score_errors.push_back(0.0);
				}
			}
			this->score_network->backprop(score_errors, score_potentials_on);

			for (int o_index = 0; o_index < (int)score_potentials_on.size(); o_index++) {
				potential_state_errors[o_index] += this->score_network->potential_inputs[
					score_potentials_on[o_index]]->errors[0];
				this->score_network->potential_inputs[score_potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->score_network->mtx.unlock();

			delete score_network_history;
			network_historys.pop_back();
		}
	}
}

void SolutionNode::activate_explore_jump_network(Problem& problem,
												 double* state_vals,
												 bool* states_on,
												 bool backprop,
												 vector<NetworkHistory*>& network_historys,
												 double& predicted_score,
												 double& predicted_misguess) {
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

	if (backprop) {
		this->explore_jump_score_network->mtx.lock();
		this->explore_jump_score_network->activate(explore_network_inputs, network_historys);
		predicted_score = this->explore_jump_score_network->output->acti_vals[0];
		this->explore_jump_score_network->mtx.unlock();

		this->explore_jump_certainty_network->mtx.lock();
		this->explore_jump_certainty_network->activate(explore_network_inputs, network_historys);
		predicted_misguess = this->explore_jump_certainty_network->output->acti_vals[0];
		this->explore_jump_certainty_network->mtx.unlock();
	} else {
		this->explore_jump_score_network->mtx.lock();
		this->explore_jump_score_network->activate(explore_network_inputs);
		predicted_score = this->explore_jump_score_network->output->acti_vals[0];
		this->explore_jump_score_network->mtx.unlock();

		this->explore_jump_certainty_network->mtx.lock();
		this->explore_jump_certainty_network->activate(explore_network_inputs);
		predicted_misguess = this->explore_jump_certainty_network->output->acti_vals[0];
		this->explore_jump_certainty_network->mtx.unlock();
	}
}

void SolutionNode::backprop_explore_jump_network(double score,
												 double misguess,
												 double* state_errors,
												 bool* states_on,
												 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	this->explore_jump_certainty_network->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->explore_jump_certainty_network->output->acti_vals[0]);
	this->explore_jump_certainty_network->backprop(certainty_errors);

	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			state_errors[this->explore_network_inputs_state_indexes[i_index]] += \
				this->explore_jump_certainty_network->input->errors[1+i_index];
		}
		this->explore_jump_certainty_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_jump_certainty_network->mtx.unlock();

	delete certainty_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->explore_jump_score_network->mtx.lock();

	score_network_history->reset_weights();

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
	this->explore_jump_score_network->backprop(score_errors);

	for (int i_index = 0; i_index < (int)this->explore_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->explore_network_inputs_state_indexes[i_index]]) {
			state_errors[this->explore_network_inputs_state_indexes[i_index]] += \
				this->explore_jump_score_network->input->errors[1+i_index];
		}
		this->explore_jump_score_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_jump_score_network->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}

void SolutionNode::activate_explore_halt_network(Problem& problem,
												 double* state_vals,
												 bool* states_on,
												 double* potential_state_vals,
												 bool backprop,
												 vector<NetworkHistory*>& network_historys,
												 double& predicted_score,
												 double& predicted_misguess) {
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

	if (backprop) {
		this->explore_halt_score_network->mtx.lock();
		this->explore_halt_score_network->activate(explore_network_inputs, network_historys);
		predicted_score = this->explore_halt_score_network->output->acti_vals[0];
		this->explore_halt_score_network->mtx.unlock();

		this->explore_halt_certainty_network->mtx.lock();
		this->explore_halt_certainty_network->activate(explore_network_inputs, network_historys);
		predicted_misguess = this->explore_halt_certainty_network->output->acti_vals[0];
		this->explore_halt_certainty_network->mtx.unlock();
	} else {
		this->explore_halt_score_network->mtx.lock();
		this->explore_halt_score_network->activate(explore_network_inputs);
		predicted_score = this->explore_halt_score_network->output->acti_vals[0];
		this->explore_halt_score_network->mtx.unlock();

		this->explore_halt_certainty_network->mtx.lock();
		this->explore_halt_certainty_network->activate(explore_network_inputs);
		predicted_misguess = this->explore_halt_certainty_network->output->acti_vals[0];
		this->explore_halt_certainty_network->mtx.unlock();
	}
}

void SolutionNode::backprop_explore_halt_network(double score,
												 double misguess,
												 double* potential_state_errors,
												 vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	this->explore_halt_certainty_network->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->explore_halt_certainty_network->output->acti_vals[0]);
	this->explore_halt_certainty_network->backprop(certainty_errors);

	for (int ps_index = 0; ps_index < 2; ps_index++) {
		potential_state_errors[ps_index] += this->explore_halt_certainty_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index];
		this->explore_halt_certainty_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index] = 0.0;
	}

	this->explore_halt_certainty_network->mtx.unlock();

	delete certainty_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->explore_halt_score_network->mtx.lock();

	score_network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_halt_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_halt_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_halt_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_halt_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_halt_score_network->backprop(score_errors);

	for (int ps_index = 0; ps_index < 2; ps_index++) {
		potential_state_errors[ps_index] += this->explore_halt_score_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index];
		this->explore_halt_score_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index] = 0.0;
	}

	this->explore_halt_score_network->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}

void SolutionNode::activate_explore_no_halt_network(Problem& problem,
													double* state_vals,
													bool* states_on,
													double* potential_state_vals,
													bool backprop,
													vector<NetworkHistory*>& network_historys,
													double& predicted_score,
													double& predicted_misguess) {
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

	if (backprop) {
		this->explore_no_halt_score_network->mtx.lock();
		this->explore_no_halt_score_network->activate(explore_network_inputs, network_historys);
		predicted_score = this->explore_no_halt_score_network->output->acti_vals[0];
		this->explore_no_halt_score_network->mtx.unlock();

		this->explore_no_halt_certainty_network->mtx.lock();
		this->explore_no_halt_certainty_network->activate(explore_network_inputs, network_historys);
		predicted_misguess = this->explore_no_halt_certainty_network->output->acti_vals[0];
		this->explore_no_halt_certainty_network->mtx.unlock();
	} else {
		this->explore_no_halt_score_network->mtx.lock();
		this->explore_no_halt_score_network->activate(explore_network_inputs);
		predicted_score = this->explore_no_halt_score_network->output->acti_vals[0];
		this->explore_no_halt_score_network->mtx.unlock();

		this->explore_no_halt_certainty_network->mtx.lock();
		this->explore_no_halt_certainty_network->activate(explore_network_inputs);
		predicted_misguess = this->explore_no_halt_certainty_network->output->acti_vals[0];
		this->explore_no_halt_certainty_network->mtx.unlock();
	}
}

void SolutionNode::backprop_explore_no_halt_network(double score,
													double misguess,
													double* potential_state_errors,
													std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* certainty_network_history = network_historys.back();

	this->explore_no_halt_score_network->mtx.lock();

	certainty_network_history->reset_weights();

	vector<double> certainty_errors;
	certainty_errors.push_back(misguess - this->explore_no_halt_certainty_network->output->acti_vals[0]);
	this->explore_no_halt_certainty_network->backprop(certainty_errors);

	for (int ps_index = 0; ps_index < 2; ps_index++) {
		potential_state_errors[ps_index] += this->explore_no_halt_certainty_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index];
		this->explore_no_halt_certainty_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index] = 0.0;
	}

	this->explore_no_halt_certainty_network->mtx.unlock();

	delete certainty_network_history;
	network_historys.pop_back();

	NetworkHistory* score_network_history = network_historys.back();

	this->explore_no_halt_score_network->mtx.unlock();

	score_network_history->reset_weights();

	vector<double> score_errors;
	if (score == 1.0) {
		if (this->explore_no_halt_score_network->output->acti_vals[0] < 1.0) {
			score_errors.push_back(1.0 - this->explore_no_halt_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	} else {
		if (this->explore_no_halt_score_network->output->acti_vals[0] > 0.0) {
			score_errors.push_back(0.0 - this->explore_no_halt_score_network->output->acti_vals[0]);
		} else {
			score_errors.push_back(0.0);
		}
	}
	this->explore_no_halt_score_network->backprop(score_errors);

	for (int ps_index = 0; ps_index < 2; ps_index++) {
		potential_state_errors[ps_index] += this->explore_no_halt_score_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index];
		this->explore_no_halt_score_network->input->errors[
			1 + (int)this->explore_network_inputs_state_indexes.size() + ps_index] = 0.0;
	}

	this->explore_no_halt_score_network->mtx.unlock();

	delete score_network_history;
	network_historys.pop_back();
}
