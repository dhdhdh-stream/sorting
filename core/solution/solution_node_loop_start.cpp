#include "solution_node_loop_start.h"

using namespace std;

void SolutionNodeLoopStart::reset() override {
	this->states_on->clear();
}

void SolutionNodeLoopStart::add_potential_state(vector<int> potential_state_indexes,
												SolutionNode* scope) override {
	add_potential_state_for_score_network(potential_state_indexes);

	this->next->add_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopStart::extend_with_potential_state(vector<int> potential_state_indexes,
														vector<int> new_state_indexes,
														SolutionNode* scope) override {
	extend_state_for_score_network(potential_state_indexes);

	this->next->extend_with_potential_state(potential_state_indexes,
											new_state_indexes,
											scope);
}

void SolutionNodeLoopStart::reset_potential_state(vector<int> potential_state_indexes,
												  SolutionNode* scope) override {
	reset_potential_state_for_score_network(potential_state_indexes);

	this->next->reset_potential_state(potential_state_indexes, scope);
}

void SolutionNodeLoopStart::clear_potential_state() {
	clear_potential_state_for_score_network();
}

SolutionNode* SolutionNodeLoopStart::activate(Problem& problem,
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
											  vector<int>& explore_decisions) override {
	if (iter_explore_type == EXPLORE_TYPE_NONE && is_first_time) {
		if (randuni() < (2.0/this->average_future_nodes)) {
			if (rand()%2 == 0) {
				if (!this->has_explored_state) {
					if (this->explore_state_state == EXPLORE_STATE_STATE_LEARN) {
						int rand_states = 1 + rand()%5;
						this->try_states = {this->scope_potential_states.begin(),
							this->scope_potential_states.begin() + rand_states};

						iter_explore_node = this;
						iter_explore_type = EXPLORE_TYPE_LEARN_STATE;
					} else if (this->explore_state_state == EXPLORE_STATE_STATE_MEASURE) {
						int rand_states = rand()%6;
						this->try_states = {this->scope_potential_states.begin(),
							this->scope_potential_states.begin() + rand_states};

						iter_explore_node = this;
						iter_explore_type = EXPLORE_TYPE_MEASURE_STATE;
					}
				}
			} else {
				if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					find_potential_jumps(this->next,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);
					if (rand()%(potential_inclusive_jump_ends.size()+1) == 0) {
						seq_length = 1 + seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = NULL;
						this->explore_end_inclusive = NULL;
						this->explore_end_non_inclusive = this->next;

						this->path_explore_type = PATH_EXPLORE_TYPE_LOOP;
					} else {
						int random_index = rand()%potential_inclusive_jump_ends.size();

						if (this == potential_inclusive_jump_ends[random_index]) {
							seq_length = 1 + seq_length_dist(generator);

							this->explore_start_non_inclusive = this;
							this->explore_start_inclusive = NULL;
							this->explore_end_inclusive = NULL;
							this->explore_end_non_inclusive = this->next;
						} else {
							seq_length = seq_length_dist(generator);

							this->explore_start_non_inclusive = this;
							this->explore_start_inclusive = this->next;
							this->explore_end_inclusive = potential_inclusive_jump_ends[random_index];
							this->explore_end_non_inclusive = potential_non_inclusive_jump_ends[random_index];
						}

						this->path_explore_type = PATH_EXPLORE_TYPE_JUMP;
					}

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

	bool skip_action = false;
	if (iter_explore_node == this) {
		if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
			if (this->explore_start_inclusive == NULL) {
				if (loop_scopes.back() == this) {
					skip_action = true;
				}
			}
		}
	}

	double score;
	if (!skip_action) {
		if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
			score = activate_score_network(problem,
										   state_vals,
										   states_on,
										   true,
										   network_historys);
			guesses.push_back(score);
		} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
			// do nothing
		} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
			if (iter_explore_node == this) {
				score = activate_score_network(problem,
											   state_vals,
											   states_on,
											   false,
											   network_historys);
			}
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
			if (iter_explore_node == this
					&& this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
					// do nothing
				} else {
					score = activate_score_network(problem,
												   state_vals,
												   states_on,
												   false,
												   network_historys);
				}
			} else {
				score = activate_score_network(problem,
											   state_vals,
											   states_on,
											   true,
											   network_historys);
			}
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
			score = activate_score_network_with_potential(
				problem,
				state_vals,
				states_on,
				potential_state_vals,
				potential_states_on,
				true,
				network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_PATH) {
			score = activate_score_network(problem,
										   state_vals,
										   states_on,
										   false,
										   network_historys);
			if (iter_explore_node != this) {
				guesses.push_back(score);
			}
		} else if (iter_explore_type == EXPLORE_TYPE_MEASURE_STATE) {
			score = activate_score_network_with_potential(
				problem,
				state_vals,
				states_on,
				potential_state_vals,
				potential_states_on
				false,
				network_historys);
			guesses.push_back(score);
		}
	}

	if (iter_explore_node == this) {
		if (explore_type == EXPLORE_TYPE_LEARN_STATE) {
			for (int ts_index = 0; ts_index < (int)this->try_states.size(); ts_index++) {
				potential_state_vals[this->try_states[ts_index]] = 0.0;
				potential_states_on[this->try_states[ts_index]] = true;
			}
		} else if (explore_type == EXPLORE_TYPE_MEASURE_STATE) {
			explore_decisions.push_back(this->try_states.size());
			for (int ts_index = 0; ts_index < (int)this->try_states.size(); ts_index++) {
				potential_state_vals[this->try_states[ts_index]] = 0.0;
				potential_states_on[this->try_states[ts_index]] = true;
			}
		} else if (explore_type == EXPLORE_TYPE_PATH) {
			if (score < this->average) {
				// explore path
			}
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_EXPLORE) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				bool do_explore = false;
				if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
					if (score > this->average) {
						do_explore = true;
					}
				} else {
					if (score < this->average) {
						do_explore = true;
					}
				}

				if (do_explore) {
					for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
						problem.perform_action(try_path[a_index]);
					}

					return this->explore_end_non_inclusive;
				}
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
					if ((loop_scope_counts.back() == 3 && rand()%3 == 0)
							(loop_scope_counts.back() == 4 && rand()%2 == 0)
							loop_scope_counts.back() == 5) {
						loop_scopes.pop_back();
						loop_scope_counts.pop_back();

						return this->next;
					} else {
						loop_scope_counts.back()++;

						for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
							problem.perform_action(try_path[a_index]);
						}

						return this->explore_start_inclusive;
					}
				} else {
					bool do_explore = false;
					if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
						if (score > this->average) {
							do_explore = true;
						}
					} else {
						if (score < this->average) {
							do_explore = true;
						}
					}

					if (do_explore) {
						loop_scopes.push_back(this);
						loop_scope_counts.push_back(1);

						for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
							problem.perform_action(try_path[a_index]);
						}

						return this->explore_start_inclusive;
					}
				}
			}
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_LEARN) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				bool do_explore = false;
				if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
					if (score > this->average) {
						do_explore = true;
					}
				} else {
					if (score < this->average) {
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

					return this->explore_path[0];
				}
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
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

						return this->next;
					} else {
						activate_explore_no_halt_network(problem,
														 state_vals,
														 states_on,
														 true,
														 network_historys);

						loop_scope_counts.back()++;

						return this->explore_path[0];
					}
				} else {
					bool do_explore = false;
					if (this->path_target_type == PATH_TARGET_TYPE_GOOD) {
						if (score > this->average) {
							do_explore = true;
						}
					} else {
						if (score < this->average) {
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

						loop_scopes.push_back(this);
						loop_scope_counts.push_back(1);

						return this->explore_path[0];
					} else {
						// score_network history not saved

						activate_explore_no_halt_network(problem,
														 state_vals,
														 states_on,
														 true
														 network_historys);

						return this->next;
					}
				}
			}
		} else if (this->explore_path_state == EXPLORE_PATH_STATE_MEASURE) {
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
						return this->explore_path[0];
					} else {
						guesses.push_back(score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
						return this->next;
					}
				} else {
					explore_diffs.push_back(0.0);
					guesses.push_back(score);
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_N_A);
					return this->next;
				}
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
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
						return this->next;
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
						return this->next;
					} else {
						loop_scope_counts.back()++;

						guesses.push_back(no_halt_score);
						return this->explore_path[0];
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
						return this->next;
					} else {
						explore_diffs.push_back(no_halt_score - halt_score);
						if (rand()%2 == 0) {
							loop_scopes.push_back(this);
							loop_scope_counts.push_back(1);

							loop_scopes.push_back(this);
							loop_scope_counts.push_back(1);

							guesses.push_back(no_halt_score);
							explore_decisions.push_back(EXPLORE_DECISION_TYPE_EXPLORE);
							return this->explore_path[0];
						} else {
							guesses.push_back(halt_score);
							explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_EXPLORE);
							return this->next;
						}
					}
				}
			}
		}
	}

	for (int o_index = 0; o_index < (int)this->scope_states_on.size(); o_index++) {
		state_vals[this->scope_states_on[o_index]] = 0.0;
		states_on[this->scope_states_on[o_index]] = true;
	}

	return this->next;
}

void SolutionNodeLoopStart::backprop(double score,
									 double misguess,
									 double* state_errors,
									 bool* states_on,
									 int& iter_explore_type,
									 SolutionNode* iter_explore_node,
									 double* potential_state_errors,
									 bool* potential_states_on,
									 vector<NetworkHistory*>& network_historys,
									 vector<int>& explore_decisions) override {
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_score_network(score,
							   state_errors,
							   states_on,
							   network_historys);

		// TODO: add certainty networks

		this->average_score = 0.9999*this->average_score + score;
		this->average_misguess = 0.9999*this->average_score + score;
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		if (iter_explore_node == this) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				NetworkHistory* network_history = network_historys.back();
				if (network_history->network == this->explore_if_network) {
					backprop_explore_if_network(score,
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
					backprop_score_network_errors_with_no_weight_change(
						score,
						state_errors,
						states_on,
						network_historys);
				}
			}
		} else {
			backprop_score_network_errors_with_no_weight_change(
				score,
				state_errors,
				states_on,
				network_historys);
		}
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		backprop_score_network_with_potential(score,
											  potential_state_errors,
											  network_historys);
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
			this->explore_path_measure_count++;

			this->explore_state_scores[explore_decisions.back()] += score;
			this->explore_state_misguesses[explore_decisions.back()] += misguess;

			explore_decisions.pop_back();
		}
	}
}

void SolutionNodeLoopStart::explore_increment(double score,
											  int iter_explore_type) override {
	if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		this->explore_state_iter_index++;

		if (this->explore_state_iter_index > 2000000) {
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
		}

		reset_potential_state(this->scope_potential_states, this);

		this->explore_state_state = EXPLORE_STATE_STATE_LEARN;
		this->explore_state_iter_index = 0;
		this->has_explored_state = true;
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		if (score == 1.0) {
			if (this->try_path.size() == 0) {
				// do nothing
			} else {
				SolutionNodeNormal* curr_node = new SolutionNodeNormal(-1, this->try_path[0]);
				curr_node->temp_node_state = TEMP_NODE_STATE_LEARN;
				this->explore_path.push_back(curr_node);
				for (int a_index = 1; a_index < (int)this->try_path.size(); a_index++) {
					SolutionNodeNormal* next_node = new SolutionNodeNormal(-1, this->try_path[a_index]);
					next_node->temp_node_state = TEMP_NODE_STATE_LEARN;
					this->explore_path.push_back(next_node);
					// no need to set previous
					curr_node->next = next_node;
					curr_node = next_node;
				}
				if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
					curr_node->next = this->explore_end_non_inclusive;
				} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
					if (this->explore_start_inclusive == NULL) {
						curr_node->next = this;
					} else {
						curr_node->next = this->explore_start_inclusive;
					}
				}
			}

			int input_size = 1 + (int)this->network_inputs_state_indexes;
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

		if (this->explore_path_iter_index > 2000000) {
			for (int e_index = 0; e_index < (int)this->explore_path.size(); e_index++) {
				this->explore_path[e_index]->temp_node_state = TEMP_NODE_STATE_MEASURE;
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
		double improvement = explore_explore_is_good - explore_no_explore_is_good;

		if (improvement > 0.0) {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				this->solution->nodes_mtx.lock();
				
				int new_start_node_index = (int)this->solution->nodes.size();
				SolutionNodeIfStart* new_start_node = new SolutionNodeIfStart(
					new_start_node_index, this);
				// takes and clears explore networks
				this->solution->nodes.push_back(new_start_node);

				vector<int> new_path_node_indexes;
				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->temp_node_state = TEMP_NODE_STATE_NOT;

					int new_index = (int)this->solution->nodes.size();
					this->solution->nodes.push_back(this->explore_path[n_index]);
					new_path_node_indexes.push_back(new_index);
				}
				this->explore_path.clear();

				int new_end_node_index = (int)this->solution->nodes.size();
				SolutionNodeIfEnd* new_end_node = new SolutionNodeIfEnd(new_end_node_index);
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
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				this->solution->nodes_mtx.lock();

				int new_start_node_index = (int)this->solution->nodes.size();
				SolutionNodeLoopStart* new_start_node = new SolutionNodeLoopStart(this);
				this->solution->nodes.push_back(new_start_node);

				vector<int> new_path_node_indexes;
				for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
					this->explore_path[n_index]->temp_node_state = TEMP_NODE_STATE_NOT;

					int new_index = (int)this->solution->nodes.size();
					this->solution->nodes.push_back(this->explore_path[n_index]);
					new_path_node_indexes.push_back(new_index);
				}
				this->explore_path.clear();

				int new_end_node_index = (int)this->solution->nodes.size();
				SolutionNodeLoopEnd* new_end_node = new SolutionNodeLoopEnd(
					new_end_node_index, this);
				// takes and clears explore networks
				this->solution->nodes.push_back(new_end_node);

				ExploreNodeLoop* new_explore_node;
				if (this->explore_start_inclusive == NULL) {
					new_explore_node = new ExploreNodeLoop(
						this->solution->explore,
						this->explore_start_non_inclusive->node_index,
						-1,
						-1,
						this->explore_end_non_inclusive->node_index,
						new_start_node_index,
						new_end_node_index,
						new_path_node_indexes);
				} else {
					new_explore_node = new ExploreNodeLoop(
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
			}
		} else {
			if (this->path_explore_type == PATH_EXPLORE_TYPE_JUMP) {
				delete this->explore_if_network;
				this->explore_if_network = NULL;
			} else if (this->path_explore_type == PATH_EXPLORE_TYPE_LOOP) {
				delete this->explore_halt_network;
				this->explore_halt_network = NULL;
				delete this->explore_no_halt_network;
				this->explore_no_halt_network = NULL;
			}

			for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
				delete this->explore_path[n_index];
			}
			this->explore_path.clear();
		}

		this->explore_path_state = EXPLORE_PATH_STATE_EXPLORE;
	}
}
