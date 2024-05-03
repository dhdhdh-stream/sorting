#include "new_info_experiment.h"

using namespace std;

void NewInfoExperiment::explore_info_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	run_helper.num_decisions++;

	this->num_instances_until_target--;
	if (this->num_instances_until_target == 0) {
		history->instance_count++;

		vector<ContextLayer> inner_context;
		inner_context.push_back(this->new_info_subscope);

		inner_context.back().scope = this->new_info_subscope;
		inner_context.back().node = NULL;

		ScopeHistory* scope_history = new ScopeHistory(this->new_info_subscope);
		context.back().scope_history = scope_history;

		this->new_info_subscope->activate(problem,
										  inner_context,
										  run_helper,
										  scope_history);

		this->i_scope_histories.push_back(scope_history);

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void NewInfoExperiment::explore_info_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	if (history->instance_count > 0) {
		this->info_score += target_val - this->existing_average_score;

		this->sub_state_iter++;
	}

	for (int h_index = 0; h_index < history->instance_count; h_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	if (this->sub_state_iter == INITIAL_NUM_SAMPLES_PER_ITER) {
		if (this->info_score < 0.0) {
			delete this->new_info_subscope;
			this->new_info_subscope = NULL;

			for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
				delete this->i_scope_histories[h_index];
			}
			this->i_scope_histories.clear();
			this->i_target_val_histories.clear();

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->info_score = 0.0;
				this->new_info_subscope = create_new_info_scope();

				this->i_scope_histories.reserve(NUM_DATAPOINTS);
				this->i_target_val_histories.reserve(NUM_DATAPOINTS);

				this->sub_state_iter = 0;
			}
		}
	} else if (this->sub_state_iter >= NUM_DATAPOINTS) {
		if (this->info_score >= 0.0) {
			int num_instances = (int)this->i_target_val_histories.size();

			vector<vector<Scope*>> possible_scope_contexts;
			vector<vector<AbstractNode*>> possible_node_contexts;
			vector<int> possible_obs_indexes;

			vector<Scope*> scope_context;
			vector<AbstractNode*> node_context;
			gather_possible_helper(scope_context,
								   node_context,
								   possible_scope_contexts,
								   possible_node_contexts,
								   possible_obs_indexes,
								   this->i_scope_histories.back());
			/**
			 * - simply always use last ScopeHistory
			 */

			int num_obs = min(LINEAR_NUM_OBS, (int)possible_scope_contexts.size());
			{
				vector<int> remaining_indexes(possible_scope_contexts.size());
				for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
					remaining_indexes[p_index] = p_index;
				}
				for (int o_index = 0; o_index < num_obs; o_index++) {
					uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
					int rand_index = distribution(generator);

					this->input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
					this->input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);

					remaining_indexes.erase(remaining_indexes.begin() + rand_index);
				}
			}

			Eigen::MatrixXd inputs(num_instances, this->input_node_contexts.size());

			for (int d_index = 0; d_index < num_instances; d_index++) {
				for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
					int curr_layer = 0;
					ScopeHistory* curr_scope_history = this->i_scope_histories[d_index];
					while (true) {
						map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
							this->input_node_contexts[i_index][curr_layer]);
						if (it == curr_scope_history->node_histories.end()) {
							inputs(d_index, i_index) = 0.0;
							break;
						} else {
							if (curr_layer == (int)this->input_node_contexts[i_index].size()-1) {
								if (it->first->type == NODE_TYPE_ACTION) {
									ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
									inputs(d_index, i_index) = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
								} else {
									BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
									if (branch_node_history->is_branch) {
										inputs(d_index, i_index) = 1.0;
									} else {
										inputs(d_index, i_index) = -1.0;
									}
								}
								break;
							} else {
								curr_layer++;
								curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
							}
						}
					}
				}
			}

			Eigen::VectorXd outputs(num_instances);
			for (int d_index = 0; d_index < num_instances; d_index++) {
				outputs(d_index) = this->i_target_val_histories[d_index] - this->existing_average_score;
			}

			Eigen::VectorXd weights;
			try {
				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			} catch (std::invalid_argument &e) {
				cout << "Eigen error" << endl;
				weights = Eigen::VectorXd(this->input_node_contexts.size());
				for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
					weights(i_index) = 0.0;
				}
			}
			this->existing_linear_weights = vector<double>(this->input_node_contexts.size());
			for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
				/**
				 * - don't worry about comparing against new weights
				 *   - may not remove inputs that could be removed
				 *     - but provides more accurate score initially for training networks
				 */
				double sum_impact_size = 0.0;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					sum_impact_size += abs(inputs(d_index, i_index));
				}
				double average_impact = sum_impact_size / num_instances;
				if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * this->existing_score_standard_deviation
						|| abs(weights(i_index)) > LINEAR_MAX_WEIGHT) {
					weights(i_index) = 0.0;
				} else {
					weights(i_index) = trunc(1000000 * weights(i_index)) / 1000000;
				}
				this->existing_linear_weights[i_index] = weights(i_index);
			}

			Eigen::VectorXd predicted_scores = inputs * weights;
			Eigen::VectorXd diffs = outputs - predicted_scores;
			vector<double> network_target_vals(num_instances);
			for (int d_index = 0; d_index < num_instances; d_index++) {
				network_target_vals[d_index] = diffs(d_index);
			}

			vector<double> misguesses(num_instances);
			for (int d_index = 0; d_index < num_instances; d_index++) {
				misguesses[d_index] = diffs(d_index) * diffs(d_index);
			}

			double sum_misguesses = 0.0;
			for (int d_index = 0; d_index < num_instances; d_index++) {
				sum_misguesses += misguesses[d_index];
			}
			this->existing_average_misguess = sum_misguesses / num_instances;

			double sum_misguess_variances = 0.0;
			for (int d_index = 0; d_index < num_instances; d_index++) {
				sum_misguess_variances += (misguesses[d_index] - this->existing_average_misguess) * (misguesses[d_index] - this->existing_average_misguess);
			}
			this->existing_misguess_standard_deviation = sqrt(sum_misguess_variances / num_instances);
			if (this->existing_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
				this->existing_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
			}

			vector<vector<vector<double>>> network_inputs(num_instances);
			int train_index = 0;
			while (train_index < 3) {
				vector<vector<Scope*>> possible_scope_contexts;
				vector<vector<AbstractNode*>> possible_node_contexts;
				vector<int> possible_obs_indexes;

				vector<Scope*> scope_context;
				vector<AbstractNode*> node_context;
				uniform_int_distribution<int> history_distribution(0, num_instances-1);
				gather_possible_helper(scope_context,
									   node_context,
									   possible_scope_contexts,
									   possible_node_contexts,
									   possible_obs_indexes,
									   this->i_scope_histories[history_distribution(generator)]);

				int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW, (int)possible_scope_contexts.size());
				vector<vector<Scope*>> test_network_input_scope_contexts;
				vector<vector<AbstractNode*>> test_network_input_node_contexts;
				vector<int> test_network_input_obs_indexes;
				{
					vector<int> remaining_indexes(possible_scope_contexts.size());
					for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
						remaining_indexes[p_index] = p_index;
					}
					for (int i_index = 0; i_index < num_new_input_indexes; i_index++) {
						uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
						int rand_index = distribution(generator);

						test_network_input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
						test_network_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
						test_network_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);

						remaining_indexes.erase(remaining_indexes.begin() + rand_index);
					}
				}

				Network* test_network;
				if (this->existing_network == NULL) {
					test_network = new Network(num_new_input_indexes);
				} else {
					test_network = new Network(this->existing_network);

					uniform_int_distribution<int> increment_above_distribution(0, 3);
					if (increment_above_distribution(generator) == 0) {
						test_network->increment_above(num_new_input_indexes);
					} else {
						test_network->increment_side(num_new_input_indexes);
					}
				}

				for (int d_index = 0; d_index < num_instances; d_index++) {
					vector<double> test_input_vals(num_new_input_indexes, 0.0);
					for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
						int curr_layer = 0;
						ScopeHistory* curr_scope_history = this->i_scope_histories[d_index];
						while (true) {
							map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
								test_network_input_node_contexts[t_index][curr_layer]);
							if (it == curr_scope_history->node_histories.end()) {
								break;
							} else {
								if (curr_layer == (int)test_network_input_scope_contexts[t_index].size()-1) {
									if (it->first->type == NODE_TYPE_ACTION) {
										ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
										test_input_vals[t_index] = action_node_history->obs_snapshot[test_network_input_obs_indexes[t_index]];
									} else {
										BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
										if (branch_node_history->is_branch) {
											test_input_vals[t_index] = 1.0;
										} else {
											test_input_vals[t_index] = -1.0;
										}
									}
									break;
								} else {
									curr_layer++;
									curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
								}
							}
						}
					}
					network_inputs[d_index].push_back(test_input_vals);
				}

				train_network(network_inputs,
							  network_target_vals,
							  test_network_input_scope_contexts,
							  test_network_input_node_contexts,
							  test_network_input_obs_indexes,
							  test_network);

				double average_misguess;
				double misguess_standard_deviation;
				measure_network(network_inputs,
								network_target_vals,
								test_network,
								average_misguess,
								misguess_standard_deviation);

				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				double improvement = this->existing_average_misguess - average_misguess;
				double standard_deviation = min(this->existing_misguess_standard_deviation, misguess_standard_deviation);
				double t_score = improvement / (standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

				if (t_score > 1.645) {
				#endif /* MDEBUG */
					optimize_network(network_inputs,
									 network_target_vals,
									 test_network);

					measure_network(network_inputs,
									network_target_vals,
									test_network,
									average_misguess,
									misguess_standard_deviation);

					vector<int> new_input_indexes;
					for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
						int index = -1;
						for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
							if (test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]
									&& test_network_input_obs_indexes[t_index] == this->input_obs_indexes[i_index]) {
								index = i_index;
								break;
							}
						}
						if (index == -1) {
							this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);
							this->input_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);

							this->existing_linear_weights.push_back(0.0);

							index = this->input_scope_contexts.size()-1;
						}
						new_input_indexes.push_back(index);
					}
					this->existing_network_input_indexes.push_back(new_input_indexes);

					if (this->existing_network != NULL) {
						delete this->existing_network;
					}
					this->existing_network = test_network;

					this->existing_average_misguess = average_misguess;
					this->existing_misguess_standard_deviation = misguess_standard_deviation;

					#if defined(MDEBUG) && MDEBUG
					#else
					train_index = 0;
					#endif /* MDEBUG */
				} else {
					delete test_network;

					for (int d_index = 0; d_index < num_instances; d_index++) {
						network_inputs[d_index].pop_back();
					}

					train_index++;
				}
			}

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_target_val_histories.clear();

			this->i_scope_histories.reserve(NUM_DATAPOINTS);
			this->i_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state = NEW_INFO_EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		} else {
			delete this->new_info_subscope;
			this->new_info_subscope = NULL;

			for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
				delete this->i_scope_histories[h_index];
			}
			this->i_scope_histories.clear();
			this->i_target_val_histories.clear();

			this->state_iter++;
			if (this->state_iter >= EXPLORE_ITERS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->info_score = 0.0;
				this->new_info_subscope = create_new_info_scope();

				this->i_scope_histories.reserve(NUM_DATAPOINTS);
				this->i_target_val_histories.reserve(NUM_DATAPOINTS);

				this->sub_state_iter = 0;
			}
		}
	}
}
