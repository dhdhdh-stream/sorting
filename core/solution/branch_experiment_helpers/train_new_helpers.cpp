#include "branch_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target == 0) {
		history->instance_count++;

		train_new_target_activate(curr_node,
								  context,
								  exit_depth,
								  exit_node,
								  run_helper);

		uniform_int_distribution<int> until_distribution(0, (int)(this->average_instances_per_run-1.0)/2.0);
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void BranchExperiment::train_new_target_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	/**
	 * - don't need to add history for new branch_node
	 */

	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			curr_node = this->exit_node;
		} else {
			curr_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else {
			curr_node = this->best_scopes[0];
		}
	}
}

void BranchExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	if ((int)this->i_target_val_histories.size() >= NUM_DATAPOINTS) {
		int num_instances = (int)this->i_target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->i_target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / num_instances;

		Eigen::MatrixXd inputs(num_instances, this->input_scope_contexts.size());

		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
				action_node->hook_obs_indexes.push_back(this->input_obs_indexes[i_index]);
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
			input_vals_helper(0,
							  this->input_max_depth,
							  scope_context,
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
				action_node->hook_obs_indexes.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}

		Eigen::VectorXd outputs(num_instances);
		for (int d_index = 0; d_index < num_instances; d_index++) {
			outputs(d_index) = this->i_target_val_histories[d_index] - this->new_average_score;
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
		this->new_linear_weights = vector<double>(this->input_scope_contexts.size());
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
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
			this->new_linear_weights[i_index] = weights(i_index);
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
		this->new_average_misguess = sum_misguesses / num_instances;

		double sum_misguess_variances = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_misguess_variances += (misguesses[d_index] - this->new_average_misguess) * (misguesses[d_index] - this->new_average_misguess);
		}
		this->new_misguess_standard_deviation = sqrt(sum_misguess_variances / num_instances);
		if (this->new_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->new_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		if (this->skip_explore) {
			cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
			cout << "this->new_misguess_standard_deviation: " << this->new_misguess_standard_deviation << endl;
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
			int test_network_input_max_depth = 0;

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
				if ((int)possible_scope_contexts[remaining_indexes[rand_index]].size() > test_network_input_max_depth) {
					test_network_input_max_depth = (int)possible_scope_contexts[remaining_indexes[rand_index]].size();
				}

				remaining_indexes.erase(remaining_indexes.begin() + rand_index);
			}

			Network* test_network;
			if (this->new_network == NULL) {
				test_network = new Network(num_new_input_indexes);
			} else {
				test_network = new Network(this->new_network);

				uniform_int_distribution<int> increment_above_distribution(0, 3);
				if (increment_above_distribution(generator) == 0) {
					test_network->increment_above(num_new_input_indexes);
				} else {
					test_network->increment_side(num_new_input_indexes);
				}
			}

			for (int t_index = 0; t_index < num_new_input_indexes; t_index++) {
				if (test_network_input_node_contexts[t_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)test_network_input_node_contexts[t_index].back();
					action_node->hook_indexes.push_back(t_index);
					action_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					action_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
					action_node->hook_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);
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
				input_vals_helper(0,
								  test_network_input_max_depth,
								  scope_context,
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
					action_node->hook_obs_indexes.clear();
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
			double improvement = this->new_average_misguess - average_misguess;
			double standard_deviation = min(this->new_misguess_standard_deviation, misguess_standard_deviation);
			double t_score = improvement / (standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

			if (t_score > 1.645) {
			#endif /* MDEBUG */
				vector<int> new_input_indexes;
				for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
					int index = -1;
					for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
						if (test_network_input_scope_contexts[t_index] == this->input_scope_contexts[i_index]
								&& test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]
								&& test_network_input_obs_indexes[t_index] == this->input_obs_indexes[i_index]) {
							index = i_index;
							break;
						}
					}
					if (index == -1) {
						this->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
						this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);
						this->input_obs_indexes.push_back(test_network_input_obs_indexes[t_index]);
						if ((int)test_network_input_scope_contexts[t_index].size() > this->input_max_depth) {
							this->input_max_depth = (int)test_network_input_scope_contexts[t_index].size();
						}

						this->existing_linear_weights.push_back(0.0);
						this->new_linear_weights.push_back(0.0);

						index = this->input_scope_contexts.size()-1;
					}
					new_input_indexes.push_back(index);
				}
				this->new_network_input_indexes.push_back(new_input_indexes);

				if (this->new_network != NULL) {
					delete this->new_network;
				}
				this->new_network = test_network;

				this->new_average_misguess = average_misguess;
				this->new_misguess_standard_deviation = misguess_standard_deviation;

				if (this->skip_explore) {
					cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
					cout << "this->new_misguess_standard_deviation: " << this->new_misguess_standard_deviation << endl;
				}

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

		this->combined_score = 0.0;
		this->original_count = 0;
		this->branch_count = 0;

		this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
