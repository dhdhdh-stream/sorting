#include "branch_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 5;
#else
const int BRANCH_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void BranchExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	vector<double> input_vals(this->existing_inputs.size());
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		fetch_input_helper(scope_history,
						   this->existing_inputs[i_index],
						   0,
						   input_vals[i_index]);
	}

	vector<double> factor_vals(this->existing_factor_ids.size());
	for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
		fetch_factor_helper(scope_history,
							this->existing_factor_ids[f_index],
							factor_vals[f_index]);
	}

	if (history->is_active) {
		this->num_instances_until_target--;
		if (this->explore_input_histories.size() == this->explore_target_val_histories.size()
				&& this->num_instances_until_target == 0) {
			this->explore_input_histories.push_back(input_vals);
			this->explore_factor_histories.push_back(factor_vals);

			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node;
			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;
					starting_node = obs_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
			int random_index = exit_distribution(generator);
			this->explore_exit_next_nodes.push_back(possible_exits[random_index]);

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.2);
			if (random_index == 0) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			this->explore_step_types.push_back(vector<int>());
			this->explore_actions.push_back(vector<Action>());
			this->explore_scopes.push_back(vector<Scope*>());
			/**
			 * - always give raw actions a large weight
			 *   - existing scopes often learned to avoid certain patterns
			 *     - which can prevent innovation
			 */
			uniform_int_distribution<int> scope_distribution(0, 1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
					this->explore_step_types.back().push_back(STEP_TYPE_SCOPE);
					this->explore_actions.back().push_back(Action());

					uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
					this->explore_scopes.back().push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
				} else {
					this->explore_step_types.back().push_back(STEP_TYPE_ACTION);

					this->explore_actions.back().push_back(problem_type->random_action());

					this->explore_scopes.back().push_back(NULL);
				}
			}

			for (int s_index = 0; s_index < (int)this->explore_step_types.back().size(); s_index++) {
				if (this->explore_step_types.back()[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->explore_actions.back()[s_index]);

					run_helper.num_actions++;
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->explore_scopes.back()[s_index]);
					this->explore_scopes.back()[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}
			}

			curr_node = this->explore_exit_next_nodes.back();
		}
	} else {
		this->existing_input_histories.push_back(input_vals);
		this->existing_factor_histories.push_back(factor_vals);
	}
}

void BranchExperiment::explore_backprop(
		double target_val,
		BranchExperimentHistory* history) {
	uniform_int_distribution<int> until_distribution(0, 2*((int)this->average_instances_per_run-1));
	this->num_instances_until_target = 1 + until_distribution(generator);

	if (history->is_active) {
		if (this->explore_input_histories.size() > this->explore_target_val_histories.size()) {
			this->explore_target_val_histories.push_back(target_val);
		}
	} else {
		while (this->existing_input_histories.size() > this->existing_target_val_histories.size()) {
			this->existing_target_val_histories.push_back(target_val);
		}
	}

	if (this->explore_target_val_histories.size() >= BRANCH_EXPERIMENT_EXPLORE_ITERS) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_input_histories.begin(), this->existing_input_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_factor_histories.begin(), this->existing_factor_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->existing_target_val_histories.begin(), this->existing_target_val_histories.end(), generator_copy);
		}

		int num_instances = (int)this->existing_target_val_histories.size();
		int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
		int num_test_instances = num_instances - num_train_instances;

		double sum_score = 0.0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_score += this->existing_target_val_histories[i_index];
		}
		this->existing_average_score = sum_score / num_instances;

		vector<double> remaining_scores(num_instances);

		if (this->existing_factor_ids.size() > 0) {
			#if defined(MDEBUG) && MDEBUG
			#else
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->existing_target_val_histories[i_index] - this->existing_average_score);
			}
			double average_offset = sum_offset / num_train_instances;
			#endif /* MDEBUG */

			Eigen::MatrixXd inputs(num_train_instances, this->existing_factor_ids.size());
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					inputs(i_index, f_index) = this->existing_factor_histories[i_index][f_index];
				}
			}

			Eigen::VectorXd outputs(num_train_instances);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				outputs(i_index) = this->existing_target_val_histories[i_index] - this->existing_average_score;
			}

			Eigen::VectorXd weights;
			try {
				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			} catch (std::invalid_argument &e) {
				cout << "Eigen error" << endl;
				weights = Eigen::VectorXd(this->existing_factor_ids.size());
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					weights(f_index) = 0.0;
				}
			}

			for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
				this->existing_factor_weights.push_back(weights(f_index));
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
			#endif /* MDEBUG */

			for (int f_index = (int)this->existing_factor_ids.size() - 1; f_index >= 0; f_index--) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double sum_impact = 0.0;
				for (int i_index = 0; i_index < num_train_instances; i_index++) {
					sum_impact += abs(this->existing_factor_histories[i_index][f_index]);
				}

				double impact = abs(this->existing_factor_weights[f_index]) * sum_impact
					/ num_train_instances;
				if (impact < impact_threshold
						|| abs(this->existing_factor_weights[f_index]) > REGRESSION_WEIGHT_LIMIT) {
				#endif /* MDEBUG */
					this->existing_factor_ids.erase(this->existing_factor_ids.begin() + f_index);
					this->existing_factor_weights.erase(this->existing_factor_weights.begin() + f_index);

					for (int i_index = 0; i_index < num_instances; i_index++) {
						this->existing_factor_histories[i_index].erase(this->existing_factor_histories[i_index].begin() + f_index);
					}
					for (int i_index = 0; i_index < (int)this->explore_factor_histories.size(); i_index++) {
						this->explore_factor_histories[i_index].erase(this->explore_factor_histories[i_index].begin() + f_index);
					}
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = 0.0;
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					sum_score += this->existing_factor_weights[f_index]
						* this->existing_factor_histories[i_index][f_index];
				}

				#if defined(MDEBUG) && MDEBUG
				#else
				if (abs(sum_score) > REGRESSION_FAIL_MULTIPLIER * average_offset) {
					delete this;
					return;
				}
				#endif /* MDEBUG */

				remaining_scores[i_index] = this->existing_target_val_histories[i_index]
					- this->existing_average_score - sum_score;
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->existing_target_val_histories[i_index] - this->existing_average_score;
			}
		}

		double sum_misguess = 0.0;
		for (int i_index = num_train_instances; i_index < num_instances; i_index++) {
			sum_misguess += remaining_scores[i_index] * remaining_scores[i_index];
		}
		double average_misguess = sum_misguess / num_test_instances;

		double sum_misguess_variance = 0.0;
		for (int i_index = num_train_instances; i_index < num_instances; i_index++) {
			double curr_misguess = remaining_scores[i_index] * remaining_scores[i_index];
			sum_misguess_variance += (curr_misguess - average_misguess) * (curr_misguess - average_misguess);
		}
		double misguess_standard_deviation = sqrt(sum_misguess_variance / num_test_instances);
		if (misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		Network* existing_network = new Network((int)this->existing_inputs.size(),
												this->existing_input_histories);

		train_network(this->existing_input_histories,
					  remaining_scores,
					  existing_network);

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(this->existing_input_histories,
						remaining_scores,
						existing_network,
						new_average_misguess,
						new_misguess_standard_deviation);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 2.326) {
		#endif /* MDEBUG */
			average_misguess = new_average_misguess;

			for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
				vector<Input> remove_inputs = this->existing_inputs;
				remove_inputs.erase(remove_inputs.begin() + i_index);

				Network* remove_network = new Network(existing_network);
				remove_network->remove_input(i_index);

				vector<vector<double>> existing_remove_input_vals = this->existing_input_histories;
				vector<vector<double>> explore_remove_input_vals = this->explore_input_histories;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					existing_remove_input_vals[d_index].erase(existing_remove_input_vals[d_index].begin() + i_index);
				}
				for (int d_index = 0; d_index < (int)explore_remove_input_vals.size(); d_index++) {
					explore_remove_input_vals[d_index].erase(explore_remove_input_vals[d_index].begin() + i_index);
				}

				optimize_network(existing_remove_input_vals,
								 remaining_scores,
								 remove_network);

				double remove_average_misguess;
				double remove_misguess_standard_deviation;
				measure_network(existing_remove_input_vals,
								remaining_scores,
								remove_network,
								remove_average_misguess,
								remove_misguess_standard_deviation);

				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double remove_improvement = average_misguess - remove_average_misguess;
				double remove_standard_deviation = min(misguess_standard_deviation, remove_misguess_standard_deviation);
				double remove_t_score = remove_improvement / (remove_standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

				if (remove_t_score > -0.674) {
				#endif /* MDEBUG */
					this->existing_inputs = remove_inputs;

					delete existing_network;
					existing_network = remove_network;

					this->existing_input_histories = existing_remove_input_vals;
					this->explore_input_histories = explore_remove_input_vals;
				} else {
					delete remove_network;
				}
			}

			if (this->existing_inputs.size() > 0) {
				Factor* new_factor = new Factor();
				new_factor->inputs = this->existing_inputs;
				new_factor->network = existing_network;
				if (this->node_context->type == NODE_TYPE_OBS) {
					ObsNode* obs_node = (ObsNode*)this->node_context;

					obs_node->factors.push_back(new_factor);

					this->existing_factor_ids.push_back({obs_node->id, (int)obs_node->factors.size()-1});
					this->existing_factor_weights.push_back(1.0);
				} else {
					ObsNode* new_obs_node = new ObsNode();
					new_obs_node->parent = this->scope_context;
					new_obs_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;
					this->scope_context->nodes[new_obs_node->id] = new_obs_node;

					new_obs_node->factors.push_back(new_factor);

					switch (this->node_context->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;

							new_obs_node->next_node_id = action_node->next_node_id;
							new_obs_node->next_node = action_node->next_node;

							for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
								if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
									action_node->next_node->ancestor_ids.erase(
										action_node->next_node->ancestor_ids.begin() + a_index);
									break;
								}
							}
							action_node->next_node->ancestor_ids.push_back(new_obs_node->id);

							action_node->next_node_id = new_obs_node->id;
							action_node->next_node = new_obs_node;

							new_obs_node->ancestor_ids.push_back(action_node->id);
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)this->node_context;

							new_obs_node->next_node_id = scope_node->next_node_id;
							new_obs_node->next_node = scope_node->next_node;

							for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
								if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
									scope_node->next_node->ancestor_ids.erase(
										scope_node->next_node->ancestor_ids.begin() + a_index);
									break;
								}
							}
							scope_node->next_node->ancestor_ids.push_back(new_obs_node->id);

							scope_node->next_node_id = new_obs_node->id;
							scope_node->next_node = new_obs_node;

							new_obs_node->ancestor_ids.push_back(scope_node->id);
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;

							if (this->is_branch) {
								new_obs_node->next_node_id = branch_node->branch_next_node_id;
								new_obs_node->next_node = branch_node->branch_next_node;

								for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
									if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
										branch_node->branch_next_node->ancestor_ids.erase(
											branch_node->branch_next_node->ancestor_ids.begin() + a_index);
										break;
									}
								}
								branch_node->branch_next_node->ancestor_ids.push_back(new_obs_node->id);

								branch_node->branch_next_node_id = new_obs_node->id;
								branch_node->branch_next_node = new_obs_node;
							} else {
								new_obs_node->next_node_id = branch_node->original_next_node_id;
								new_obs_node->next_node = branch_node->original_next_node;

								for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
									if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
										branch_node->original_next_node->ancestor_ids.erase(
											branch_node->original_next_node->ancestor_ids.begin() + a_index);
										break;
									}
								}
								branch_node->original_next_node->ancestor_ids.push_back(new_obs_node->id);

								branch_node->original_next_node_id = new_obs_node->id;
								branch_node->original_next_node = new_obs_node;
							}

							new_obs_node->ancestor_ids.push_back(branch_node->id);
						}
						break;
					}

					new_obs_node->is_init = true;

					this->node_context->experiment = NULL;

					this->node_context = new_obs_node;
					this->is_branch = false;
					this->node_context->experiment = this;

					this->existing_factor_ids.push_back({new_obs_node->id, 0});
					this->existing_factor_weights.push_back(1.0);
				}

				for (int h_index = 0; h_index < (int)this->explore_input_histories.size(); h_index++) {
					existing_network->activate(this->explore_input_histories[h_index]);
					this->explore_factor_histories[h_index].push_back(existing_network->output->acti_vals[0]);
				}
			} else {
				delete existing_network;
			}
		} else {
			delete existing_network;
		}

		double best_surprise = 0.0;
		int best_index = -1;
		for (int e_index = 0; e_index < (int)this->explore_target_val_histories.size(); e_index++) {
			double sum_vals = this->existing_average_score;
			for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
				sum_vals += this->existing_factor_weights[f_index] * this->explore_factor_histories[e_index][f_index];
			}
			double existing_predicted_score = sum_vals;

			double surprise = this->explore_target_val_histories[e_index] - existing_predicted_score;
			if (surprise > best_surprise) {
				best_surprise = surprise;
				best_index = e_index;
			}
		}

		if (best_surprise > 0.0) {
			this->best_step_types = this->explore_step_types[best_index];
			this->best_actions = this->explore_actions[best_index];
			this->best_scopes = this->explore_scopes[best_index];
			this->best_exit_next_node = this->explore_exit_next_nodes[best_index];

			this->existing_inputs.clear();

			this->existing_input_histories.clear();
			this->existing_factor_histories.clear();
			this->existing_target_val_histories.clear();

			this->explore_input_histories.clear();
			this->explore_factor_histories.clear();
			this->explore_step_types.clear();
			this->explore_actions.clear();
			this->explore_scopes.clear();
			this->explore_exit_next_nodes.clear();
			this->explore_target_val_histories.clear();

			this->state = BRANCH_EXPERIMENT_STATE_NEW_GATHER;
			this->state_iter = 0;
		} else {
			delete this;
		}
	}
}
