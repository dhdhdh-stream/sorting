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
										   int visited_count,
										   SolutionNode* explore_node,
										   int& explore_type,
										   double* potential_state_vals,
										   bool* potential_states_on,
										   vector<NetworkHistory*>& network_historys) override {
	bool jump_to_explore = false;
	if (explore_node == this) {
		if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
			if (this->explore_start_inclusive == NULL) {
				if (loop_scopes.back() == this) {
					jump_to_explore = true;
				}
			}
		}
	}

	if (!jump_to_explore) {
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

		if (explore_type == EXPLORE_TYPE_STATE) {
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

		problem.perform_action(this->action);

		double score = activate_score_network(problem,
											  state_vals,
											  states_on,
											  explore_type,
											  potential_state_vals,
											  potential_states_on,
											  network_historys);

		if (visited_count == 0 && explore_node == NULL) {
			if (randuni() < (1.0/this->average_future_nodes)) {
				explore_node = this;
				explore_type = EXPLORE_TYPE_PATH;

				if (this->explore_state == EXPLORE_STATE_EXPLORE) {
					int seq_length;
					geometric_distribution<int> seq_length_dist(0.2);

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

						this->explore_type = NODE_NORMAL_EXPLORE_TYPE_JUMP;
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

						this->explore_type = NODE_NORMAL_EXPLORE_TYPE_JUMP;
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
						
						this->explore_type = NODE_NORMAL_EXPLORE_TYPE_LOOP;
					}

					try_path.clear();

					normal_distribution<double> write_val_dist(0.0, 2.0);
					for (int i = 0; i < seq_length; i++) {
						Action a(write_val_dist(generator), rand()%3);
						try_path.push_back(a);
					}
				}
			}
		}
	}

	if (explore_node == this) {
		if (this->explore_state == EXPLORE_STATE_EXPLORE) {
			if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				if (score < this->average) {
					for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
						problem.perform_action(try_path[a_index]);
					}

					return this->explore_end_non_inclusive;
				}
			} else if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				if (loop_scopes.back() == this) {
					if ((loop_scope_count.back() == 3 && rand()%3 == 0)
							(loop_scope_count.back() == 4 && rand()%2 == 0)
							loop_scope_count.back() == 5) {
						loop_scopes.pop_back();
						loop_scope_count.pop_back();

						return this->next;
					} else {
						loop_scope_count.back()++;

						for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
							problem.perform_action(try_path[a_index]);
						}

						return this->explore_start_inclusive;
					}
				} else {
					if (score < this->average) {
						loop_scopes.push_back(this);
						loop_scope_count.push_back(1);

						for (int a_index = 0; a_index < (int)this->try_path.size(); a_index++) {
							problem.perform_action(try_path[a_index]);
						}

						return this->explore_start_inclusive;
					}
				}
			}
		} else if (this->explore_state == EXPLORE_STATE_LEARN) {
			if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				if (score < this->average) {
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
					this->explore_if_network->activate(explore_network_inputs,
													   network_historys);
					this->explore_if_network->mtx.unlock();

					return this->explore_path[0];
				}
			} else if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
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
				
				int current_count;
				if (loop_scopes.back() == this) {
					current_count = loop_scope_count.back();
				} else {
					current_count = 0;
				}
				if (rand()%(6-current_count) == 0) {
					this->explore_halt_network->mtx.lock();
					this->explore_halt_network->activate(explore_network_inputs,
														 network_historys);
					this->explore_halt_network->mtx.unlock();

					if (loop_scopes.back() == this) {
						loop_scopes.pop_back();
						loop_scope_count.pop_back();
					}

					return this->next;
				} else {
					this->explore_no_halt_network->mtx.lock();
					this->explore_no_halt_network->activate(explore_network_inputs,
															network_historys);
					this->explore_no_halt_network->mtx.unlock();

					if (loop_scopes.back() == this) {
						loop_scope_count.back()++;
					} else {
						loop_scopes.push_back(this);
						loop_scope_count.push_back(1);
					}

					return this->explore_path[0];
				}
			}
		}
	}

	return this->next;
}

void SolutionNodeNormal::backprop(double score,
								  SolutionNode* explore_node,
								  int& explore_type,
								  double* potential_state_errors,
								  bool* potential_states_on,
								  std::vector<NetworkHistory*>& network_historys) override {
	if (explore_node == this) {
		if (this->explore_state == EXPLORE_STATE_LEARN) {
			if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				NetworkHistory* network_history = network_historys.back();

				if (network_history->network == this->explore_if_network) {
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

					this->explore_if_network->mtx.unlock();

					delete network_history;
					network_historys.pop_back();
				}
			} else if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				NetworkHistory* network_history = network_historys.back();

				if (network_history->network == this->explore_halt_network) {
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

					this->explore_halt_network->mtx.unlock();

					delete network_history;
					network_historys.pop_back();
				} else if (network_history->network == this->explore_no_halt_network) {
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

					this->explore_no_halt_network->mtx.unlock();

					delete network_history;
					network_historys.pop_back();
				}
			}
		}
	}

	backprop_score_network(score,
						   potential_state_errors,
						   network_historys);

	if (explore_type == EXPLORE_TYPE_STATE) {
		for (int p_index = (int)this->potential_state_networks_target_states.size()-1; p_index >= 0; p_index--) {
			if (potential_states_on[this->potential_state_networks_target_states[p_index]]) {
				this->potential_state_networks[p_index]->mtx.lock();

				NetworkHistory* network_history = network_historys.back();
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
									this->potential_inputs_state_indexes.size() + pi_index];
						} else {
							potential_state_errors[this->potential_potential_inputs_state_indexes[pi_index]] += \
								this->potential_state_networks[p_index]->input->errors[
									this->potential_inputs_state_indexes.size() + pi_index];
						}
					}

					this->potential_state_networks[p_index]->input->errors[
						this->potential_inputs_state_indexes.size() + pi_index] = 0.0;
				}

				this->potential_state_networks[p_index]->mtx.unlock();

				delete network_history;
				network_historys.pop_back();
			}
		}
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
				if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
					curr_node->next = this->explore_end_non_inclusive;
				} else if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
					if (this->explore_start_inclusive == NULL) {
						curr_node->next = this;
					} else {
						curr_node->next = this->explore_start_inclusive;
					}
				}
			}

			int input_size = (int)this->score_network_inputs_state_indexes+1;
			if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_JUMP) {
				this->explore_if_network = new Network(input_size, 4*input_size, 1);
			} else if (this->explore_type == NODE_NORMAL_EXPLORE_TYPE_LOOP) {
				this->explore_halt_network = new Network(input_size, 4*input_size, 1);
				this->explore_no_halt_network = new Network(input_size, 4*input_size, 1);
			}

			this->explore_state = EXPLORE_STATE_LEARN;
			this->explore_iter_index = 0;
		}
	} else if (this->explore_state == EXPLORE_STATE_LEARN) {
		this->explore_iter_index++;

		if (this->explore_iter_index > 2000000) {
			this->explore_state = EXPLORE_STATE_MEASURE;
			this->explore_iter_index = 0;
		}
	}
}
