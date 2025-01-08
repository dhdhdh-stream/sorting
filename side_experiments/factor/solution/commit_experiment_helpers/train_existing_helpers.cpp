#include "commit_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

using namespace std;

const int TRAIN_EXISTING_NUM_DATAPOINTS = 2000;

void CommitExperiment::train_existing_activate(
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		CommitExperimentHistory* history) {
	history->instance_count++;

	vector<double> input_vals(this->existing_inputs.size());
	for (int i_index = 0; i_index < (int)this->existing_inputs.size(); i_index++) {
		fetch_input_helper(run_helper,
						   scope_history,
						   this->existing_inputs[i_index],
						   0,
						   input_vals[i_index]);
	}
	this->input_histories.push_back(input_vals);

	map<pair<int,int>, double> factors;
	gather_factors(run_helper,
				   scope_history,
				   factors);
	this->factor_histories.push_back(factors);
}

void CommitExperiment::train_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	CommitExperimentHistory* history = (CommitExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}
	this->o_target_val_histories.push_back(target_val);

	this->state_iter++;
	if (this->state_iter >= TRAIN_EXISTING_NUM_DATAPOINTS) {
		double sum_score = 0.0;
		for (int i_index = 0; i_index < (int)this->o_target_val_histories.size(); i_index++) {
			sum_score += this->o_target_val_histories[i_index];
		}
		this->existing_average_score = sum_score / num_instances;

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
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->i_target_val_histories[i_index] - this->existing_average_score);
			}
			double average_offset = sum_offset / num_train_instances;

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
				outputs(d_index) = this->i_target_val_histories[i_index] - this->existing_average_score;
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

			double impact_threshold = average_offset / num_factors * FACTOR_IMPACT_THRESHOLD;

			for (map<pair<int,int>, double>::iterator it = sum_factor_impacts.begin();
					it != sum_factor_impacts.end(); it++) {
				if (abs(weights(factor_mapping[it->first])) * sum_factor_impacts[it->first] > impact_threshold) {
					this->existing_factor_ids.push_back(it->first);
					this->existing_factor_weights.push_back(weights(factor_mapping[it->first]));
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = this->existing_average_score;
				for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
					map<pair<int,int>, double>::iterator it = this->factor_histories[i_index]
						.find(this->existing_factor_ids[f_index]);
					if (it != this->factor_histories[i_index].end()) {
						sum_score += this->existing_factor_weights[f_index] * it->double;
					}
				}

				remaining_scores[i_index] = this->i_target_val_histories[i_index] - sum_score;
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->i_target_val_histories[i_index] - this->existing_average_score;
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

		Network* existing_network = new Network((int)this->existing_inputs.size());

		train_network(this->input_histories,
					  remaining_scores,
					  existing_network);

		double new_average_misguess;
		double new_misguess_standard_deviation;
		measure_network(this->input_histories,
						remaining_scores,
						existing_network,
						new_average_misguess,
						new_misguess_standard_deviation);

		double new_improvement = average_misguess - new_average_misguess;
		double new_standard_deviation = min(misguess_standard_deviation, new_misguess_standard_deviation);
		double new_t_score = new_improvement / (new_standard_deviation / sqrt(num_test_instances));

		if (new_t_score > 1.645) {
			for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
				vector<pair<pair<vector<Scope*>,vector<int>>,pair<int,int>>> remove_inputs = this->existing_inputs;
				remove_inputs.erase(remove_inputs.begin() + i_index);

				Network* remove_network = new Network(existing_network);
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
					this->existing_inputs = remove_inputs;

					delete existing_network;
					existing_network = remove_network;

					this->input_histories = remove_input_vals;
				} else {
					delete remove_network;
				}
			}

			Factor* new_factor = new Factor();
			new_factor->inputs = this->existing_inputs;
			new_factor->network = existing_network;
			if (this->node_context->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)this->node_context;

				new_factor->parent = obs_node;
				new_factor->index = (int)obs_node->factors.size();

				obs_node->factors.push_back(new_factor);

				this->existing_factor_ids.push_back({obs_node->id, new_factor->index});
				this->existing_factor_weights.push_back(1.0);
			} else {
				ObsNode* new_obs_node = new ObsNode();
				new_obs_node->parent = this->scope_context;
				new_obs_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;
				this->scope_context->nodes[new_obs_node->id] = new_obs_node;

				new_factor->parent = new_obs_node;
				new_factor->index = (int)new_obs_node->factors.size();

				new_obs_node->factors.push_back(new_factor);

				switch (this->node_context->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)this->node_context;

						new_obs_node->next_node_id = action_node->next_node_id;
						new_obs_node->next_node = action_node->next_node;

						for (int a_index = 0; a_index < (int)action_node->next_node->ancestors.size(); a_index++) {
							if (action_node->next_node->ancestors[a_index] == a_index) {
								action_node->next_node->ancestors.erase(
									action_node->next_node->ancestors.begin() + a_index);
								break;
							}
						}
						action_node->next_node->ancestors.push_back(new_obs_node);

						action_node->next_node_id = new_obs_node->id;
						action_node->next_node = new_obs_node;

						new_obs_node->ancestors.push_back(action_node);
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)this->node_context;

						new_obs_node->next_node_id = scope_node->next_node_id;
						new_obs_node->next_node = scope_node->next_node;

						for (int a_index = 0; a_index < (int)scope_node->next_node->ancestors.size(); a_index++) {
							if (scope_node->next_node->ancestors[a_index] == a_index) {
								scope_node->next_node->ancestors.erase(
									scope_node->next_node->ancestors.begin() + a_index);
								break;
							}
						}
						action_node->next_node->ancestors.push_back(new_obs_node);

						scope_node->next_node_id = new_obs_node->id;
						scope_node->next_node = new_obs_node;

						new_obs_node->ancestors.push_back(scope_node);
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)this->node_context;

						if (this->is_branch) {
							new_obs_node->next_node_id = branch_node->branch_next_node_id;
							new_obs_node->next_node = branch_node->branch_next_node;

							for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestors.size(); a_index++) {
								if (branch_node->branch_next_node->ancestors[a_index] == a_index) {
									branch_node->branch_next_node->ancestors.erase(
										branch_node->branch_next_node->ancestors.begin() + a_index);
									break;
								}
							}
							branch_node->branch_next_node->ancestors.push_back(new_obs_node);

							branch_node->branch_next_node_id = new_obs_node->id;
							branch_node->branch_next_node = new_obs_node;
						} else {
							new_obs_node->next_node_id = branch_node->original_next_node_id;
							new_obs_node->next_node = branch_node->original_next_node;

							for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestors.size(); a_index++) {
								if (branch_node->original_next_node->ancestors[a_index] == a_index) {
									branch_node->original_next_node->ancestors.erase(
										branch_node->original_next_node->ancestors.begin() + a_index);
									break;
								}
							}
							branch_node->original_next_node->ancestors.push_back(new_obs_node);

							branch_node->original_next_node_id = new_obs_node->id;
							branch_node->original_next_node = new_obs_node;
						}

						new_obs_node->ancestors.push_back(branch_node);
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

				this->existing_factor_ids.push_back({new_obs_node->id, 0});
				this->existing_factor_weights.push_back(1.0);
			}
		} else {
			delete existing_network;
		}

		this->input_histories.clear();
		this->factor_histories.clear();
		this->i_target_val_histories.clear();
		this->o_target_val_histories.clear();

		uniform_int_distribution<int> good_distribution(0, 3);
		if (good_distribution(generator) == 0) {
			this->explore_type = EXPLORE_TYPE_GOOD;
		} else {
			this->explore_type = EXPLORE_TYPE_BEST;

			this->best_surprise = 0.0;
		}

		uniform_int_distribution<int> until_distribution(0, (int)this->node_context->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = COMMIT_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
