#include "branch_experiment.h"

#include <cmath>
#include <iostream>
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
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	bool is_target = false;
	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
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

		train_new_target_activate(curr_node,
								  problem,
								  context,
								  exit_depth,
								  exit_node,
								  run_helper,
								  history);
	} else {
		if (this->sub_state_iter != 0) {
			train_new_non_target_activate(curr_node,
										  problem,
										  context,
										  exit_depth,
										  exit_node,
										  run_helper,
										  history);
		}
	}
}

void BranchExperiment::train_new_target_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
	history = instance_history;

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			instance_history->step_histories.push_back(action_node_history);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
		} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_existing_scopes[s_index]);
			instance_history->step_histories.push_back(scope_node_history);
			this->best_existing_scopes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_potential_scopes[s_index]);
			instance_history->step_histories.push_back(scope_node_history);
			this->best_potential_scopes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node = this->best_exit_node;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node = this->best_exit_node;
	}
}

void BranchExperiment::train_new_non_target_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
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
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->scope_context.size()].scope_history);
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

	double existing_predicted_score = this->existing_average_score;
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

	if (decision_is_branch) {
		BranchExperimentInstanceHistory* instance_history = new BranchExperimentInstanceHistory(this);
		history = instance_history;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
				instance_history->step_histories.push_back(action_node_history);
				this->best_actions[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					action_node_history);
			} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_existing_scopes[s_index]);
				instance_history->step_histories.push_back(scope_node_history);
				this->best_existing_scopes[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					scope_node_history);
			} else {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_potential_scopes[s_index]);
				instance_history->step_histories.push_back(scope_node_history);
				this->best_potential_scopes[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					scope_node_history);
			}
		}

		if (this->best_exit_depth == 0) {
			curr_node = this->best_exit_node;
		} else {
			exit_depth = this->best_exit_depth-1;
			exit_node = this->best_exit_node;
		}
	}
}

void BranchExperiment::train_new_backprop(double target_val,
										  BranchExperimentOverallHistory* history) {
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

				Eigen::MatrixXd inputs(solution->curr_num_datapoints, this->input_scope_contexts.size());

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
					if (abs(weights(i_index)) * average_impact < WEIGHT_MIN_SCORE_IMPACT * this->existing_score_standard_deviation) {
						weights(i_index) = 0.0;
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
			} else {
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
					} else {
						BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
						branch_node->hook_indexes.clear();
						branch_node->hook_scope_contexts.clear();
						branch_node->hook_node_contexts.clear();
					}
				}
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
								&& test_network_input_node_contexts[t_index] == this->input_node_contexts[i_index]) {
							index = i_index;
							break;
						}
					}
					if (index == -1) {
						this->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
						this->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);

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
			} else {
				delete test_network;
			}

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_target_val_histories.clear();

			this->sub_state_iter++;
			if (this->sub_state_iter >= BRANCH_EXPERIMENT_TRAIN_ITERS) {
				this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_RETRAIN_EXISTING;
				/**
				 * - continue this->sub_state_iter
				 */
			}
		}
	}
}
