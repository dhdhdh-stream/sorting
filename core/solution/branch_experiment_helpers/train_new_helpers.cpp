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
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::train_new_activate(
		vector<int>& context_match_indexes,
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	history->instance_count++;

	bool is_target = false;
	if (!history->has_target) {
		double target_probability;
		if (history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		history->has_target = true;

		train_new_target_activate(context_match_indexes,
								  curr_node,
								  context,
								  run_helper);
	} else {
		if (this->sub_state_iter != 0) {
			train_new_non_target_activate(context_match_indexes,
										  curr_node,
										  context,
										  run_helper);
		}
	}
}

void BranchExperiment::train_new_target_activate(
		vector<int>& context_match_indexes,
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context_match_indexes[0]].scope_history));
	this->i_context_match_indexes_histories.push_back(context_match_indexes);

	BranchNodeHistory* branch_node_history = new BranchNodeHistory(this->branch_node);
	context.back().scope_history->node_histories.push_back(branch_node_history);
	branch_node_history->is_branch = true;

	if (this->throw_id != -1) {
		run_helper.throw_id = -1;
	}

	if (this->best_step_types.size() == 0) {
		curr_node = this->exit_node;
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			curr_node = this->best_existing_scopes[0];
		} else {
			curr_node = this->best_potential_scopes[0];
		}
	}
}

void BranchExperiment::train_new_non_target_activate(
		vector<int>& context_match_indexes,
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.push_back(i_index);
			action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			action_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
			action_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.push_back(i_index);
			branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
			branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			branch_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
			branch_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  true,
					  0,
					  context_match_indexes,
					  input_vals,
					  context[context_match_indexes[0]].scope_history);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
			action_node->hook_indexes.clear();
			action_node->hook_scope_contexts.clear();
			action_node->hook_node_contexts.clear();
			action_node->hook_is_fuzzy_match.clear();
			action_node->hook_strict_root_indexes.clear();
		} else {
			BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
			branch_node->hook_indexes.clear();
			branch_node->hook_scope_contexts.clear();
			branch_node->hook_node_contexts.clear();
			branch_node->hook_is_fuzzy_match.clear();
			branch_node->hook_strict_root_indexes.clear();
		}
	}

	double existing_predicted_score = this->existing_average_score
		+ this->original_bias * this->existing_score_standard_deviation;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
	}
	if (this->existing_network != NULL) {
		vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
			existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
				existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
			}
		}
		this->existing_network->activate(existing_network_input_vals);
		existing_predicted_score += this->existing_network->output->acti_vals[0];
	}

	double new_predicted_score = this->new_average_score;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
	}
	if (this->new_network != NULL) {
		vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
			new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
				new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
			}
		}
		this->new_network->activate(new_network_input_vals);
		new_predicted_score += this->new_network->output->acti_vals[0];
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = new_predicted_score > existing_predicted_score;
	#endif /* MDEBUG */

	BranchNodeHistory* branch_node_history = new BranchNodeHistory(this->branch_node);
	context.back().scope_history->node_histories.push_back(branch_node_history);
	if (decision_is_branch) {
		branch_node_history->is_branch = true;

		if (this->throw_id != -1) {
			run_helper.throw_id = -1;
		}

		if (this->best_step_types.size() == 0) {
			curr_node = this->exit_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
				curr_node = this->best_existing_scopes[0];
			} else {
				curr_node = this->best_potential_scopes[0];
			}
		}
	} else {
		branch_node_history->is_branch = false;
	}
}

void BranchExperiment::train_new_backprop(double target_val,
										  BranchExperimentHistory* history) {
	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (history->has_target) {
		this->i_target_val_histories.push_back(target_val);

		if ((int)this->i_target_val_histories.size() >= solution->curr_num_datapoints) {
			vector<vector<vector<double>>> network_inputs(solution->curr_num_datapoints);
			vector<double> network_target_vals(solution->curr_num_datapoints);
			if (this->sub_state_iter == 0) {
				double sum_scores = 0.0;
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					sum_scores += this->i_target_val_histories[d_index];
				}
				this->new_average_score = sum_scores / solution->curr_num_datapoints;

				uniform_real_distribution<double> bias_distribution(0.0, 1.0);
				this->original_bias = bias_distribution(generator);

				Eigen::MatrixXd inputs(solution->curr_num_datapoints, this->input_scope_contexts.size());

				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
						action_node->hook_indexes.push_back(i_index);
						action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
						action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						action_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
						action_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.push_back(i_index);
						branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
						branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						branch_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
						branch_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
					}
				}
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					vector<double> input_vals(this->input_scope_contexts.size(), 0.0);

					vector<Scope*> scope_context;
					vector<AbstractNode*> node_context;
					input_vals_helper(scope_context,
									  node_context,
									  true,
									  0,
									  this->i_context_match_indexes_histories[d_index],
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
						action_node->hook_is_fuzzy_match.clear();
						action_node->hook_strict_root_indexes.clear();
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.clear();
						branch_node->hook_scope_contexts.clear();
						branch_node->hook_node_contexts.clear();
						branch_node->hook_is_fuzzy_match.clear();
						branch_node->hook_strict_root_indexes.clear();
					}
				}

				Eigen::VectorXd outputs(solution->curr_num_datapoints);
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
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
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						sum_impact_size += abs(inputs(d_index, i_index));
					}
					double average_impact = sum_impact_size / solution->curr_num_datapoints;
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
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					network_target_vals[d_index] = diffs(d_index);
				}

				vector<double> misguesses(solution->curr_num_datapoints);
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					misguesses[d_index] = diffs(d_index) * diffs(d_index);
				}

				double sum_misguesses = 0.0;
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					sum_misguesses += misguesses[d_index];
				}
				this->new_average_misguess = sum_misguesses / solution->curr_num_datapoints;

				double sum_misguess_variances = 0.0;
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					sum_misguess_variances += (misguesses[d_index] - this->new_average_misguess) * (misguesses[d_index] - this->new_average_misguess);
				}
				this->new_misguess_standard_deviation = sqrt(sum_misguess_variances / solution->curr_num_datapoints);
				if (this->new_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
					this->new_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				if (this->skip_explore) {
					cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
					cout << "this->new_misguess_standard_deviation: " << this->new_misguess_standard_deviation << endl;
				}
			} else {
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
						action_node->hook_indexes.push_back(i_index);
						action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
						action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						action_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
						action_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.push_back(i_index);
						branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
						branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
						branch_node->hook_is_fuzzy_match.push_back(this->input_is_fuzzy_match[i_index]);
						branch_node->hook_strict_root_indexes.push_back(this->input_strict_root_indexes[i_index]);
					}
				}
				for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
					vector<double> input_vals(this->input_scope_contexts.size(), 0.0);

					vector<Scope*> scope_context;
					vector<AbstractNode*> node_context;
					input_vals_helper(scope_context,
									  node_context,
									  true,
									  0,
									  this->i_context_match_indexes_histories[d_index],
									  input_vals,
									  this->i_scope_histories[d_index]);

					double linear_impact = 0.0;
					for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
						linear_impact += input_vals[i_index] * this->new_linear_weights[i_index];
					}
					network_target_vals[d_index] = this->i_target_val_histories[d_index] - this->new_average_score - linear_impact;

					network_inputs[d_index] = vector<vector<double>>((int)this->new_network_input_indexes.size());
					for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
						network_inputs[d_index][i_index] = vector<double>((int)this->new_network_input_indexes[i_index].size());
						for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
							network_inputs[d_index][i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
						}
					}
				}
				for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
					if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
						ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
						action_node->hook_indexes.clear();
						action_node->hook_scope_contexts.clear();
						action_node->hook_node_contexts.clear();
						action_node->hook_is_fuzzy_match.clear();
						action_node->hook_strict_root_indexes.clear();
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.clear();
						branch_node->hook_scope_contexts.clear();
						branch_node->hook_node_contexts.clear();
						branch_node->hook_is_fuzzy_match.clear();
						branch_node->hook_strict_root_indexes.clear();
					}
				}

				if (this->new_network != NULL) {
					optimize_network(network_inputs,
									 network_target_vals,
									 this->new_network);

					double average_misguess;
					double misguess_standard_deviation;
					measure_network(network_inputs,
									network_target_vals,
									this->new_network,
									average_misguess,
									misguess_standard_deviation);

					this->new_average_misguess = average_misguess;
					this->new_misguess_standard_deviation = misguess_standard_deviation;

					if (this->skip_explore) {
						cout << "optimize" << endl;
						cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
						cout << "this->new_misguess_standard_deviation: " << this->new_misguess_standard_deviation << endl;
					}
				} else {
					double sum_misguess = 0.0;
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						sum_misguess += network_target_vals[d_index] * network_target_vals[d_index];
					}
					this->new_average_misguess = sum_misguess / solution->curr_num_datapoints;

					double sum_misguess_variance = 0.0;
					for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
						double curr_misguess = network_target_vals[d_index] * network_target_vals[d_index];
						sum_misguess_variance += (curr_misguess - this->new_average_misguess) * (curr_misguess - this->new_average_misguess);
					}
					this->new_misguess_standard_deviation = sqrt(sum_misguess_variance / solution->curr_num_datapoints);
					if (this->new_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
						this->new_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
					}

					if (this->skip_explore) {
						cout << "linear remeasure" << endl;
						cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
						cout << "this->new_misguess_standard_deviation: " << this->new_misguess_standard_deviation << endl;
					}
				}
			}

			vector<vector<Scope*>> possible_scope_contexts;
			vector<vector<AbstractNode*>> possible_node_contexts;
			vector<int> possible_strict_root_indexes;

			vector<Scope*> scope_context;
			vector<AbstractNode*> node_context;
			gather_possible_helper(scope_context,
								   node_context,
								   true,
								   0,
								   this->i_context_match_indexes_histories.back(),
								   possible_scope_contexts,
								   possible_node_contexts,
								   possible_strict_root_indexes,
								   this->i_scope_histories.back());
			/**
			 * - simply always use last ScopeHistory
			 */

			int num_new_input_indexes = min(NETWORK_INCREMENT_NUM_NEW, (int)possible_scope_contexts.size());
			vector<vector<Scope*>> test_network_input_scope_contexts;
			vector<vector<AbstractNode*>> test_network_input_node_contexts;
			vector<bool> test_network_input_is_fuzzy_match;
			vector<int> test_network_input_strict_root_indexes;

			vector<int> remaining_indexes(possible_scope_contexts.size());
			for (int p_index = 0; p_index < (int)possible_scope_contexts.size(); p_index++) {
				remaining_indexes[p_index] = p_index;
			}
			for (int i_index = 0; i_index < num_new_input_indexes; i_index++) {
				uniform_int_distribution<int> distribution(0, (int)remaining_indexes.size()-1);
				int rand_index = distribution(generator);

				uniform_int_distribution<int> is_strict_distribution(0, 2);
				if (is_strict_distribution(generator) == 0) {
					int start_index = this->i_context_match_indexes_histories.back()[possible_strict_root_indexes[remaining_indexes[rand_index]]]
						- this->i_context_match_indexes_histories.back()[0];

					vector<Scope*> new_scope_context(possible_scope_contexts[remaining_indexes[rand_index]].begin() + start_index,
						possible_scope_contexts[remaining_indexes[rand_index]].end());
					vector<AbstractNode*> new_node_context(possible_node_contexts[remaining_indexes[rand_index]].begin() + start_index,
						possible_node_contexts[remaining_indexes[rand_index]].end());

					test_network_input_scope_contexts.push_back(new_scope_context);
					test_network_input_node_contexts.push_back(new_node_context);
					test_network_input_is_fuzzy_match.push_back(false);
					test_network_input_strict_root_indexes.push_back(possible_strict_root_indexes[remaining_indexes[rand_index]]);
				} else {
					vector<Scope*> new_scope_context;
					vector<AbstractNode*> new_node_context;

					uniform_int_distribution<int> exclude_distribution(0, 2);
					for (int l_index = 0; l_index < (int)possible_scope_contexts[remaining_indexes[rand_index]].size()-1; l_index++) {
						if (exclude_distribution(generator) != 0) {
							new_scope_context.push_back(possible_scope_contexts[remaining_indexes[rand_index]][l_index]);
							new_node_context.push_back(possible_node_contexts[remaining_indexes[rand_index]][l_index]);
						}
					}
					new_scope_context.push_back(possible_scope_contexts[remaining_indexes[rand_index]].back());
					new_node_context.push_back(possible_node_contexts[remaining_indexes[rand_index]].back());

					test_network_input_scope_contexts.push_back(new_scope_context);
					test_network_input_node_contexts.push_back(new_node_context);
					test_network_input_is_fuzzy_match.push_back(true);
					test_network_input_strict_root_indexes.push_back(-1);
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
					action_node->hook_is_fuzzy_match.push_back(test_network_input_is_fuzzy_match[t_index]);
					action_node->hook_strict_root_indexes.push_back(test_network_input_strict_root_indexes[t_index]);
				} else {
					BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
					branch_node->hook_indexes.push_back(t_index);
					branch_node->hook_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
					branch_node->hook_node_contexts.push_back(test_network_input_node_contexts[t_index]);
					branch_node->hook_is_fuzzy_match.push_back(test_network_input_is_fuzzy_match[t_index]);
					branch_node->hook_strict_root_indexes.push_back(test_network_input_strict_root_indexes[t_index]);
				}
			}
			for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
				vector<double> test_input_vals(num_new_input_indexes, 0.0);

				vector<Scope*> scope_context;
				vector<AbstractNode*> node_context;
				input_vals_helper(scope_context,
								  node_context,
								  true,
								  0,
								  this->i_context_match_indexes_histories[d_index],
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
					action_node->hook_is_fuzzy_match.clear();
					action_node->hook_strict_root_indexes.clear();
				} else {
					BranchNode* branch_node = (BranchNode*)test_network_input_node_contexts[t_index].back();
					branch_node->hook_indexes.clear();
					branch_node->hook_scope_contexts.clear();
					branch_node->hook_node_contexts.clear();
					branch_node->hook_is_fuzzy_match.clear();
					branch_node->hook_strict_root_indexes.clear();
				}
			}

			train_network(network_inputs,
						  network_target_vals,
						  test_network_input_scope_contexts,
						  test_network_input_node_contexts,
						  test_network_input_is_fuzzy_match,
						  test_network_input_strict_root_indexes,
						  test_network);

			double average_misguess;
			double misguess_standard_deviation;
			measure_network(network_inputs,
							network_target_vals,
							test_network,
							average_misguess,
							misguess_standard_deviation);

			double improvement = this->new_average_misguess - average_misguess;
			double standard_deviation = min(this->new_misguess_standard_deviation, misguess_standard_deviation);
			double t_score = improvement / (standard_deviation / sqrt(solution->curr_num_datapoints * TEST_SAMPLES_PERCENTAGE));
			if (t_score > 2.326) {
				vector<int> new_input_indexes;
				for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
					int index = -1;
					for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
						if (test_network_input_scope_contexts[t_index] == this->input_scope_contexts[i_index]
								&& test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]
								&& test_network_input_is_fuzzy_match[t_index] == this->input_is_fuzzy_match[i_index]
								&& test_network_input_strict_root_indexes[t_index] == this->input_strict_root_indexes[i_index]) {
							index = i_index;
							break;
						}
					}
					if (index == -1) {
						this->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
						this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);
						this->input_is_fuzzy_match.push_back(test_network_input_is_fuzzy_match[t_index]);
						this->input_strict_root_indexes.push_back(test_network_input_strict_root_indexes[t_index]);

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

				this->sub_state_iter = 1;
			} else {
				delete test_network;
			}

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_context_match_indexes_histories.clear();
			this->i_target_val_histories.clear();

			this->sub_state_iter++;
			if (this->sub_state_iter >= BRANCH_EXPERIMENT_TRAIN_ITERS) {
				this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_context_match_indexes_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_RETRAIN_EXISTING;
				/**
				 * - continue this->sub_state_iter
				 */
			}
		}
	}
}
