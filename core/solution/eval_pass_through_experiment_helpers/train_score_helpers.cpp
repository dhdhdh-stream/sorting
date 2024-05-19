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

void EvalPassThroughExperiment::train_score() {
	vector<double> combined_target_val_histories;
	vector<ScopeHistory*> combined_scope_histories;
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		combined_target_val_histories.push_back(this->start_target_val_histories[d_index]);
		combined_scope_histories.push_back(this->start_scope_histories[d_index]);

		combined_target_val_histories.push_back(this->end_target_val_histories[d_index]);
		combined_scope_histories.push_back(this->end_scope_histories[d_index]);
	}

	double sum_scores = 0.0;
	for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
		sum_scores += combined_target_val_histories[d_index];
	}
	this->score_average_score = sum_scores / (2 * NUM_DATAPOINTS);

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;
	vector<int> possible_obs_indexes;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	uniform_int_distribution<int> history_distribution(0, 2 * NUM_DATAPOINTS - 1);
	gather_possible_helper(scope_context,
						   node_context,
						   possible_scope_contexts,
						   possible_node_contexts,
						   possible_obs_indexes,
						   combined_scope_histories[history_distribution(generator)]);

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
			for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
				if (possible_node_contexts[remaining_indexes[rand_index]].back() == this->score_input_node_contexts[i_index]
						&& possible_obs_indexes[remaining_indexes[rand_index]] == this->score_input_obs_indexes[i_index]) {
					index = i_index;
					break;
				}
			}
			if (index == -1) {
				this->score_input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]].back());
				this->score_input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
			}

			remaining_indexes.erase(remaining_indexes.begin() + rand_index);
		}
	}

	Eigen::MatrixXd inputs(2 * NUM_DATAPOINTS, this->score_input_node_contexts.size());

	for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = combined_scope_histories[d_index]->node_histories.find(
				this->score_input_node_contexts[i_index]);
			if (it == combined_scope_histories[d_index]->node_histories.end()) {
				inputs(d_index, i_index) = 0.0;
			} else {
				switch (this->score_input_node_contexts[i_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						inputs(d_index, i_index) = action_node_history->obs_snapshot[this->score_input_obs_indexes[i_index]];
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

	Eigen::VectorXd outputs(2 * NUM_DATAPOINTS);
	for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
		outputs(d_index) = combined_target_val_histories[d_index] - this->score_average_score;
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		weights = Eigen::VectorXd(this->score_input_node_contexts.size());
		for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
			weights(i_index) = 0.0;
		}
	}
	this->score_linear_weights = vector<double>(this->score_input_node_contexts.size());
	for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
		double sum_impact_size = 0.0;
		for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
			sum_impact_size += abs(inputs(d_index, i_index));
		}
		double average_impact = sum_impact_size / (2 * NUM_DATAPOINTS);
		if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * this->existing_score_standard_deviation
				|| abs(weights(i_index)) > LINEAR_MAX_WEIGHT) {
			weights(i_index) = 0.0;
		} else {
			weights(i_index) = trunc(1000000 * weights(i_index)) / 1000000;
		}
		this->score_linear_weights[i_index] = weights(i_index);
	}

	Eigen::VectorXd predicted_scores = inputs * weights;
	Eigen::VectorXd diffs = outputs - predicted_scores;
	vector<double> network_target_vals(2 * NUM_DATAPOINTS);
	for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
		network_target_vals[d_index] = diffs(d_index);
	}

	vector<vector<vector<double>>> network_inputs(2 * NUM_DATAPOINTS);
	for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
		vector<vector<double>> network_input_vals(this->score_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->score_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->score_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->score_network_input_indexes[i_index].size(); v_index++) {
				network_input_vals[i_index][v_index] = inputs(d_index, this->score_network_input_indexes[i_index][v_index]);
			}
		}
		network_inputs[d_index] = network_input_vals;
	}

	if (this->score_network == NULL) {
		vector<double> misguesses(2 * NUM_DATAPOINTS);
		for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
			misguesses[d_index] = diffs(d_index) * diffs(d_index);
		}

		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
			sum_misguesses += misguesses[d_index];
		}
		this->score_network_average_misguess = sum_misguesses / (2 * NUM_DATAPOINTS);

		double sum_misguess_variances = 0.0;
		for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
			sum_misguess_variances += (misguesses[d_index] - this->score_network_average_misguess) * (misguesses[d_index] - this->score_network_average_misguess);
		}
		this->score_network_misguess_standard_deviation = sqrt(sum_misguess_variances / (2 * NUM_DATAPOINTS));
		if (this->score_network_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->score_network_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}
	} else {
		optimize_network(network_inputs,
						 network_target_vals,
						 this->score_network);

		double average_misguess;
		double misguess_standard_deviation;
		measure_network(network_inputs,
						network_target_vals,
						this->score_network,
						average_misguess,
						misguess_standard_deviation);

		this->score_network_average_misguess = average_misguess;
		this->score_network_misguess_standard_deviation = misguess_standard_deviation;
	}

	int train_index = 0;
	while (train_index < 3) {
		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;
		vector<int> possible_obs_indexes;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		uniform_int_distribution<int> history_distribution(0, 2 * NUM_DATAPOINTS - 1);
		gather_possible_helper(scope_context,
							   node_context,
							   possible_scope_contexts,
							   possible_node_contexts,
							   possible_obs_indexes,
							   combined_scope_histories[history_distribution(generator)]);

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
		if (this->score_network == NULL) {
			test_network = new Network(num_new_input_indexes);
		} else {
			test_network = new Network(this->score_network);

			uniform_int_distribution<int> increment_above_distribution(0, 3);
			if (increment_above_distribution(generator) == 0) {
				test_network->increment_above(num_new_input_indexes);
			} else {
				test_network->increment_side(num_new_input_indexes);
			}
		}

		for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
			vector<double> test_input_vals(num_new_input_indexes, 0.0);
			for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = combined_scope_histories[d_index]->node_histories.find(
					test_network_input_node_contexts[t_index].back());
				if (it != combined_scope_histories[d_index]->node_histories.end()) {
					switch (test_network_input_node_contexts[t_index].back()->type) {
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
		double improvement = this->score_network_average_misguess - average_misguess;
		double standard_deviation = min(this->score_network_misguess_standard_deviation, misguess_standard_deviation);
		double t_score = improvement / (standard_deviation / sqrt(2 * NUM_DATAPOINTS * TEST_SAMPLES_PERCENTAGE));

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
				for (int i_index = 0; i_index < (int)this->score_input_node_contexts.size(); i_index++) {
					if (test_network_input_node_contexts[t_index].back() == this->score_input_node_contexts[i_index]
							&& test_network_input_obs_indexes[t_index] == this->score_input_obs_indexes[i_index]) {
						index = i_index;
						break;
					}
				}
				if (index == -1) {
					this->score_input_node_contexts.push_back(test_network_input_node_contexts[t_index].back());
					this->score_input_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);

					this->score_linear_weights.push_back(0.0);

					index = this->score_input_node_contexts.size()-1;
				}
				new_input_indexes.push_back(index);
			}
			this->score_network_input_indexes.push_back(new_input_indexes);

			if (this->score_network != NULL) {
				delete this->score_network;
			}
			this->score_network = test_network;

			this->score_network_average_misguess = average_misguess;
			this->score_network_misguess_standard_deviation = misguess_standard_deviation;

			#if defined(MDEBUG) && MDEBUG
			#else
			train_index = 0;
			#endif /* MDEBUG */
		} else {
			delete test_network;

			for (int d_index = 0; d_index < 2 * NUM_DATAPOINTS; d_index++) {
				network_inputs[d_index].pop_back();
			}

			train_index++;
		}
	}
}
