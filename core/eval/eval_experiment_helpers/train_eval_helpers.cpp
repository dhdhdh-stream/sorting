#include "eval_experiment.h"

using namespace std;

void EvalExperiment::train_eval_helper(vector<double>& existing_target_vals,
									   vector<double>& new_target_vals) {
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
						   this->new_final_scope_histories.back());

	int num_obs = min(LINEAR_NUM_OBS, (int)possible_node_contexts.size());
	{
		vector<int> remaining_indexes(possible_node_contexts.size());
		for (int p_index = 0; p_index < (int)possible_node_contexts.size(); p_index++) {
			remaining_indexes[p_index] = p_index;
		}
		for (int o_index = 0; o_index < num_obs; o_index++) {
			uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
			int rand_index = distribution(generator);

			int index = -1;
			for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
				if (possible_node_contexts[remaining_indexes[rand_index]] == this->eval_input_node_contexts[i_index]
						&& possible_obs_indexes[remaining_indexes[rand_index]] == this->eval_input_obs_indexes[i_index]) {
					index = i_index;
					break;
				}
			}
			if (index == -1) {
				this->eval_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
				this->eval_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
			}

			remaining_indexes.erase(remaining_indexes.begin() + rand_index);
		}
	}

	/**
	 * - simply use first NUM_DATAPOINTS from existing and new
	 *   - and interweave
	 */

	Eigen::MatrixXd inputs(2*NUM_DATAPOINTS, this->eval_input_node_contexts.size());

	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = this->existing_final_scope_histories[d_index];
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->eval_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					inputs(2*d_index, i_index) = 0.0;
					break;
				} else {
					if (curr_layer == (int)this->eval_input_node_contexts[i_index].size()-1) {
						if (it->first->type == NODE_TYPE_ACTION) {
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							inputs(2*d_index, i_index) = action_node_history->obs_snapshot[this->eval_input_obs_indexes[i_index]];
						} else {
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								inputs(2*d_index, i_index) = 1.0;
							} else {
								inputs(2*d_index, i_index) = -1.0;
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

		for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = this->new_final_scope_histories[d_index];
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->eval_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					inputs(2*d_index+1, i_index) = 0.0;
					break;
				} else {
					if (curr_layer == (int)this->eval_input_node_contexts[i_index].size()-1) {
						if (it->first->type == NODE_TYPE_ACTION) {
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							inputs(2*d_index+1, i_index) = action_node_history->obs_snapshot[this->eval_input_obs_indexes[i_index]];
						} else {
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								inputs(2*d_index+1, i_index) = 1.0;
							} else {
								inputs(2*d_index+1, i_index) = -1.0;
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

	Eigen::VectorXd outputs(2*NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		outputs(2*d_index) = this->existing_target_val_histories[d_index];
		outputs(2*d_index+1) = this->new_target_val_histories[d_index];
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		weights = Eigen::VectorXd(this->eval_input_node_contexts.size());
		for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
			weights(i_index) = 0.0;
		}
	}
	this->eval_linear_weights = vector<double>(this->eval_input_node_contexts.size());
	for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
		double sum_impact_size = 0.0;
		for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
			sum_impact_size += abs(inputs(d_index, i_index));
		}
		double average_impact = sum_impact_size / (2*NUM_DATAPOINTS);
		if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * this->original_score_standard_deviation
				|| abs(weights(i_index)) > LINEAR_MAX_WEIGHT) {
			weights(i_index) = 0.0;
		} else {
			weights(i_index) = trunc(1000000 * weights(i_index)) / 1000000;
		}
		this->eval_linear_weights[i_index] = weights(i_index);
	}

	Eigen::VectorXd predicted_scores = inputs * weights;
	Eigen::VectorXd diffs = outputs - predicted_scores;
	vector<double> network_target_vals(2*NUM_DATAPOINTS);
	for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
		network_target_vals[d_index] = diffs(d_index);
	}

	vector<vector<vector<double>>> network_inputs(2*NUM_DATAPOINTS);
	for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
		vector<vector<double>> network_input_vals(this->eval_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->eval_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->eval_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->eval_network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = inputs(d_index, this->eval_network_input_indexes[i_index][v_index]);
			}
		}
		network_inputs[d_index] = network_input_vals;
	}
	
	if (this->eval_network == NULL) {
		vector<double> misguesses(2*NUM_DATAPOINTS);
		for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
			misguesses[d_index] = diffs(d_index) * diffs(d_index);
		}

		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
			sum_misguesses += misguesses[d_index];
		}
		this->eval_average_misguess = sum_misguesses / (2*NUM_DATAPOINTS);

		double sum_misguess_variances = 0.0;
		for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
			sum_misguess_variances += (misguesses[d_index] - this->eval_average_misguess) * (misguesses[d_index] - this->eval_average_misguess);
		}
		this->eval_misguess_standard_deviation = sqrt(sum_misguess_variances / (2*NUM_DATAPOINTS));
		if (this->eval_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->eval_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}
	} else {
		optimize_network(network_inputs,
						 network_target_vals,
						 this->eval_network);

		double average_misguess;
		double misguess_standard_deviation;
		measure_network(network_inputs,
						network_target_vals,
						this->eval_network,
						average_misguess,
						misguess_standard_deviation);

		this->eval_average_misguess = average_misguess;
		this->eval_misguess_standard_deviation = misguess_standard_deviation;
	}

	int train_index = 0;
	while (train_index < 3) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<int> possible_obs_indexes;

		uniform_int_distribution<int> history_distribution(0, (int)this->new_final_scope_histories.size()-1);
		int history_index = history_distribution(generator);

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		gather_possible_helper(scope_context,
							   node_context,
							   possible_scope_contexts,
							   possible_node_contexts,
							   possible_obs_indexes,
							   this->new_final_scope_histories[history_index]);
		/**
		 * - select from new
		 */

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
		if (this->eval_network == NULL) {
			test_network = new Network(num_new_input_indexes);
		} else {
			test_network = new Network(this->eval_network);

			uniform_int_distribution<int> increment_above_distribution(0, 3);
			if (increment_above_distribution(generator) == 0) {
				test_network->increment_above(num_new_input_indexes);
			} else {
				test_network->increment_side(num_new_input_indexes);
			}
		}

		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			{
				vector<double> test_input_vals(num_new_input_indexes, 0.0);
				for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
					int curr_layer = 0;
					ScopeHistory* curr_scope_history = this->existing_final_scope_histories[d_index];
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
				network_inputs[2*d_index].push_back(test_input_vals);
			}

			{
				vector<double> test_input_vals(num_new_input_indexes, 0.0);
				for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
					int curr_layer = 0;
					ScopeHistory* curr_scope_history = this->new_final_scope_histories[d_index];
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
				network_inputs[2*d_index+1].push_back(test_input_vals);
			}
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
		double improvement = this->eval_average_misguess - average_misguess;
		double standard_deviation = min(this->eval_misguess_standard_deviation, misguess_standard_deviation);
		double t_score = improvement / (standard_deviation / sqrt(2*NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

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
				for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
					if (test_network_input_node_contexts[t_index] == this->eval_input_node_contexts[i_index]
							&& test_network_input_obs_indexes[t_index] == this->eval_input_obs_indexes[i_index]) {
						index = i_index;
						break;
					}
				}
				if (index == -1) {
					this->eval_input_node_contexts.push_back(test_network_input_node_contexts[t_index]);
					this->eval_input_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);

					this->eval_linear_weights.push_back(0.0);

					index = this->input_node_contexts.size()-1;
				}
				new_input_indexes.push_back(index);
			}
			this->eval_network_input_indexes.push_back(new_input_indexes);

			if (this->eval_network != NULL) {
				delete this->eval_network;
			}
			this->eval_network = test_network;

			this->eval_average_misguess = average_misguess;
			this->eval_misguess_standard_deviation = misguess_standard_deviation;

			#if defined(MDEBUG) && MDEBUG
			#else
			train_index = 0;
			#endif /* MDEBUG */
		} else {
			delete test_network;

			for (int d_index = 0; d_index < 2*NUM_DATAPOINTS; d_index++) {
				network_inputs[d_index].pop_back();
			}

			train_index++;
		}
	}

	existing_target_vals = vector<double>(NUM_DATAPOINTS);
	new_target_vals = vector<double>(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		{
			this->eval_network->activate(network_inputs[2*d_index]);
			double predicted_score = this->eval_network->output->acti_vals[0];
			existing_target_vals[d_index] = -(predicted_score - this->existing_target_val_histories[d_index]) * (predicted_score - this->existing_target_val_histories[d_index]);
			/**
			 * - reverse signs
			 */
		}
		{
			this->eval_network->activate(network_inputs[2*d_index+1]);
			double predicted_score = this->eval_network->output->acti_vals[0];
			new_target_vals[d_index] = -(predicted_score - this->new_target_val_histories[d_index]) * (predicted_score - this->new_target_val_histories[d_index]);
		}
	}
}
