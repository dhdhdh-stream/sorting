#include "eval_pass_through_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

void EvalPassThroughExperiment::train_vs() {
	vector<double> vs_target_vals(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		vector<double> input_vals(this->score_input_node_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = this->start_scope_histories[d_index]->node_histories.find(this->score_input_node_contexts[i_index]);
			if (it != this->start_scope_histories[d_index]->node_histories.end()) {
				switch (this->score_input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->score_input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_INFO_SCOPE:
					{
						InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
						if (info_scope_node_history->is_positive) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
						if (info_branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				}
			}
		}

		double score = this->score_average_score;
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			score += input_vals[i_index] * this->score_linear_weights[i_index];
		}
		if (this->score_network != NULL) {
			vector<vector<double>> network_input_vals(this->score_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->score_network_input_indexes.size(); i_index++) {
				network_input_vals[i_index] = vector<double>(this->score_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->score_network_input_indexes[i_index].size(); v_index++) {
					network_input_vals[i_index][v_index] = input_vals[this->score_network_input_indexes[i_index][v_index]];
				}
			}
			this->score_network->activate(network_input_vals);
			score += this->score_network->output->acti_vals[0];
		}

		vs_target_vals[d_index] = this->end_target_val_histories[d_index] - score;
	}

	double sum_scores = 0.0;
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		sum_scores += vs_target_vals[d_index];
	}
	this->vs_average_score = sum_scores / NUM_DATAPOINTS;

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;
	vector<int> possible_obs_indexes;

	ScopeHistory* gather_scope_history;
	uniform_int_distribution<int> history_is_start_distribution(0, 1);
	uniform_int_distribution<int> history_index_distribution(0, NUM_DATAPOINTS - 1);
	if (history_is_start_distribution(generator) == 0) {
		gather_scope_history = this->start_scope_histories[history_index_distribution(generator)];
	} else {
		gather_scope_history = this->end_scope_histories[history_index_distribution(generator)];
	}

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	gather_possible_helper(scope_context,
						   node_context,
						   possible_scope_contexts,
						   possible_node_contexts,
						   possible_obs_indexes,
						   gather_scope_history);

	/**
	 * - simply add for both start and end
	 */
	int num_obs = min(LINEAR_NUM_OBS/2, (int)possible_node_contexts.size());
	{
		vector<int> remaining_indexes(possible_node_contexts.size());
		for (int p_index = 0; p_index < (int)possible_node_contexts.size(); p_index++) {
			remaining_indexes[p_index] = p_index;
		}
		for (int o_index = 0; o_index < num_obs; o_index++) {
			uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
			int rand_index = distribution(generator);

			int start_index = -1;
			for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
				if (this->vs_input_is_start[i_index]
						&& possible_node_contexts[remaining_indexes[rand_index]].back() == this->vs_input_node_contexts[i_index]
						&& possible_obs_indexes[remaining_indexes[rand_index]] == this->vs_input_obs_indexes[i_index]) {
					start_index = i_index;
					break;
				}
			}
			if (start_index == -1) {
				this->vs_input_is_start.push_back(true);
				this->vs_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]].back());
				this->vs_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
			}

			int end_index = -1;
			for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
				if (!this->vs_input_is_start[i_index]
						&& possible_node_contexts[remaining_indexes[rand_index]].back() == this->vs_input_node_contexts[i_index]
						&& possible_obs_indexes[remaining_indexes[rand_index]] == this->vs_input_obs_indexes[i_index]) {
					end_index = i_index;
					break;
				}
			}
			if (end_index == -1) {
				this->vs_input_is_start.push_back(false);
				this->vs_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]].back());
				this->vs_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
			}

			remaining_indexes.erase(remaining_indexes.begin() + rand_index);
		}
	}

	Eigen::MatrixXd inputs(NUM_DATAPOINTS, this->vs_input_node_contexts.size());

	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it;
			bool does_contain;
			if (this->vs_input_is_start[i_index]) {
				it = this->start_scope_histories[d_index]->node_histories.find(this->vs_input_node_contexts[i_index]);
				if (it != this->start_scope_histories[d_index]->node_histories.end()) {
					does_contain = true;
				} else {
					does_contain = false;
				}
			} else {
				it = this->end_scope_histories[d_index]->node_histories.find(this->vs_input_node_contexts[i_index]);
				if (it != this->end_scope_histories[d_index]->node_histories.end()) {
					does_contain = true;
				} else {
					does_contain = false;
				}
			}
			if (!does_contain) {
				inputs(d_index, i_index) = 0.0;
			} else {
				switch (this->vs_input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						inputs(d_index, i_index) = action_node_history->obs_snapshot[this->vs_input_obs_indexes[i_index]];
					}
					break;
				case NODE_TYPE_INFO_SCOPE:
					{
						InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
						if (info_scope_node_history->is_positive) {
							inputs(d_index, i_index) = 1.0;
						} else {
							inputs(d_index, i_index) = -1.0;
						}
					}
					break;
				case NODE_TYPE_INFO_BRANCH:
					{
						InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
						if (info_branch_node_history->is_branch) {
							inputs(d_index, i_index) = 1.0;
						} else {
							inputs(d_index, i_index) = -1.0;
						}
					}
					break;
				}
			}
		}
	}

	Eigen::VectorXd outputs(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		outputs(d_index) = vs_target_vals[d_index] - this->vs_average_score;
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		weights = Eigen::VectorXd(this->vs_input_node_contexts.size());
		for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
			weights(i_index) = 0.0;
		}
	}
	this->vs_linear_weights = vector<double>(this->vs_input_node_contexts.size());
	for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
		double sum_impact_size = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_impact_size += abs(inputs(d_index, i_index));
		}
		double average_impact = sum_impact_size / NUM_DATAPOINTS;
		if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * this->existing_score_standard_deviation
				|| abs(weights(i_index)) > LINEAR_MAX_WEIGHT) {
			weights(i_index) = 0.0;
		} else {
			weights(i_index) = trunc(1000000 * weights(i_index)) / 1000000;
		}
		this->vs_linear_weights[i_index] = weights(i_index);
	}

	Eigen::VectorXd predicted_scores = inputs * weights;
	Eigen::VectorXd diffs = outputs - predicted_scores;
	vector<double> network_target_vals(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		network_target_vals[d_index] = diffs(d_index);
	}

	vector<vector<vector<double>>> network_inputs(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		vector<vector<double>> network_input_vals(this->vs_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->vs_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->vs_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->vs_network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = inputs(d_index, this->vs_network_input_indexes[i_index][v_index]);
			}
		}
		network_inputs[d_index] = network_input_vals;
	}

	if (this->vs_network == NULL) {
		vector<double> misguesses(NUM_DATAPOINTS);
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			misguesses[d_index] = diffs(d_index) * diffs(d_index);
		}

		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_misguesses += misguesses[d_index];
		}
		this->vs_network_average_misguess = sum_misguesses / NUM_DATAPOINTS;

		double sum_misguess_variances = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_misguess_variances += (misguesses[d_index] - this->vs_network_average_misguess) * (misguesses[d_index] - this->vs_network_average_misguess);
		}
		this->vs_network_misguess_standard_deviation = sqrt(sum_misguess_variances / NUM_DATAPOINTS);
		if (this->vs_network_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->vs_network_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}
	} else {
		optimize_network(network_inputs,
						 network_target_vals,
						 this->vs_network);

		double average_misguess;
		double misguess_standard_deviation;
		measure_network(network_inputs,
						network_target_vals,
						this->vs_network,
						average_misguess,
						misguess_standard_deviation);

		this->vs_network_average_misguess = average_misguess;
		this->vs_network_misguess_standard_deviation = misguess_standard_deviation;
	}

	int train_index = 0;
	while (train_index < 3) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<int> possible_obs_indexes;

		ScopeHistory* gather_scope_history;
		uniform_int_distribution<int> history_is_start_distribution(0, 1);
		uniform_int_distribution<int> history_index_distribution(0, NUM_DATAPOINTS - 1);
		if (history_is_start_distribution(generator) == 0) {
			gather_scope_history = this->start_scope_histories[history_index_distribution(generator)];
		} else {
			gather_scope_history = this->end_scope_histories[history_index_distribution(generator)];
		}

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		gather_possible_helper(scope_context,
							   node_context,
							   possible_scope_contexts,
							   possible_node_contexts,
							   possible_obs_indexes,
							   gather_scope_history);

		int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW/2, (int)possible_scope_contexts.size());
		vector<bool> test_network_input_is_start;
		vector<AbstractNode*> test_network_input_node_contexts;
		vector<int> test_network_input_obs_indexes;
		{
			vector<int> remaining_indexes(possible_scope_contexts.size());
			for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
				remaining_indexes[p_index] = p_index;
			}
			for (int i_index = 0; i_index < num_new_input_indexes; i_index++) {
				uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
				int rand_index = distribution(generator);

				test_network_input_is_start.push_back(true);
				test_network_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]].back());
				test_network_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);

				test_network_input_is_start.push_back(false);
				test_network_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]].back());
				test_network_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
			}
		}

		Network* test_network;
		if (this->vs_network == NULL) {
			test_network = new Network(num_new_input_indexes);
		} else {
			test_network = new Network(this->vs_network);

			uniform_int_distribution<int> increment_above_distribution(0, 3);
			if (increment_above_distribution(generator) == 0) {
				test_network->increment_above(num_new_input_indexes);
			} else {
				test_network->increment_side(num_new_input_indexes);
			}
		}

		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			vector<double> test_input_vals(num_new_input_indexes, 0.0);
			for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it;
				bool does_contain;
				if (test_network_input_is_start[t_index]) {
					it = this->start_scope_histories[d_index]->node_histories.find(test_network_input_node_contexts[t_index]);
					if (it != this->start_scope_histories[d_index]->node_histories.end()) {
						does_contain = true;
					} else {
						does_contain = false;
					}
				} else {
					it = this->end_scope_histories[d_index]->node_histories.find(test_network_input_node_contexts[t_index]);
					if (it != this->end_scope_histories[d_index]->node_histories.end()) {
						does_contain = true;
					} else {
						does_contain = false;
					}
				}
				if (does_contain) {
					switch (test_network_input_node_contexts[t_index]->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							test_input_vals[t_index] = action_node_history->obs_snapshot[test_network_input_obs_indexes[t_index]];
						}
						break;
					case NODE_TYPE_INFO_SCOPE:
						{
							InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
							if (info_scope_node_history->is_positive) {
								test_input_vals[t_index] = 1.0;
							} else {
								test_input_vals[t_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								test_input_vals[t_index] = 1.0;
							} else {
								test_input_vals[t_index] = -1.0;
							}
						}
						break;
					}
				}
			}
			network_inputs[d_index].push_back(test_input_vals);
		}

		train_network(network_inputs,
					  network_target_vals,
					  test_network_input_is_start,
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
		double improvement = this->vs_network_average_misguess - average_misguess;
		double standard_deviation = min(this->vs_network_misguess_standard_deviation, misguess_standard_deviation);
		double t_score = improvement / (standard_deviation / sqrt(NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

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
			for (int t_index = 0; t_index < (int)test_network_input_node_contexts.size(); t_index++) {
				int index = -1;
				for (int i_index = 0; i_index < (int)this->vs_input_node_contexts.size(); i_index++) {
					if (test_network_input_is_start[t_index] == this->vs_input_is_start[i_index]
							&& test_network_input_node_contexts[t_index] == this->vs_input_node_contexts[i_index]
							&& test_network_input_obs_indexes[t_index] == this->vs_input_obs_indexes[i_index]) {
						index = i_index;
						break;
					}
				}
				if (index == -1) {
					this->vs_input_is_start.push_back(test_network_input_is_start[t_index]);
					this->vs_input_node_contexts.push_back(test_network_input_node_contexts[t_index]);
					this->vs_input_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);

					this->vs_linear_weights.push_back(0.0);

					index = this->vs_input_node_contexts.size()-1;
				}
				new_input_indexes.push_back(index);
			}
			this->vs_network_input_indexes.push_back(new_input_indexes);

			if (this->vs_network != NULL) {
				delete this->vs_network;
			}
			this->vs_network = test_network;

			this->vs_network_average_misguess = average_misguess;
			this->vs_network_misguess_standard_deviation = misguess_standard_deviation;

			#if defined(MDEBUG) && MDEBUG
			#else
			train_index = 0;
			#endif /* MDEBUG */
		} else {
			delete test_network;

			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
				network_inputs[d_index].pop_back();
			}

			train_index++;
		}
	}
}
