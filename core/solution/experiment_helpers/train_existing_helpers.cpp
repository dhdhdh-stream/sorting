#include "experiment.h"

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
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void Experiment::train_existing_activate(vector<ContextLayer>& context,
										 RunHelper& run_helper,
										 ExperimentHistory* history) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	history->instance_count++;
}

void Experiment::train_existing_backprop(double target_val,
										 RunHelper& run_helper) {
	ExperimentHistory* history = run_helper.experiment_histories.back();

	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (this->parent_experiment == NULL) {
		if (!run_helper.exceeded_limit) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;
			}

			if (run_helper.num_actions > solution->max_num_actions) {
				solution->max_num_actions = run_helper.num_actions;
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
		this->existing_score_standard_deviation = sqrt(sum_score_variance / solution->curr_num_datapoints);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

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
		this->input_max_depth = 0;
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
				this->input_obs_indexes.push_back(possible_obs_indexes[remaining_indexes[rand_index]]);
				if ((int)possible_scope_contexts[remaining_indexes[rand_index]].size() > this->input_max_depth) {
					this->input_max_depth = (int)possible_scope_contexts[remaining_indexes[rand_index]].size();
				}

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

		if (this->skip_explore) {
			cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
			cout << "this->existing_misguess_standard_deviation: " << this->existing_misguess_standard_deviation << endl;
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
					if ((int)possible_scope_contexts[remaining_indexes[rand_index]].size() > test_network_input_max_depth) {
						test_network_input_max_depth = (int)possible_scope_contexts[remaining_indexes[rand_index]].size();
					}

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
			double improvement = this->existing_average_misguess - average_misguess;
			double standard_deviation = min(this->existing_misguess_standard_deviation, misguess_standard_deviation);
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

				if (this->skip_explore) {
					cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
					cout << "this->existing_misguess_standard_deviation: " << this->existing_misguess_standard_deviation << endl;
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

		this->o_target_val_histories.clear();
		for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
			delete this->i_scope_histories[i_index];
		}
		this->i_scope_histories.clear();
		this->i_target_val_histories.clear();

		if (this->skip_explore) {
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					this->actions[s_index]->parent = this->scope_context.back();
					this->actions[s_index]->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;
				} else {
					this->scopes[s_index]->parent = this->scope_context.back();
					this->scopes[s_index]->id = this->scope_context.back()->node_counter;
					this->scope_context.back()->node_counter++;
				}
			}

			int exit_node_id;
			AbstractNode* exit_node;
			if (this->exit_depth > 0
					|| this->exit_throw_id != -1) {
				ExitNode* new_exit_node = new ExitNode();
				new_exit_node->parent = this->scope_context.back();
				new_exit_node->id = this->scope_context.back()->node_counter;
				this->scope_context.back()->node_counter++;

				new_exit_node->exit_depth = this->exit_depth;
				new_exit_node->next_node_parent = this->scope_context[this->scope_context.size()-1 - this->exit_depth];
				if (this->exit_next_node == NULL) {
					new_exit_node->next_node_id = -1;
				} else {
					new_exit_node->next_node_id = this->exit_next_node->id;
				}
				new_exit_node->next_node = this->exit_next_node;
				if (this->exit_throw_id == TEMP_THROW_ID) {
					new_exit_node->throw_id = solution->throw_counter;
					solution->throw_counter++;
				} else {
					new_exit_node->throw_id = this->exit_throw_id;
				}

				this->exit_node = new_exit_node;

				exit_node_id = new_exit_node->id;
				exit_node = new_exit_node;
			} else {
				if (this->exit_next_node == NULL) {
					exit_node_id = -1;
				} else {
					exit_node_id = this->exit_next_node->id;
				}
				exit_node = this->exit_next_node;
			}

			/**
			 * - just need a placeholder for now
			 */
			this->branch_node = new BranchNode();
			this->branch_node->parent = this->scope_context.back();
			this->branch_node->id = this->scope_context.back()->node_counter;
			this->scope_context.back()->node_counter++;

			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				int next_node_id;
				AbstractNode* next_node;
				if (s_index == (int)this->step_types.size()-1) {
					next_node_id = exit_node_id;
					next_node = exit_node;
				} else {
					if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
						next_node_id = this->actions[s_index+1]->id;
						next_node = this->actions[s_index+1];
					} else {
						next_node_id = this->scopes[s_index+1]->id;
						next_node = this->scopes[s_index+1];
					}
				}

				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					this->actions[s_index]->next_node_id = next_node_id;
					this->actions[s_index]->next_node = next_node;
				} else {
					this->scopes[s_index]->next_node_id = next_node_id;
					this->scopes[s_index]->next_node = next_node;

					for (set<int>::iterator it = this->catch_throw_ids[s_index].begin();
							it != this->catch_throw_ids[s_index].end(); it++) {
						this->scopes[s_index]->catch_ids[*it] = next_node_id;
						this->scopes[s_index]->catches[*it] = next_node;
					}
				}
			}

			this->i_scope_histories.reserve(solution->curr_num_datapoints);
			this->i_target_val_histories.reserve(solution->curr_num_datapoints);

			this->state = EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
			this->explore_iter = MAX_EXPLORE_TRIES;
			this->experiment_iter = MAX_EXPERIMENT_NUM_EXPERIMENTS;
		} else {
			uniform_int_distribution<int> explore_distribution(0, 9);
			if (explore_distribution(generator) == 0) {
				this->explore_type = EXPLORE_TYPE_NEUTRAL;
			} else {
				this->explore_type = EXPLORE_TYPE_GOOD;
			}

			this->state = EXPERIMENT_STATE_EXPLORE_CREATE;
			this->state_iter = 0;
			this->explore_iter = 0;
		}
	}
}
