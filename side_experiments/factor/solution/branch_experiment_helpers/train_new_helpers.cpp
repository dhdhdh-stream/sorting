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
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		run_helper.has_explore = true;

		history->instance_count++;

		map<pair<int,int>, double> factors;
		gather_factors(run_helper,
					   scope_history,
					   factors);
		double sum_vals = this->existing_average_score;
		for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
			map<pair<int,int>, double>::iterator it = factors.find(this->existing_factor_ids[f_index]);
			if (it != factors.end()) {
				sum_vals += this->existing_factor_weights[f_index] * it->second;
			}
		}
		history->existing_predicted_scores.push_back(sum_vals);

		vector<double> input_vals(this->new_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			fetch_input_helper(run_helper,
							   scope_history,
							   this->new_inputs[i_index],
							   0,
							   input_vals[i_index]);
		}
		this->input_histories.push_back(input_vals);

		this->factor_histories.push_back(factors);

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_actions[s_index]);
			} else {
				context.back().node_id = -1;

				ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
				this->best_scopes[s_index]->activate(problem,
					context,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->best_exit_next_node;

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);
	}
}

void BranchExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_history;

	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->input_histories.begin(), this->input_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->factor_histories.begin(), this->factor_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->i_target_val_histories.begin(), this->i_target_val_histories.end(), generator_copy);
		}

		int num_instances = (int)this->i_target_val_histories.size();
		int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
		int num_test_instances = num_instances - num_train_instances;

		double sum_score = 0.0;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			sum_score += this->i_target_val_histories[i_index];
		}
		this->new_average_score = sum_score / num_instances;

		map<pair<int,int>, double> sum_factor_impacts;
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			for (map<pair<int,int>, double>::iterator it = this->factor_histories[i_index].begin();
					it != this->factor_histories[i_index].end(); it++) {
				map<pair<int,int>, double>::iterator s_it = sum_factor_impacts.find(it->first);
				if (s_it == sum_factor_impacts.end()) {
					sum_factor_impacts[it->first] = abs(it->second);
				} else {
					s_it->second += abs(it->second);
				}
			}
		}

		vector<double> remaining_scores(num_instances);

		int num_factors = (int)sum_factor_impacts.size();
		if (num_factors > 0) {
			#if defined(MDEBUG) && MDEBUG
			#else
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->i_target_val_histories[i_index] - this->existing_average_score);
			}
			double average_offset = sum_offset / num_train_instances;
			#endif /* MDEBUG */

			map<pair<int,int>, int> factor_mapping;
			for (map<pair<int,int>, double>::iterator it = sum_factor_impacts.begin();
					it != sum_factor_impacts.end(); it++) {
				int index = (int)factor_mapping.size();
				factor_mapping[it->first] = index;
			}

			Eigen::MatrixXd inputs(num_train_instances, num_factors);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				for (int f_index = 0; f_index < num_factors; f_index++) {
					inputs(i_index, f_index) = 0.0;
				}

				for (map<pair<int,int>, double>::iterator it = this->factor_histories[i_index].begin();
						it != this->factor_histories[i_index].end(); it++) {
					inputs(i_index, factor_mapping[it->first]) = it->second;
				}
			}

			Eigen::VectorXd outputs(num_train_instances);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				outputs(i_index) = this->i_target_val_histories[i_index] - this->new_average_score;
			}

			Eigen::VectorXd weights;
			try {
				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			} catch (std::invalid_argument &e) {
				cout << "Eigen error" << endl;
				weights = Eigen::VectorXd(num_factors);
				for (int f_index = 0; f_index < num_factors; f_index++) {
					weights(f_index) = 0.0;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			#else
			double impact_threshold = average_offset * FACTOR_IMPACT_THRESHOLD;
			#endif /* MDEBUG */

			for (map<pair<int,int>, double>::iterator it = sum_factor_impacts.begin();
					it != sum_factor_impacts.end(); it++) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double impact = abs(weights(factor_mapping[it->first])) * sum_factor_impacts[it->first]
					/ num_train_instances;
				if (impact > impact_threshold) {
				#endif /* MDEBUG */
					this->new_factor_ids.push_back(it->first);
					this->new_factor_weights.push_back(weights(factor_mapping[it->first]));
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = this->new_average_score;
				for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
					map<pair<int,int>, double>::iterator it = this->factor_histories[i_index]
						.find(this->new_factor_ids[f_index]);
					if (it != this->factor_histories[i_index].end()) {
						sum_score += this->new_factor_weights[f_index] * it->second;
					}
				}

				remaining_scores[i_index] = this->i_target_val_histories[i_index] - sum_score;
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->i_target_val_histories[i_index] - this->new_average_score;
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

		Network* new_network = new Network((int)this->new_inputs.size());

		train_network(this->input_histories,
					  remaining_scores,
					  new_network);

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(this->input_histories,
						remaining_scores,
						new_network,
						new_average_misguess,
						new_misguess_standard_deviation);

		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 2.326) {
			average_misguess = new_average_misguess;

			for (int i_index = (int)this->new_inputs.size()-1; i_index >= 0; i_index--) {
				vector<pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>> remove_inputs = this->new_inputs;
				remove_inputs.erase(remove_inputs.begin() + i_index);

				Network* remove_network = new Network(new_network);
				remove_network->remove_input(i_index);

				vector<vector<double>> remove_input_vals = this->input_histories;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					remove_input_vals[d_index].erase(remove_input_vals[d_index].begin() + i_index);
				}

				optimize_network(remove_input_vals,
								 remaining_scores,
								 remove_network);

				double remove_average_misguess;
				double remove_misguess_standard_deviation;
				measure_network(remove_input_vals,
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
					this->new_inputs = remove_inputs;

					delete new_network;
					new_network = remove_network;

					this->input_histories = remove_input_vals;
				} else {
					delete remove_network;
				}
			}

			if (this->new_inputs.size() > 1) {
				Factor* new_factor = new Factor();
				new_factor->inputs = this->new_inputs;
				new_factor->network = new_network;
				if (this->node_context->type == NODE_TYPE_OBS) {
					ObsNode* obs_node = (ObsNode*)this->node_context;

					obs_node->factors.push_back(new_factor);

					this->new_factor_ids.push_back({obs_node->id, (int)obs_node->factors.size()-1});
					this->new_factor_weights.push_back(1.0);
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

					new_obs_node->average_instances_per_run = this->node_context->average_instances_per_run;

					new_obs_node->was_commit = this->node_context->was_commit;

					new_obs_node->num_measure = this->node_context->num_measure;
					new_obs_node->sum_score = this->node_context->sum_score;

					int experiment_index;
					for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
						if (this->node_context->experiments[e_index] == this) {
							experiment_index = e_index;
							break;
						}
					}
					this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);

					this->node_context = new_obs_node;
					this->node_context->experiments.push_back(this);

					this->new_factor_ids.push_back({new_obs_node->id, 0});
					this->new_factor_weights.push_back(1.0);
				}
			} else {
				delete new_network;
			}
		} else {
			delete new_network;
		}

		this->input_histories.clear();
		this->factor_histories.clear();
		this->i_target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
