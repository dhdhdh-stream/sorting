#include "solution_node_normal.h"

#include "solution_node_utilities.h"

using namespace std;

void SolutionNodeNormal::reset() override {
	// do nothing
}

SolutionNode* SolutionNodeNormal::activate(Problem& problem,
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
		if (randuni() < (1.0/this->average_future_nodes)) {
			if (this->explore_state == EXPLORE_STATE_EXPLORE) {
				int rand_index = rand()%3;
				if (rand_index == 0) {
					SolutionNode* inclusive_end;
					SolutionNode* non_inclusive_end;
					find_scope_end(this, inclusive_end, non_inclusive_end);
					
					if (this == inclusive_end) {
						seq_length = 1 + seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = NULL;
						this->explore_end_inclusive = NULL;
						this->explore_end_non_inclusive = this->next;
					} else {
						seq_length = seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = this->next;
						this->explore_end_inclusive = inclusive_end;
						this->explore_end_non_inclusive = non_inclusive_end;
					}

					this->node_explore_type = NODE_NORMAL_EXPLORE_TYPE_JUMP;
				} else if (rand_index == 1) {
					vector<SolutionNode*> potential_inclusive_jump_ends;
					vector<SolutionNode*> potential_non_inclusive_jump_ends;
					find_potential_jumps(this,
										 potential_inclusive_jump_ends,
										 potential_non_inclusive_jump_ends);
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

					this->node_explore_type = NODE_NORMAL_EXPLORE_TYPE_JUMP;
				} else {
					vector<SolutionNode*> potential_non_inclusive_loop_starts;
					vector<SolutionNode*> potential_inclusive_loop_starts;
					potential_non_inclusive_loop_starts.push_back(this);
					potential_inclusive_loop_starts.push_back(this->next);
					find_potential_loops(this,
										 potential_non_inclusive_loop_starts,
										 potential_inclusive_loop_starts);
					int random_index = rand()%potential_non_inclusive_loop_starts.size();

					if (this == potential_non_inclusive_loop_starts[random_index]) {
						seq_length = 1 + seq_length_dist(generator);

						this->explore_start_non_inclusive = this;
						this->explore_start_inclusive = NULL;
						this->explore_end_inclusive = NULL;
						this->explore_end_non_inclusive = this->next;
					} else {
						seq_length = seq_length_dist(generator);

						this->explore_start_non_inclusive = potential_non_inclusive_loop_starts[random_index];
						this->explore_start_inclusive = potential_inclusive_loop_starts[random_index];
						this->explore_end_inclusive = this;
						this->explore_end_non_inclusive = this->next;
					}
					
					this->node_explore_type = NODE_NORMAL_EXPLORE_TYPE_LOOP;
				}

				try_path.clear();

				normal_distribution<double> write_val_dist(0.0, 2.0);
				for (int i = 0; i < seq_length; i++) {
					Action a(write_val_dist(generator), rand()%3);
					try_path.push_back(a);
				}

				if (rand()%2 == 0) {
					this->explore_target_type = EXPLORE_TARGET_TYPE_GOOD;
				} else {
					this->explore_target_type = EXPLORE_TARGET_TYPE_BAD;
				}

				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_EXPLORE;
			} else if (this->explore_state == EXPLORE_STATE_LEARN) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_LEARN_PATH;
			} else if (this->explore_state == EXPLORE_STATE_MEASURE) {
				iter_explore_node = this;
				iter_explore_type = EXPLORE_TYPE_MEASURE
			}
		}
	}

	bool skip_action = false;
	if (iter_explore_node == this) {
		if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
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
			activate_state_networks_eval(problem,
										 state_vals,
										 states_on,
										 network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
			activate_state_networks(problem,
									state_vals,
									states_on,
									network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
			activate_state_networks(problem,
									state_vals,
									states_on,
									network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
			activate_state_networks_eval(problem,
										 state_vals,
										 states_on,
										 network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
			activate_state_networks_state(problem,
										  state_vals,
										  states_on,
										  potential_state_vals,
										  potential_states_on
										  network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_MEASURE) {
			activate_state_networks(problem,
									state_vals,
									states_on,
									network_historys);
		}

		problem.perform_action(this->action);

		if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
			score = activate_score_network_eval(problem,
												state_vals,
												states_on,
												network_historys);
			guesses.push_back(score);
		} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
			// do nothing
		} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
			// do nothing
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
			if (iter_explore_node == this
					&& this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
					// do nothing
				} else {
					score = activate_score_network(problem,
												   state_vals,
												   states_on);
				}
			} else {
				score = activate_score_network_eval(problem,
													state_vals,
													states_on,
													network_historys);
			}
		} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
			score = activate_score_network_state(problem,
												 state_vals,
												 states_on,
												 potential_state_vals,
												 potential_states_on,
												 network_historys);
		} else if (iter_explore_type == EXPLORE_TYPE_MEASURE) {
			score = activate_score_network(problem,
										   state_vals,
										   states_on);
			if (iter_explore_node != this) {
				guesses.push_back(score);
			}
		}
	}

	if (iter_explore_node == this) {
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				bool do_explore = false;
				if (this->explore_target_type == EXPLORE_TARGET_TYPE_GOOD) {
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
			} else if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
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
					if (this->explore_target_type == EXPLORE_TARGET_TYPE_GOOD) {
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
		} else if (this->explore_state == EXPLORE_STATE_LEARN) {
			if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				bool do_explore = false;
				if (this->explore_target_type == EXPLORE_TARGET_TYPE_GOOD) {
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

					activate_explore_if_network_eval(problem,
													 state_vals,
													 states_on,
													 network_historys);

					return this->explore_path[0];
				}
			} else if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
					// score_network not run

					int current_count = loop_scope_counts.back();
					if (rand()%(6-current_count) == 0) {
						activate_explore_halt_network_eval(problem,
														   state_vals,
														   states_on,
														   network_historys);

						loop_scopes.pop_back();
						loop_scope_counts.pop_back();

						return this->next;
					} else {
						activate_explore_no_halt_network_eval(problem,
															  state_vals,
															  states_on,
															  network_historys);

						loop_scope_counts.back()++;

						return this->explore_path[0];
					}
				} else {
					bool do_explore = false;
					if (this->explore_target_type == EXPLORE_TARGET_TYPE_GOOD) {
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

						activate_explore_no_halt_network_eval(problem,
															  state_vals,
															  states_on,
															  network_historys);

						loop_scopes.push_back(this);
						loop_scope_counts.push_back(1);

						return this->explore_path[0];
					} else {
						// score_network history not saved

						activate_explore_no_halt_network_eval(problem,
															  state_vals,
															  states_on,
															  network_historys);

						return this->next;
					}
				}
			}
		} else if (this->explore_state == EXPLORE_STATE_MEASURE) {
			if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				double explore_score = activate_explore_if_network(problem,
																   state_vals,
																   states_on);
				if (explore_score > score) {
					guesses.push_back(explore_score);
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_JUMP);
					return this->explore_path[0];
				} else {
					guesses.push_back(score);
					explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_JUMP);
					return this->next;
				}
			} else if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
					if (loop_scope_counts.back() >= 20) {
						double halt_score = activate_explore_halt_network(problem,
																		  state_vals,
																		  states_on);
						
						loop_scopes.pop_back();
						loop_scope_counts.pop_back();

						guesses.push_back(halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_HALT);
						return this->next;
					}

					double halt_score = activate_explore_halt_network(problem,
																	  state_vals,
																	  states_on);
					double no_halt_score = activate_explore_no_halt_network(problem,
																			state_vals,
																			states_on);
					if (halt_score > no_halt_score) {
						loop_scopes.pop_back();
						loop_scope_counts.pop_back();

						guesses.push_back(halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_HALT);
						return this->next;
					} else {
						loop_scope_counts.back()++;

						guesses.push_back(no_halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_HALT);
						return this->explore_path[0];
					}
				} else {
					double halt_score = activate_explore_halt_network(problem,
																	  state_vals,
																	  states_on);
					double no_halt_score = activate_explore_no_halt_network(problem,
																			state_vals,
																			states_on);

					if (halt_score > no_halt_score) {
						guesses.push_back(halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_HALT);
						return this->next;
					} else {
						loop_scopes.push_back(this);
						loop_scope_counts.push_back(1);

						guesses.push_back(no_halt_score);
						explore_decisions.push_back(EXPLORE_DECISION_TYPE_NO_HALT);
						return this->explore_path[0];
					}
				}
			}
		}
	}

	return this->next;
}

void SolutionNodeNormal::backprop(double score,
								  double misguess,
								  double* state_errors,
								  bool* states_on,
								  int& iter_explore_type,
								  SolutionNode* iter_explore_node,
								  double* potential_state_errors,
								  bool* potential_states_on,
								  std::vector<NetworkHistory*>& network_historys) override {
	if (iter_explore_type == EXPLORE_TYPE_RE_EVAL) {
		backprop_score_network_eval(score,
									state_errors,
									states_on,
									network_historys);

		backprop_state_networks_eval(state_errors,
									 states_on,
									 network_historys);

		this->average_score = 0.9999*this->average_score + score;
		// TODO: add certainty networks
	} else if (iter_explore_type == EXPLORE_TYPE_NONE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_EXPLORE) {
		// should not happen
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_PATH) {
		if (iter_explore_node == this) {
			if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				NetworkHistory* network_history = network_historys.back();
				if (network_history->network == this->explore_if_network) {
					backprop_explore_if_network_eval(score,
													 state_errors,
													 states_on,
													 network_historys);
				} else {
					backprop_score_network_path(score,
												state_errors,
												states_on,
												network_historys);
				}
			} else if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				NetworkHistory* network_history = network_historys.back();
				if (network_history->network == this->explore_halt_network) {
					backprop_explore_halt_network_eval(score,
													   state_errors,
													   states_on,
													   network_historys);
				} else if (network_history->network == this->explore_no_halt_network) {
					backprop_explore_no_halt_network_eval(score,
														  state_errors,
														  states_on,
														  network_historys);
				} else {
					backprop_score_network_path(score,
												state_errors,
												states_on,
												network_historys);
				}
			}
		} else {
			backprop_score_network_path(score,
										state_errors,
										states_on,
										network_historys);
		}

		backprop_state_networks_path(state_errors,
									 states_on,
									 network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_LEARN_STATE) {
		backprop_score_network_state(score,
									 potential_state_errors,
									 network_historys);

		backprop_state_networks_state(state_errors,
									  states_on,
									  network_historys);
	} else if (iter_explore_type == EXPLORE_TYPE_MEASURE) {

	}
}

void SolutionNodeNormal::explore_increment(double score) override {
	if (this->explore_state == EXPLORE_STATE_EXPLORE) {
		if (score == 1.0) {
			if (this->try_path.size() == 0) {
				// do nothing
			} else {
				SolutionNodeNormal* curr_node = new SolutionNodeNormal(-1, this->try_path[0]);
				curr_node->explore_node_state = EXPLORE_NODE_STATE_LEARN;
				this->explore_path.push_back(curr_node);
				for (int a_index = 1; a_index < (int)this->try_path.size(); a_index++) {
					SolutionNodeNormal* next_node = new SolutionNodeNormal(-1, this->try_path[a_index]);
					next_node->explore_node_state = EXPLORE_NODE_STATE_LEARN;
					this->explore_path.push_back(next_node);
					// no need to set previous
					curr_node->next = next_node;
					curr_node = next_node;
				}
				if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
					curr_node->next = this->explore_end_non_inclusive;
				} else if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
					if (this->explore_start_inclusive == NULL) {
						curr_node->next = this;
					} else {
						curr_node->next = this->explore_start_inclusive;
					}
				}
			}

			int input_size = (int)this->score_network_inputs_state_indexes+1;
			if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				this->explore_if_network = new Network(input_size, 4*input_size, 1);
			} else if (this->node_explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				this->explore_halt_network = new Network(input_size, 4*input_size, 1);
				this->explore_no_halt_network = new Network(input_size, 4*input_size, 1);
			}

			this->explore_state = EXPLORE_STATE_LEARN;
			this->explore_iter_index = 0;
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 2000000) {
			for (int e_index = 0; e_index < (int)this->explore_path.size(); e_index++) {
				this->explore_path[e_index]->explore_node_state = EXPLORE_NODE_STATE_MEASURE;
			}

			this->explore_state = EXPLORE_STATE_MEASURE;
			this->explore_iter_index = 0;
		}
	}
}

void SolutionNodeNormal::activate_state_networks(Problem& problem,
												 double* state_vals,
												 bool* states_on) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[i_index]]) {
					state_network_inputs.push_back(state_vals[this->state_network_inputs_state_indexes[i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}

			this->state_networks[sn_index]->mtx.lock();
			this->state_networks[sn_index]->activate(state_network_inputs);
			state_vals[this->state_networks_target_states[sn_index]] = \
				this->state_networks[sn_index]->output->acti_vals[0];
			this->state_networks[sn_index]->mtx.unlock();
		}
	}
}

void SolutionNodeNormal::activate_state_networks_eval(Problem& problem,
													  double* state_vals,
													  bool* states_on,
													  vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[i_index]]) {
					state_network_inputs.push_back(state_vals[this->state_network_inputs_state_indexes[i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}

			this->state_networks[sn_index]->mtx.lock();
			this->state_networks[sn_index]->activate(state_network_inputs, network_historys);
			state_vals[this->state_networks_target_states[sn_index]] = \
				this->state_networks[sn_index]->output->acti_vals[0];
			this->state_networks[sn_index]->mtx.unlock();
		}
	}
}

void SolutionNodeNormal::backprop_state_networks_eval(double* state_errors,
													  bool* states_on,
													  vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[
				this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->backprop(state_network_errors);

			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[i_index]]) {
					if (this->state_network_inputs_state_indexes[i_index]
							== this->state_networks_target_states[sn_index]) {
						state_errors[this->state_network_inputs_state_indexes[i_index]] = \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					} else {
						state_errors[this->state_network_inputs_state_indexes[i_index]] += \
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

void SolutionNodeNormal::backprop_state_networks_path(double* state_errors,
													  bool* states_on,
													  vector<NetworkHistory*>& network_historys) {
	for (int sn_index = 0; sn_index < (int)this->state_networks_target_states.size(); sn_index++) {
		if (states_on[this->state_networks_target_states[sn_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->state_networks[sn_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(state_errors[
				this->state_networks_target_states[sn_index]]);
			this->state_networks[sn_index]->backprop_errors_with_no_weight_change(state_network_errors);

			for (int i_index = 0; i_index < (int)this->state_network_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->state_network_inputs_state_indexes[i_index]]) {
					if (this->state_network_inputs_state_indexes[i_index]
							== this->state_networks_target_states[sn_index]) {
						state_errors[this->state_network_inputs_state_indexes[i_index]] = \
							this->state_networks[sn_index]->input->errors[1 + i_index];
					} else {
						state_errors[this->state_network_inputs_state_indexes[i_index]] += \
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

void SolutionNodeNormal::activate_state_networks_state(Problem& problem,
													   double* state_vals,
													   bool* states_on,
													   double* potential_state_vals,
													   bool* potential_states_on,
													   vector<NetworkHistory*>& network_historys) {
	activate_state_networks(problem,
							state_vals,
							states_on);

	for (int p_index = 0; p_index < (int)this->potential_state_networks_target_states.size(); p_index++) {
		if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
			vector<double> state_network_inputs;
			double curr_observations = problem.get_observation();
			state_network_inputs.push_back(curr_observations);
			for (int i_index = 0; i_index < (int)this->potential_inputs_state_indexes[sn_index].size(); i_index++) {
				if (states_on[this->potential_inputs_state_indexes[i_index]]) {
					state_network_inputs.push_back(state_vals[this->potential_inputs_state_indexes[i_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}
			for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes.size(); pi_index++) {
				if (potential_states_on[this->potential_potential_inputs_state_indexes[pi_index]]) {
					state_network_inputs.push_back(potential_state_vals[this->potential_potential_inputs_state_indexes[pi_index]]);
				} else {
					state_network_inputs.push_back(0.0);
				}
			}
			this->potential_state_networks[p_index]->mtx.lock();
			this->potential_state_networks[p_index]->activate(state_network_inputs, network_historys);
			potential_state_vals[this->potential_state_networks_target_states[p_index]] = \
				this->potential_state_vals[p_index]->output->acti_vals[0];
			this->potential_state_networks[p_index]->mtx.unlock();
		}
	}
}

void SolutionNodeNormal::backprop_state_networks_state(double* potential_state_errors,
													   bool* potential_states_on,
													   vector<NetworkHistory*>& network_historys) {
	for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
		if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
			NetworkHistory* network_history = network_historys.back();

			this->potential_state_networks[p_index]->mtx.lock();

			network_history->reset_weights();

			vector<double> state_network_errors;
			state_network_errors.push_back(potential_state_errors[
				this->potential_state_networks_target_states[p_index]]);
			this->potential_state_networks[p_index]->backprop(state_network_errors);

			for (int pi_index = 0; pi_index < (int)this->potential_potential_inputs_state_indexes.size(); pi_index++) {
				if (potential_states_on[this->potential_potential_inputs_state_indexes[pi_index]]) {
					if (this->potential_potential_inputs_state_indexes[pi_index]
							== this->potential_state_networks_target_states[p_index]) {
						potential_state_errors[this->potential_potential_inputs_state_indexes[pi_index]] = \
							this->potential_state_networks[p_index]->input->errors[
								1 + this->potential_inputs_state_indexes.size() + pi_index];
					} else {
						potential_state_errors[this->potential_potential_inputs_state_indexes[pi_index]] += \
							this->potential_state_networks[p_index]->input->errors[
								1 + this->potential_inputs_state_indexes.size() + pi_index];
					}
				}

				this->potential_state_networks[p_index]->input->errors[
					1 + this->potential_inputs_state_indexes.size() + pi_index] = 0.0;
			}

			this->potential_state_networks[p_index]->mtx.unlock();

			delete network_history;
			network_historys.pop_back();
		}
	}
}

double SolutionNodeNormal::activate_explore_if_network(Problem& problem,
													   double* state_vals,
													   bool* states_on) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	this->explore_if_network->mtx.lock();
	double score = this->explore_if_network->activate(explore_network_inputs);
	this->explore_if_network->mtx.unlock();

	return score;
}

void SolutionNodeNormal::activate_explore_if_network_eval(Problem& problem,
														  double* state_vals,
														  bool* states_on,
														  vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	this->explore_if_network->mtx.lock();
	this->explore_if_network->activate(explore_network_inputs, network_historys);
	this->explore_if_network->mtx.unlock();
}

void SolutionNodeNormal::backprop_explore_if_network_eval(double score,
														  double* state_errors,
														  bool* states_on,
														  vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_if_network->mtx.lock();

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

	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			state_errors[this->score_network_inputs_state_indexes[i_index]] += \
				this->explore_if_network->input->errors[1+i_index];
		}
		this->explore_if_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_if_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNodeNormal::activate_explore_halt_network(Problem& problem,
														 double* state_vals,
														 bool* states_on) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	this->explore_halt_network->mtx.lock();
	double score = this->explore_halt_network->activate(explore_network_inputs);
	this->explore_halt_network->mtx.unlock();

	return score;
}

void SolutionNodeNormal::activate_explore_halt_network_eval(Problem& problem,
															double* state_vals,
															bool* states_on,
															vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}

	this->explore_halt_network->mtx.lock();
	this->explore_halt_network->activate(explore_network_inputs, network_historys);
	this->explore_halt_network->mtx.unlock();
}

void SolutionNodeNormal::backprop_explore_halt_network_eval(double score,
															double* state_errors,
															bool* states_on,
															vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	this->explore_halt_network->mtx.lock();

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

	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			state_errors[this->score_network_inputs_state_indexes[i_index]] += \
				this->explore_halt_network->input->errors[1+i_index];
		}
		this->explore_halt_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}

double SolutionNodeNormal::activate_explore_no_halt_network(Problem& problem,
															double* state_vals,
															bool* states_on) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}
	this->explore_no_halt_network->mtx.lock();
	double score = this->explore_no_halt_network->activate(explore_network_inputs);
	this->explore_no_halt_network->mtx.unlock();

	return score;
}

void SolutionNodeNormal::activate_explore_no_halt_network_eval(Problem& problem,
															   double* state_vals,
															   bool* states_on,
															   vector<NetworkHistory*>& network_historys) {
	vector<double> explore_network_inputs;
	double curr_observations = problem.get_observation();
	explore_network_inputs.push_back(curr_observations);
	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			score_network_inputs.push_back(state_vals[this->score_network_inputs_state_indexes[i_index]]);
		} else {
			score_network_inputs.push_back(0.0);
		}
	}
	this->explore_no_halt_network->mtx.lock();
	this->explore_no_halt_network->activate(explore_network_inputs, network_historys);
	this->explore_no_halt_network->mtx.unlock();
}

void SolutionNodeNormal::backprop_explore_no_halt_network_eval(double score,
															   double* states_errors,
															   bool* states_on,
															   std::vector<NetworkHistory*>& network_historys) {
	this->explore_no_halt_network->mtx.lock();

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

	for (int i_index = 0; i_index < (int)this->score_network_inputs_state_indexes.size(); i_index++) {
		if (states_on[this->score_network_inputs_state_indexes[i_index]]) {
			state_errors[this->score_network_inputs_state_indexes[i_index]] += \
				this->explore_no_halt_network->input->errors[1+i_index];
		}
		this->explore_no_halt_network->input->errors[1+i_index] = 0.0;
	}

	this->explore_no_halt_network->mtx.unlock();

	delete network_history;
	network_historys.pop_back();
}
