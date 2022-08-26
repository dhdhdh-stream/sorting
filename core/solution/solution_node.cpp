#include "solution_node.h"

#include <cmath>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "explore_node_state.h"
#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"

using namespace std;

SolutionNode::~SolutionNode() {
	// do nothing
}

double SolutionNode::activate_score_network_helper(Problem& problem,
												   double* state_vals,
												   bool* states_on,
												   std::vector<SolutionNode*>& loop_scopes,
												   std::vector<int>& loop_scope_counts,
												   int& iter_explore_type,
												   SolutionNode*& iter_explore_node,
												   double* potential_state_vals,
												   bool* potential_states_on,
												   vector<NetworkHistory*>& network_historys,
												   vector<double>& guesses) {
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		double score = activate_score_network(problem,
											  state_vals,
											  states_on,
											  true,
											  network_historys);
		guesses.push_back(score);
		return score;
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// do nothing
		return 0.0; 
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		if (iter_explore_node == this) {
			double score = activate_score_network(problem,
												  state_vals,
												  states_on,
												  false,
												  network_historys);
			return score;
		} else {
			// do nothing
			return 0.0;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		if (iter_explore_node == this
				&& this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
			if (loop_scopes.size() > 0 && loop_scopes.back() == NULL) {
				// do nothing
				return 0.0;
			} else {
				double score = activate_score_network(problem,
													  state_vals,
													  states_on,
													  false,
													  network_historys);
				return score;
			}
		} else {
			double score = activate_score_network(problem,
												  state_vals,
												  states_on,
												  true,
												  network_historys);
			return score;
		} 
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		double score = activate_score_network_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_states_on,
			true,
			network_historys);
		return score;
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		double score = activate_score_network(problem,
											  state_vals,
											  states_on,
											  false,
											  network_historys);
		if (iter_explore_node != this) {
			guesses.push_back(score);
		}
		return score;
	} else {
		// iter_explore_type == EXPLORE_TYPE_MEASURE_STATE
		double score = activate_score_network_with_potential(
			problem,
			state_vals,
			states_on,
			potential_state_vals,
			potential_states_on,
			false,
			network_historys);
		guesses.push_back(score);
		return score;
	}
}

SolutionNode* SolutionNode::explore(double score,
									Problem& problem,
									double* state_vals,
									bool* states_on,
									vector<SolutionNode*>& loop_scopes,
									vector<int>& loop_scope_counts,
									int& iter_explore_type,
									SolutionNode*& iter_explore_node,
									double* potential_state_errors,
									bool* potential_states_on,
									vector<NetworkHistory*>& network_historys,
									vector<double>& guesses,
									vector<int>& explore_decisions,
									vector<double>& explore_diffs) {
	if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		// do nothing
		return NULL;
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		// do nothing
		return NULL;
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
			bool do_explore = false;
			if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
				if (score > this->average_score) {
					do_explore = true;
				}
			} else {
				if (score < this->average_score) {
					do_explore = true;
				}
			}

			if (do_explore) {
				for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
					problem.perform_action(try_path[a_index]);
				}
				this->explore_path_used = true;

				return this->explore_end_non_inclusive;
			} else {
				return NULL;
			}
		} else {
			// this->path_explore_type == PATH_EXPLORE_TYPE_LOOP
			if (loop_scopes.size() > 0 && loop_scopes.back() == NULL) {
				if ((loop_scope_counts.back() == 3 && rand()%3 == 0)
						|| (loop_scope_counts.back() == 4 && rand()%2 == 0)
						|| loop_scope_counts.back() == 5) {
					loop_scopes.pop_back();
					loop_scope_counts.pop_back();

					return NULL;
				} else {
					loop_scope_counts.back()++;

					return this->explore_start_inclusive;
				}
			} else {
				bool do_explore = false;
				if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
					if (score > this->average_score) {
						do_explore = true;
					}
				} else {
					if (score < this->average_score) {
						do_explore = true;
					}
				}

				if (do_explore) {
					loop_scopes.push_back(NULL);
					loop_scope_counts.push_back(1);
					this->explore_path_used = true;

					return this->explore_start_inclusive;
				} else {
					return NULL;
				}
			}
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
			bool do_explore = false;
			if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
				if (score > this->average_score) {
					do_explore = true;
				}
			} else {
				if (score < this->average_score) {
					do_explore = true;
				}
			}

			if (do_explore || rand()%20 == 0) {
				// remove score_network history
				delete network_historys.back();
				network_historys.pop_back();

				activate_explore_if_network(problem,
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
				return NULL;
			}
		} else {
			// this->path_explore_type == PATH_EXPLORE_TYPE_LOOP
			if (loop_scopes.size() > 0 && loop_scopes.back() == NULL) {
				// score_network not run

				int current_count = loop_scope_counts.back();
				if (rand()%(6-current_count) == 0) {
					activate_explore_halt_network(problem,
												  state_vals,
												  states_on,
												  true,
												  network_historys);

					loop_scopes.pop_back();
					loop_scope_counts.pop_back();

					return NULL;
				} else {
					activate_explore_no_halt_network(problem,
													 state_vals,
													 states_on,
													 true,
													 network_historys);

					loop_scope_counts.back()++;

					return this->explore_start_inclusive;
				}
			} else {
				bool do_explore = false;
				if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
					if (score > this->average_score) {
						do_explore = true;
					}
				} else {
					if (score < this->average_score) {
						do_explore = true;
					}
				}

				if (do_explore || rand()%20 == 0) {
					// score_network history not saved

					activate_explore_no_halt_network(problem,
													 state_vals,
													 states_on,
													 true,
													 network_historys);

					loop_scopes.push_back(NULL);
					loop_scope_counts.push_back(1);

					return this->explore_start_inclusive;
				} else {
					// score_network history not saved

					activate_explore_no_halt_network(problem,
													 state_vals,
													 states_on,
													 true,
													 network_historys);

					return NULL;
				}
			}
		}
	} else {
		// iter_explore_type == EXPLORE_TYPE_MEASURE_PATH
		if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
			double explore_score = activate_explore_if_network(
				problem,
				state_vals,
				states_on,
				false,
				network_historys);
			if (explore_score > score) {
				explore_diffs.push_back(explore_score - score);
				if (rand()%2 == 0) {
					guesses.push_back(explore_score);
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
					if (this->explore_path.size() > 0) {
						return this->explore_path[0];
					} else {
						return this->explore_end_non_inclusive;
					}
				} else {
					guesses.push_back(score);
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
					return NULL;
				}
			} else {
				explore_diffs.push_back(0.0);
				guesses.push_back(score);
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);
				return NULL;
			}
		} else {
			// this->path_explore_type == PATH_EXPLORE_TYPE_LOOP
			if (loop_scopes.size() > 0 && loop_scopes.back() == NULL) {
				explore_diffs.push_back(0.0);
				explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);

				if (loop_scope_counts.back() >= 20) {
					double halt_score = activate_explore_halt_network(
						problem,
						state_vals,
						states_on,
						false,
						network_historys);
					
					loop_scopes.pop_back();
					loop_scope_counts.pop_back();

					guesses.push_back(halt_score);
					return NULL;
				}

				double halt_score = activate_explore_halt_network(
					problem,
					state_vals,
					states_on,
					false,
					network_historys);
				double no_halt_score = activate_explore_no_halt_network(
					problem,
					state_vals,
					states_on,
					false,
					network_historys);
				if (halt_score > no_halt_score) {
					loop_scopes.pop_back();
					loop_scope_counts.pop_back();

					guesses.push_back(halt_score);
					return NULL;
				} else {
					loop_scope_counts.back()++;

					guesses.push_back(no_halt_score);
					return this->explore_start_inclusive;
				}
			} else {
				double halt_score = activate_explore_halt_network(
					problem,
					state_vals,
					states_on,
					false,
					network_historys);
				double no_halt_score = activate_explore_no_halt_network(
					problem,
					state_vals,
					states_on,
					false,
					network_historys);

				if (halt_score > no_halt_score) {
					explore_diffs.push_back(0.0);
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);

					guesses.push_back(halt_score);
					return NULL;
				} else {
					explore_diffs.push_back(no_halt_score - halt_score);
					if (rand()%2 == 0) {
						loop_scopes.push_back(NULL);
						loop_scope_counts.push_back(1);

						guesses.push_back(no_halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
						return this->explore_start_inclusive;
					} else {
						guesses.push_back(halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
						return NULL;
					}
				}
			}
		}
	}
}

void SolutionNode::backprop_explore_and_score_network_helper(
		double score,
		double misguess,
		double* state_errors,
		bool* states_on,
		int& iter_explore_type,
		SolutionNode*& iter_explore_node,
		double* potential_state_errors,
		bool* potential_states_on,
		vector<NetworkHistory*>& network_historys,
		vector<int>& explore_decisions,
		vector<double>& explore_diffs) {
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		if (this->node_type == NODE_TYPE_IF_START) {
			SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
			this_if_start->backprop_children_networks(score,
													  state_errors,
													  states_on,
													  network_historys);
		} else {
			backprop_score_network(score,
								   state_errors,
								   states_on,
								   network_historys);
		}

		// TODO: add certainty networks

		this->average_score = 0.9999*this->average_score + score;
		this->average_misguess = 0.9999*this->average_score + score;
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		if (this->temp_node_state == TEMP_NODE_STATE_LEARN) {
			backprop_score_network(score,
								   state_errors,
								   states_on,
								   network_historys);
		} else if (iter_explore_node == this) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				NetworkHistory* network_history = network_historys.back();
				if (network_history->network == this->explore_if_network) {
					backprop_explore_if_network(score,
												state_errors,
												states_on,
												network_historys);
				} else {
					if (this->node_type == NODE_TYPE_IF_START) {
						SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
						this_if_start->backprop_children_networks_errors_with_no_weight_change(
							score,
							state_errors,
							states_on,
							network_historys);
					} else {
						backprop_score_network_errors_with_no_weight_change(
							score,
							state_errors,
							states_on,
							network_historys);
					}
				}
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				NetworkHistory* network_history = network_historys.back();
				if (network_history->network == this->explore_halt_network) {
					backprop_explore_halt_network(score,
												  state_errors,
												  states_on,
												  network_historys);
				} else if (network_history->network == this->explore_no_halt_network) {
					backprop_explore_no_halt_network(score,
													 state_errors,
													 states_on,
													 network_historys);
				} else {
					// NODE_TYPE_IF_START cannot have PATH_EXPLORE_TYPE_LOOP
					backprop_score_network_errors_with_no_weight_change(
						score,
						state_errors,
						states_on,
						network_historys);
				}
			}
		} else {
			if (this->node_type == NODE_TYPE_IF_START) {
				SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
				this_if_start->backprop_children_networks_errors_with_no_weight_change(
					score,
					state_errors,
					states_on,
					network_historys);
			} else {
				backprop_score_network_errors_with_no_weight_change(
					score,
					state_errors,
					states_on,
					network_historys);
			}
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		if (this->node_type == NODE_TYPE_IF_START) {
			SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
			this_if_start->backprop_children_networks_with_potential(
				score,
				potential_state_errors,
				network_historys);
		} else {
			backprop_score_network_with_potential(score,
												  potential_state_errors,
												  network_historys);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		if (iter_explore_node == this) {
			if (explore_decisions.back() == EXPLORE_DECISION_TYPE_EXPLORE) {
				this->explore_path_measure_count++;
				if (score == 1.0) {
					this->explore_explore_is_good += explore_diffs.back();
				} else {
					this->explore_explore_is_bad += explore_diffs.back();
				}
			} else if (explore_decisions.back() == EXPLORE_DECISION_TYPE_NO_EXPLORE) {
				this->explore_path_measure_count++;
				if (score == 1.0) {
					this->explore_no_explore_is_good += explore_diffs.back();
				} else {
					this->explore_no_explore_is_bad += explore_diffs.back();
				}
			}

			explore_decisions.pop_back();
			explore_diffs.pop_back();

			// TODO: add certainty networks
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		if (iter_explore_node == this) {
			int num_potential_states_on = 0;
			for (int ps_index = 0; ps_index < this->solution->current_potential_state_counter; ps_index++) {
				if (potential_states_on[ps_index]) {
					num_potential_states_on++;
				}
			}

			this->explore_path_measure_count++;

			this->explore_state_scores[num_potential_states_on] += score;
			this->explore_state_misguesses[num_potential_states_on] += misguess;

			explore_decisions.pop_back();
		}
	}
}

void SolutionNode::explore_increment(double score,
									 int iter_explore_type) {
	if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		this->explore_state_iter_index++;

		// if (this->explore_state_iter_index > 2000000) {
		if (this->explore_state_iter_index > 5) {
			this->explore_state_state = EXPLORE_STATE_STATE_MEASURE;
			this->explore_state_iter_index = 0;

			this->explore_state_measure_count = 0;
			for (int i = 0; i < 6; i++) {
				this->explore_state_scores[i] = 0.0;
				this->explore_state_misguesses[i] = 0.0;
			}
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
		int best_index = 0;
		for (int i = 1; i < 6; i++) {
			if (this->explore_state_scores[i] > this->explore_state_scores[i-1]
					|| this->explore_state_misguesses[i] > this->explore_state_misguesses[i-1]) {
				best_index = i;
				continue;
			} else {
				break;
			}
		}

		if (best_index > 0) {
			vector<int> extend_state_potential_indexes = {this->scope_potential_states.begin(),
				this->scope_potential_states.begin() + best_index};
			vector<int> extend_state_new_state_indexes;
			this->solution->state_mtx.lock();
			for (int p_index = 0; p_index < (int)extend_state_potential_indexes.size(); p_index++) {
				extend_state_new_state_indexes.push_back(this->solution->current_state_counter);
				this->solution->current_state_counter++;
			}
			this->solution->state_mtx.unlock();
			extend_with_potential_state(extend_state_potential_indexes,
										extend_state_new_state_indexes,
										this);

			ExploreNodeState* new_explore_node = new ExploreNodeState(
				this->solution->explore,
				this->node_index,
				extend_state_new_state_indexes);
			this->solution->explore->mtx.lock();
			this->solution->explore->current_node->children.push_back(new_explore_node);
			this->solution->explore->mtx.unlock();
			cout << "new ExploreNodeState" << endl;
		}

		reset_potential_state(this->scope_potential_states, this);

		this->explore_state_state = EXPLORE_STATE_STATE_LEARN;
		this->explore_state_iter_index = 0;
		this->has_explored_state = true;
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		if (score == 1.0 && this->explore_path_used) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP
					|| this->try_path.size() == 0) {
				// do nothing
			} else {
				for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
					cout << this->try_path[a_index].to_string() << endl;
				}
				cout << endl;

				SolutionNodeAction* curr_node = new SolutionNodeAction(this, -1, this->try_path[0]);
				curr_node->temp_node_state = TEMP_NODE_STATE_LEARN;
				this->explore_path.push_back(curr_node);
				for (int a_index = 1; a_index < (int)this->try_path.size(); a_index++) {
					SolutionNodeAction* next_node = new SolutionNodeAction(this, -1, this->try_path[a_index]);
					next_node->temp_node_state = TEMP_NODE_STATE_LEARN;
					this->explore_path.push_back(next_node);
					// no need to set previous
					curr_node->next = next_node;
					curr_node = next_node;
				}
				curr_node->next = this->explore_end_non_inclusive;
			}

			int input_size = 1 + (int)this->network_inputs_state_indexes.size();
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				this->explore_if_network = new Network(input_size, 4*input_size, 1);
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				this->explore_halt_network = new Network(input_size, 4*input_size, 1);
				this->explore_no_halt_network = new Network(input_size, 4*input_size, 1);
			}

			this->explore_path_state = EXPLORE_PATH_STATE_LEARN;
			this->explore_path_iter_index = 0;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 2000000) {
		if (this->explore_path_iter_index > 10) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				for (int e_index = 0; e_index < (int)this->explore_path.size(); e_index++) {
					this->explore_path[e_index]->temp_node_state = TEMP_NODE_STATE_MEASURE;
				}
			}

			this->explore_path_state = EXPLORE_PATH_STATE_MEASURE;
			this->explore_path_iter_index = 0;

			this->explore_path_measure_count = 0;
			this->explore_explore_is_good = 0.0;
			this->explore_explore_is_bad = 0.0;
			this->explore_no_explore_is_good = 0.0;
			this->explore_no_explore_is_bad = 0.0;
		}
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
		this->explore_path_iter_index++;

		// if (this->explore_path_iter_index > 100000) {
		if (this->explore_path_iter_index > 5) {
			double improvement = explore_explore_is_good - explore_no_explore_is_good;

			if (improvement > 0.0) {
				if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
					if (this->node_type == NODE_TYPE_IF_START) {
						SolutionNodeIfStart* this_if_start = (SolutionNodeIfStart*)this;
						if (this_if_start->explore_child_index == -1) {
							this->solution->nodes_mtx.lock();

							vector<int> new_path_node_indexes;
							for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
								int new_index = (int)this->solution->nodes.size();

								this->explore_path[n_index]->temp_node_state = TEMP_NODE_STATE_NOT;
								this->explore_path[n_index]->node_index = new_index;

								this->solution->nodes.push_back(this->explore_path[n_index]);
								new_path_node_indexes.push_back(new_index);
							}
							this->explore_path.clear();

							this->solution->nodes_mtx.unlock();

							this_if_start->children_nodes.push_back(NULL);
							this_if_start->children_score_networks.push_back(this->explore_if_network);
							this->explore_if_network = NULL;
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
								new_start_node_index,
								this_if_start->children_score_networks[this_if_start->explore_child_index]);
							// takes and clears explore networks
							this->solution->nodes.push_back(new_start_node);

							vector<int> new_path_node_indexes;
							for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
								int new_index = (int)this->solution->nodes.size();

								this->explore_path[n_index]->temp_node_state = TEMP_NODE_STATE_NOT;
								this->explore_path[n_index]->node_index = new_index;

								this->solution->nodes.push_back(this->explore_path[n_index]);
								new_path_node_indexes.push_back(new_index);
							}
							this->explore_path.clear();

							int new_end_node_index = (int)this->solution->nodes.size();
							SolutionNodeIfEnd* new_end_node = new SolutionNodeIfEnd(this, new_end_node_index);
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
						this->solution->nodes_mtx.lock();
						
						int new_start_node_index = (int)this->solution->nodes.size();
						SolutionNodeIfStart* new_start_node = new SolutionNodeIfStart(
							this,
							new_start_node_index,
							this->score_network);
						// takes and clears explore networks
						this->solution->nodes.push_back(new_start_node);

						vector<int> new_path_node_indexes;
						for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
							int new_index = (int)this->solution->nodes.size();

							this->explore_path[n_index]->temp_node_state = TEMP_NODE_STATE_NOT;
							this->explore_path[n_index]->node_index = new_index;

							this->solution->nodes.push_back(this->explore_path[n_index]);
							new_path_node_indexes.push_back(new_index);
						}
						this->explore_path.clear();

						int new_end_node_index = (int)this->solution->nodes.size();
						SolutionNodeIfEnd* new_end_node = new SolutionNodeIfEnd(this, new_end_node_index);
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
				} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
					this->solution->nodes_mtx.lock();

					int new_start_node_index = (int)this->solution->nodes.size();
					SolutionNodeLoopStart* new_start_node = new SolutionNodeLoopStart(
						this, new_start_node_index);
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
				}
			} else {
				if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
					delete this->explore_if_network;
					this->explore_if_network = NULL;

					for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
						delete this->explore_path[n_index];
					}
					this->explore_path.clear();
				} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
					delete this->explore_halt_network;
					this->explore_halt_network = NULL;
					delete this->explore_no_halt_network;
					this->explore_no_halt_network = NULL;
				}
			}

			this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
		}
	}
}

void SolutionNode::clear_explore() {
	this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	this->explore_path_iter_index = 0;
	this->explore_state_state = EXPLORE_STATE_STATE_LEARN;
	this->explore_state_iter_index = 0;

	this->try_path.clear();
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}
	this->explore_path.clear();

	if (this->explore_if_network != NULL) {
		delete this->explore_if_network;
		this->explore_if_network = NULL;
	}
	if (this->explore_halt_network != NULL) {
		delete this->explore_halt_network;
		this->explore_halt_network = NULL;
	}
	if (this->explore_no_halt_network != NULL) {
		delete this->explore_no_halt_network;
		this->explore_no_halt_network = NULL;
	}

	this->has_explored_state = false;
}

void SolutionNode::increment_unique_future_nodes(int num_future_nodes) {
	this->average_unique_future_nodes = 0.9999*this->average_unique_future_nodes + 0.0001*num_future_nodes;
}

void SolutionNode::add_potential_state_for_score_network(vector<int> potential_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		this->network_inputs_potential_state_indexes.push_back(
			potential_state_indexes[ps_index]);
		this->score_network->add_potential();
	}
}

void SolutionNode::extend_state_for_score_network(vector<int> potential_state_indexes,
												  vector<int> new_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->network_inputs_state_indexes.push_back(new_state_indexes[ps_index]);
				this->score_network->extend_with_potential(pi_index);
				break;
			}
		}

		if (this->explore_if_network != NULL) {
			this->explore_if_network->increment_input_size();
		}
		if (this->explore_halt_network != NULL) {
			this->explore_halt_network->increment_input_size();
		}
		if (this->explore_no_halt_network != NULL) {
			this->explore_no_halt_network->increment_input_size();
		}
	}
}

void SolutionNode::reset_potential_state_for_score_network(vector<int> potential_state_indexes) {
	for (int ps_index = 0; ps_index < (int)potential_state_indexes.size(); ps_index++) {
		for (int pi_index = 0; pi_index < (int)this->network_inputs_potential_state_indexes.size(); pi_index++) {
			if (this->network_inputs_potential_state_indexes[pi_index]
					== potential_state_indexes[ps_index]) {
				this->score_network->reset_potential(pi_index);
			}
		}
	}
}

void SolutionNode::clear_potential_state_for_score_network() {
	this->network_inputs_potential_state_indexes.clear();
	this->score_network->remove_potentials();
}

double SolutionNode::activate_score_network(Problem& problem,
											double* state_vals,
											bool* states_on,
											bool backprop,
											vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs, network_historys);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_score_network(double score,
										  double* state_errors,
										  bool* states_on,
										  vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->score_network->mtx.lock();

	if (network_history->network != this->score_network) {
		cout << "ERROR: score_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> score_network_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	}
	this->score_network->backprop(score_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

void SolutionNode::backprop_score_network_errors_with_no_weight_change(
		double score,
		double* state_errors,
		bool* states_on,
		vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->score_network->mtx.lock();

	if (network_history->network != this->score_network) {
		cout << "ERROR: score_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> score_network_errors;
	if (score == 1.0) {
		if (this->score_network->output->acti_vals[0] < 1.0) {
			score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	} else {
		if (this->score_network->output->acti_vals[0] > 0.0) {
			score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
		} else {
			score_network_errors.push_back(0.0);
		}
	}
	this->score_network->backprop_errors_with_no_weight_change(score_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->score_network->input->errors[1+i_index];
		}
		this->score_network->input->errors[1+i_index] = 0.0;
	}

	this->score_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_score_network_with_potential(
		Problem& problem,
		double* state_vals,
		bool* states_on,
		double* potential_state_vals,
		bool* potential_states_on,
		bool backprop,
		vector<NetworkHistory*>& network_historys) {
	vector<double> score_network_inputs;
	double curr_observations = problem.get_observation();
	score_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	vector<int> potentials_on;
	vector<double> potential_vals;
	for (int p_index = 0; p_index < (int)this->network_inputs_potential_state_indexes.size(); p_index++) {
		if (potential_states_on[this->network_inputs_potential_state_indexes[p_index]]) {
			potentials_on.push_back(p_index);
			potential_vals.push_back(potential_state_vals[this->network_inputs_potential_state_indexes[p_index]]);
		}
	}

	double score;
	if (potentials_on.size() > 0) {
		if (backprop) {
			this->score_network->mtx.lock();
			this->score_network->activate(score_network_inputs,
										  potentials_on,
										  potential_vals,
										  network_historys);
			score = this->score_network->output->acti_vals[0];
			this->score_network->mtx.unlock();
		} else {
			this->score_network->mtx.lock();
			this->score_network->activate(score_network_inputs,
										  potentials_on,
										  potential_vals);
			score = this->score_network->output->acti_vals[0];
			this->score_network->mtx.unlock();
		}
	} else {
		this->score_network->mtx.lock();
		this->score_network->activate(score_network_inputs);
		score = this->score_network->output->acti_vals[0];
		this->score_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_score_network_with_potential(
		double score,
		double* potential_state_errors,
		vector<NetworkHistory*>& network_historys) {
	if (network_historys.size() > 0) {
		NetworkHistory* network_history = network_historys.back();
		if (network_history->network == this->score_network) {
			this->score_network->mtx.lock();

			network_history->reset_weights();

			vector<int> potentials_on = network_history->potentials_on;

			vector<double> score_network_errors;
			if (score == 1.0) {
				if (this->score_network->output->acti_vals[0] < 1.0) {
					score_network_errors.push_back(1.0 - this->score_network->output->acti_vals[0]);
				} else {
					score_network_errors.push_back(0.0);
				}
			} else {
				if (this->score_network->output->acti_vals[0] > 0.0) {
					score_network_errors.push_back(0.0 - this->score_network->output->acti_vals[0]);
				} else {
					score_network_errors.push_back(0.0);
				}
			}
			this->score_network->backprop(score_network_errors,
										  potentials_on);

			for (int o_index = 0; o_index < (int)potentials_on.size(); o_index++) {
				potential_state_errors[this->network_inputs_potential_state_indexes[potentials_on[o_index]]] += \
					this->score_network->potential_inputs[potentials_on[o_index]]->errors[0];
				this->score_network->potential_inputs[potentials_on[o_index]]->errors[0] = 0.0;
			}

			this->score_network->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}

double SolutionNode::activate_explore_if_network(Problem& problem,
												 double* state_vals,
												 bool* states_on,
												 bool backprop,
												 vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
	}

	double score;
	if (backprop) {
		this->explore_if_network->mtx.lock();
		this->explore_if_network->activate(explore_network_inputs, network_historys);
		score = this->explore_if_network->output->acti_vals[0];
		this->explore_if_network->mtx.unlock();
	} else {
		this->explore_if_network->mtx.lock();
		this->explore_if_network->activate(explore_network_inputs);
		score = this->explore_if_network->output->acti_vals[0];
		this->explore_if_network->mtx.unlock();
	}

	return score;
}

void SolutionNode::backprop_explore_if_network(double score,
											   double* state_errors,
											   bool* states_on,
											   vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_if_network->mtx.lock();

	if (network_history->network != this->explore_if_network) {
		cout << "ERROR: explore_if_network backprop mismatch" << endl;
	}

	network_history->reset_weights();

	vector<double> explore_network_errors;
	if (score == 1.0) {
		if (this->explore_if_network->output->acti_vals[0] < 1.0) {
			explore_network_errors.push_back(1.0 - this->explore_if_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	} else {
		if (this->explore_if_network->output->acti_vals[0] > 0.0) {
			explore_network_errors.push_back(0.0 - this->explore_if_network->output->acti_vals[0]);
		} else {
			explore_network_errors.push_back(0.0);
		}
	}
	this->explore_if_network->backprop(explore_network_errors);

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->explore_if_network->input->errors[1+i_index];
		}
		this->explore_if_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_if_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_halt_network(Problem& problem,
												   double* state_vals,
												   bool* states_on,
												   bool backprop,
												   vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
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
												 double* state_errors,
												 bool* states_on,
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

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->explore_halt_network->input->errors[1+i_index];
		}
		this->explore_halt_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNode::activate_explore_no_halt_network(Problem& problem,
													  double* state_vals,
													  bool* states_on,
													  bool backprop,
													  vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			explore_network_inputs.push_back(state_vals[this->network_inputs_state_indexes[i_index]]);
		} else {
			explore_network_inputs.push_back(0.0);
		}
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
													double* state_errors,
													bool* states_on,
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

	for (int i_index = 0; i_index < (int)this->network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->network_inputs_state_indexes[i_index]]) {
			state_errors[this->network_inputs_state_indexes[i_index]] += \
				this->explore_no_halt_network->input->errors[1+i_index];
		}
		this->explore_no_halt_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_no_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}
