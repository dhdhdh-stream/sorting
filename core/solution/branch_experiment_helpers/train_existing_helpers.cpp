#include "branch_experiment.h"

#include <iostream>
/**
 * - stability issue with Eigen BDCSVD for small singular values
 */
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::train_existing_activate(vector<ContextLayer>& context,
											   RunHelper& run_helper) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
	overall_history->instance_count++;
}

void BranchExperiment::train_existing_backprop(double target_val,
											   RunHelper& run_helper,
											   BranchExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (this->parent_pass_through_experiment == NULL) {
		if (!run_helper.exceeded_limit) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;
			}
		}
	}

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / solution->curr_num_datapoints;

		int num_instances = (int)this->i_target_val_histories.size();

		/**
		 * - parent_pass_through_experiment exceeded_limit edge case
		 *   - 1 for train, 1 for test
		 */
		if (num_instances < 2) {
			this->result = EXPERIMENT_RESULT_FAIL;
			return;
		}

		vector<vector<Scope*>> possible_scope_contexts;
		vector<vector<AbstractNode*>> possible_node_contexts;

		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		gather_possible_helper(scope_context,
							   node_context,
							   possible_scope_contexts,
							   possible_node_contexts,
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

				this->input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
				this->input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
			}
		}

		Eigen::MatrixXd inputs(num_instances, this->input_scope_contexts.size());

		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.push_back(i_index);
				branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			}
		}
		for (int d_index = 0; d_index < num_instances; d_index++) {
			vector<double> input_vals(this->input_scope_contexts.size(), 0.0);

			vector<Scope*> scope_context;
			vector<AbstractNode*> node_context;
			input_vals_helper(scope_context,
							  node_context,
							  input_vals,
							  this->i_scope_histories[d_index]);

			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				inputs(d_index, i_index) = input_vals[i_index];
			}
		}
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
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
			weights = Eigen::VectorXd(this->input_scope_contexts.size());
			for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
				weights(i_index) = 0.0;
			}
		}
		this->existing_linear_weights = vector<double>(this->input_scope_contexts.size());
		double existing_standard_deviation = sqrt(this->existing_score_variance);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
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
			if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * existing_standard_deviation) {
				weights(i_index) = 0.0;
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
		this->existing_misguess_variance = sum_misguess_variances / num_instances;

		int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW, (int)possible_scope_contexts.size());
		vector<vector<Scope*>> test_network_input_scope_contexts;
		vector<vector<AbstractNode*>> test_network_input_node_contexts;
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

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
			}
		}

		Network* test_network = new Network(num_new_input_indexes);

		vector<vector<vector<double>>> network_inputs(num_instances);

		for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
			if (test_network_input_node_contexts[t_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)test_network_input_node_contexts[t_index].back();
				action_node->hook_indexes.push_back(t_index);
				action_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
				action_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
				branch_node->hook_indexes.push_back(t_index);
				branch_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
				branch_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
			}
		}
		for (int d_index = 0; d_index < num_instances; d_index++) {
			vector<double> test_input_vals(num_new_input_indexes, 0.0);

			vector<Scope*> scope_context;
			vector<AbstractNode*> node_context;
			input_vals_helper(scope_context,
							  node_context,
							  test_input_vals,
							  this->i_scope_histories[d_index]);

			network_inputs[d_index].push_back(test_input_vals);
		}
		for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
			if (test_network_input_node_contexts[t_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)test_network_input_node_contexts[t_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}

		train_network(network_inputs,
					  network_target_vals,
					  test_network_input_scope_contexts,
					  test_network_input_node_contexts,
					  test_network);

		double average_misguess;
		double misguess_variance;
		measure_network(network_inputs,
						network_target_vals,
						test_network,
						average_misguess,
						misguess_variance);

		double improvement = this->existing_average_misguess - average_misguess;
		double standard_deviation = min(sqrt(this->existing_misguess_variance), sqrt(misguess_variance));
		double t_score = improvement / (standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));
		if (t_score > 2.326) {
			vector<int> new_input_indexes;
			for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
				int index = -1;
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (test_network_input_scope_contexts[t_index] == this->input_scope_contexts[i_index]
							&& test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]) {
						index = i_index;
						break;
					}
				}
				if (index == -1) {
					this->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);

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
			this->existing_misguess_variance = misguess_variance;
		} else {
			delete test_network;
		}

		this->o_target_val_histories.clear();
		for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
			delete this->i_scope_histories[i_index];
		}
		this->i_scope_histories.clear();
		this->i_target_val_histories.clear();

		this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
