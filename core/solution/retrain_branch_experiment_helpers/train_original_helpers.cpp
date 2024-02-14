#include "retrain_branch_experiment.h"

#include <cmath>
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::train_original_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	bool is_target = false;
	RetrainBranchExperimentOverallHistory* overall_history = (RetrainBranchExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		train_original_target_activate(is_branch,
									   context);
	} else {
		train_original_non_target_activate(is_branch,
										   context,
										   run_helper);
	}
}

void RetrainBranchExperiment::train_original_target_activate(
		bool& is_branch,
		vector<ContextLayer>& context) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->branch_node->scope_context.size()].scope_history));

	is_branch = false;
}

void RetrainBranchExperiment::train_original_non_target_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->sub_state_iter == 0) {
		vector<double> input_vals(this->branch_node->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
			if (this->branch_node->input_scope_contexts[i_index].size() > 0) {
				if (this->branch_node->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->branch_node->input_node_contexts[i_index].back();
					action_node->hook_indexes.push_back(i_index);
					action_node->hook_scope_contexts.push_back(this->branch_node->input_scope_contexts[i_index]);
					action_node->hook_node_contexts.push_back(this->branch_node->input_node_contexts[i_index]);
				} else {
					BranchNode* branch_node = (BranchNode*)this->branch_node->input_node_contexts[i_index].back();
					branch_node->hook_indexes.push_back(i_index);
					branch_node->hook_scope_contexts.push_back(this->branch_node->input_scope_contexts[i_index]);
					branch_node->hook_node_contexts.push_back(this->branch_node->input_node_contexts[i_index]);
				}
			}
		}
		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		input_vals_helper(scope_context,
						  node_context,
						  input_vals,
						  context[context.size() - this->branch_node->scope_context.size()].scope_history);
		for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
			if (this->branch_node->input_scope_contexts[i_index].size() > 0) {
				if (this->branch_node->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->branch_node->input_node_contexts[i_index].back();
					action_node->hook_indexes.clear();
					action_node->hook_scope_contexts.clear();
					action_node->hook_node_contexts.clear();
				} else {
					BranchNode* branch_node = (BranchNode*)this->branch_node->input_node_contexts[i_index].back();
					branch_node->hook_indexes.clear();
					branch_node->hook_scope_contexts.clear();
					branch_node->hook_node_contexts.clear();
				}
			}
		}

		double original_score = this->branch_node->original_average_score;
		for (int i_index = 0; i_index < (int)this->branch_node->linear_original_input_indexes.size(); i_index++) {
			original_score += input_vals[this->branch_node->linear_original_input_indexes[i_index]] * this->branch_node->linear_original_weights[i_index];
		}
		if (this->branch_node->original_network != NULL) {
			vector<vector<double>> original_network_input_vals(this->branch_node->original_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->branch_node->original_network_input_indexes.size(); i_index++) {
				original_network_input_vals[i_index] = vector<double>(this->original_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->branch_node->original_network_input_indexes[i_index].size(); v_index++) {
					original_network_input_vals[i_index][v_index] = input_vals[this->branch_node->original_network_input_indexes[i_index][v_index]];
				}
			}
			this->branch_node->original_network->activate(original_network_input_vals);
			original_score += this->branch_node->original_network->output->acti_vals[0];
		}

		double branch_score = this->branch_node->branch_average_score;
		for (int i_index = 0; i_index < (int)this->branch_node->linear_branch_input_indexes.size(); i_index++) {
			branch_score += input_vals[this->branch_node->linear_branch_input_indexes[i_index]] * this->branch_node->linear_branch_weights[i_index];
		}
		if (this->branch_node->branch_network != NULL) {
			vector<vector<double>> branch_network_input_vals(this->branch_node->branch_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->branch_node->branch_network_input_indexes.size(); i_index++) {
				branch_network_input_vals[i_index] = vector<double>(this->branch_network_input_indexes[i_index].size());
				for (int v_index = 0; v_index < (int)this->branch_node->branch_network_input_indexes[i_index].size(); v_index++) {
					branch_network_input_vals[i_index][v_index] = input_vals[this->branch_node->branch_network_input_indexes[i_index][v_index]];
				}
			}
			this->branch_node->branch_network->activate(branch_network_input_vals);
			branch_score += this->branch_node->branch_network->output->acti_vals[0];
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (branch_score > original_score) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		#endif /* MDEBUG */
	} else {
		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_scope_contexts[i_index].size() > 0) {
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
		}
		vector<Scope*> scope_context;
		vector<AbstractNode*> node_context;
		input_vals_helper(scope_context,
						  node_context,
						  input_vals,
						  context[context.size() - this->branch_node->scope_context.size()].scope_history);
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			if (this->input_scope_contexts[i_index].size() > 0) {
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
		}

		double original_predicted_score = this->original_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			original_predicted_score += input_vals[i_index] * this->original_linear_weights[i_index];
		}
		if (this->original_network != NULL) {
			vector<vector<double>> original_network_input_vals(this->original_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
				original_network_input_vals[i_index] = vector<double>(this->original_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->original_network_input_indexes[i_index].size(); s_index++) {
					original_network_input_vals[i_index][s_index] = input_vals[this->original_network_input_indexes[i_index][s_index]];
				}
			}
			this->original_network->activate(original_network_input_vals);
			original_predicted_score += this->original_network->output->acti_vals[0];
		}

		double branch_predicted_score = this->branch_average_score;
		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
			branch_predicted_score += input_vals[i_index] * this->branch_linear_weights[i_index];
		}
		if (this->branch_network != NULL) {
			vector<vector<double>> branch_network_input_vals(this->branch_network_input_indexes.size());
			for (int i_index = 0; i_index < (int)this->branch_network_input_indexes.size(); i_index++) {
				branch_network_input_vals[i_index] = vector<double>(this->branch_network_input_indexes[i_index].size());
				for (int s_index = 0; s_index < (int)this->branch_network_input_indexes[i_index].size(); s_index++) {
					branch_network_input_vals[i_index][s_index] = input_vals[this->branch_network_input_indexes[i_index][s_index]];
				}
			}
			this->branch_network->activate(branch_network_input_vals);
			branch_predicted_score += this->branch_network->output->acti_vals[0];
		}

		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		is_branch = branch_predicted_score > original_predicted_score;
		#endif /* MDEBUG */
	}
}

void RetrainBranchExperiment::train_original_backprop(
		double target_val,
		RetrainBranchExperimentOverallHistory* history) {
	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (history->has_target) {
		this->i_target_val_histories.push_back(target_val);

		if ((int)this->i_target_val_histories.size() >= solution->curr_num_datapoints) {
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

			if (this->sub_state_iter == 0) {
				double sum_scores = 0.0;
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					sum_scores += this->i_target_val_histories[d_index];
				}
				this->original_average_score = sum_scores / solution->curr_num_datapoints;

				int num_obs = min(LINEAR_NUM_OBS, (int)possible_scope_contexts.size());

				vector<int> remaining_indexes(possible_scope_contexts.size());
				for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
					remaining_indexes[p_index] = p_index;
				}
				for (int o_index = 0; o_index < num_obs; o_index++) {
					uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
					int rand_index = distribution(generator);

					bool has_obs = false;
					for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
						if (possible_scope_contexts[remaining_indexes[rand_index]] == this->input_scope_contexts[i_index]
								&& possible_node_contexts[remaining_indexes[rand_index]] == this->input_node_contexts[i_index]) {
							has_obs = true;
							break;
						}
					}
					if (!has_obs) {
						this->input_scope_contexts.push_back(possible_scope_contexts[remaining_indexes[rand_index]]);
						this->input_node_contexts.push_back(possible_node_contexts[remaining_indexes[rand_index]]);
					}

					remaining_indexes.erase(remaining_indexes.begin() + rand_index);
				}

				Eigen::MatrixXd inputs(solution->curr_num_datapoints, this->input_scope_contexts.size());
				vector<vector<vector<double>>> network_inputs(solution->curr_num_datapoints);

				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_scope_contexts[i_index].size() > 0) {
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
				}
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
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

					network_inputs[d_index] = vector<vector<double>>((int)this->original_network_input_indexes.size());
					for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
						network_inputs[d_index][i_index] = vector<double>((int)this->original_network_input_indexes[i_index].size());
						for (int s_index = 0; s_index < (int)this->original_network_input_indexes[i_index].size(); s_index++) {
							network_inputs[d_index][i_index][s_index] = input_vals[this->original_network_input_indexes[i_index][s_index]];
						}
					}
				}
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_scope_contexts[i_index].size() > 0) {
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
				}

				Eigen::VectorXd outputs(solution->curr_num_datapoints);
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					outputs(d_index) = this->i_target_val_histories[d_index] - this->original_average_score;
				}

				Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
				this->original_linear_weights = vector<double>(this->input_scope_contexts.size());
				double existing_standard_deviation = sqrt(this->existing_score_variance);
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					double sum_impact_size = 0.0;
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						sum_impact_size += abs(inputs(d_index, i_index));
					}
					double average_impact = sum_impact_size / solution->curr_num_datapoints;
					if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * existing_standard_deviation) {
						weights(i_index) = 0.0;
					}
					this->original_linear_weights[i_index] = weights(i_index);
				}

				Eigen::VectorXd predicted_scores = inputs * weights;
				Eigen::VectorXd diffs = outputs - predicted_scores;

				if (this->original_network == NULL) {
					double sum_misguesses = 0.0;
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						sum_misguesses += diffs(d_index) * diffs(d_index);
					}
					this->original_average_misguess = sum_misguesses / solution->curr_num_datapoints;

					double sum_misguess_variances = 0.0;
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						double curr_misguess = diffs(d_index) * diffs(d_index);
						sum_misguess_variances += (curr_misguess - this->original_average_misguess) * (curr_misguess - this->original_average_misguess);
					}
					this->original_misguess_variance = sum_misguess_variances / solution->curr_num_datapoints;
				} else {
					vector<double> network_target_vals(solution->curr_num_datapoints);
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						network_target_vals[d_index] = diffs(d_index);
					}

					optimize_network(network_inputs,
									 network_target_vals,
									 this->original_network);

					double final_average_misguess;
					double final_misguess_variance;
					measure_network(network_inputs,
									network_target_vals,
									this->original_network,
									final_average_misguess,
									final_misguess_variance);

					this->original_average_misguess = final_average_misguess;
					this->original_misguess_variance = final_misguess_variance;
				}
			} else {
				vector<vector<vector<double>>> network_inputs(solution->curr_num_datapoints);
				vector<double> network_target_vals(solution->curr_num_datapoints);

				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_scope_contexts[i_index].size() > 0) {
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
				}
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					vector<double> input_vals(this->input_scope_contexts.size(), 0.0);

					vector<Scope*> scope_context;
					vector<AbstractNode*> node_context;
					input_vals_helper(scope_context,
									  node_context,
									  input_vals,
									  this->i_scope_histories[d_index]);

					double linear_impact = 0.0;
					for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
						linear_impact += input_vals[i_index] * this->original_linear_weights[i_index];
					}
					network_target_vals[d_index] = this->i_target_val_histories[d_index] - this->original_average_score - linear_impact;

					network_inputs[d_index] = vector<vector<double>>((int)this->original_network_input_indexes.size());
					for (int i_index = 0; i_index < (int)this->original_network_input_indexes.size(); i_index++) {
						network_inputs[d_index][i_index] = vector<double>((int)this->original_network_input_indexes[i_index].size());
						for (int s_index = 0; s_index < (int)this->original_network_input_indexes[i_index].size(); s_index++) {
							network_inputs[d_index][i_index][s_index] = input_vals[this->original_network_input_indexes[i_index][s_index]];
						}
					}
				}
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_scope_contexts[i_index].size() > 0) {
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
				}

				int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW, (int)possible_scope_contexts.size());
				vector<vector<Scope*>> test_network_input_scope_contexts;
				vector<vector<AbstractNode*>> test_network_input_node_contexts;

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

				Network* test_network;
				if (this->original_network == NULL) {
					test_network = new Network(num_new_input_indexes);
				} else {
					test_network = new Network(this->original_network);

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
					} else {
						BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
						branch_node->hook_indexes.push_back(t_index);
						branch_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
						branch_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
					}
				}
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
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

				double improvement = this->original_average_misguess - average_misguess;
				double standard_deviation = min(sqrt(this->original_misguess_variance), sqrt(misguess_variance));
				double t_score = improvement / (standard_deviation / sqrt(solution->curr_num_datapoints * TEST_SAMPLES_PERCENTAGE));
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

							this->original_linear_weights.push_back(0.0);
							this->branch_linear_weights.push_back(0.0);

							index = this->input_scope_contexts.size()-1;
						}
						new_input_indexes.push_back(index);
					}
					this->original_network_input_indexes.push_back(new_input_indexes);

					delete this->original_network;
					this->original_network = test_network;

					this->original_average_misguess = average_misguess;
					this->original_misguess_variance = misguess_variance;
				} else {
					delete test_network;
				}
			}

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_target_val_histories.clear();

			this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_TRAIN_BRANCH;
			/**
			 * - leave this->sub_state_iter unchanged
			 */
		}
	}
}
