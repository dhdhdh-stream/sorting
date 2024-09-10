#include "branch_experiment.h"

#include <algorithm>
#include <iostream>

#include "absolute_return_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

bool BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		history->instance_count++;

		this->starting_location_histories.push_back(context.back().starting_location);
		this->local_location_histories.push_back(problem->get_location());
		this->world_model_histories.push_back(run_helper.world_model);
		this->node_histories.push_back(context.back().node_history);

		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else if (this->best_step_types[0] == STEP_TYPE_SCOPE) {
				curr_node = this->best_scopes[0];
			} else if (this->best_step_types[0] == STEP_TYPE_RETURN) {
				curr_node = this->best_returns[0];
			} else {
				curr_node = this->best_absolute_returns[0];
			}
		}

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		return true;
	}

	return false;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	if (history->instance_count > 0) {
		for (int i_index = 0; i_index < history->instance_count; i_index++) {
			double surprise = (target_val - run_helper.result) / history->instance_count;
			this->target_val_histories.push_back(surprise);
		}

		this->state_iter++;
	}

	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / num_instances;

		{
			default_random_engine generator_copy = generator;
			shuffle(this->starting_location_histories.begin(), this->starting_location_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->local_location_histories.begin(), this->local_location_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->world_model_histories.begin(), this->world_model_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->node_histories.begin(), this->node_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);
		}

		vector<vector<double>> input_vals(num_instances);
		#if defined(MDEBUG) && MDEBUG
		#else
		double average_misguess;
		double misguess_standard_deviation;
		#endif /* MDEBUG */

		int train_index = 0;
		uniform_int_distribution<int> instance_distribution(0, num_instances-1);
		uniform_int_distribution<int> history_distribution(0, 9);
		uniform_int_distribution<int> global_distribution(0, 2);
		while (train_index < 3) {
			vector<int> test_input_types = this->input_types;
			vector<vector<double>> test_input_locations = this->input_locations;
			vector<AbstractNode*> test_input_node_contexts = this->input_node_contexts;
			vector<int> test_input_obs_indexes = this->input_obs_indexes;

			for (int n_index = 0; n_index < NETWORK_INCREMENT_NUM_NEW; n_index++) {
				int instance_index = instance_distribution(generator);

				if (history_distribution(generator) == 0) {
					uniform_int_distribution<int> history_distribution(0, this->node_histories[instance_index].size()-1);
					map<AbstractNode*, pair<vector<double>,vector<double>>>::iterator it = next(
						this->node_histories[instance_index].begin(), history_distribution(generator));
					AbstractNode* node = it->first;
					uniform_int_distribution<int> obs_distribution(0, it->second.second.size()-1);
					int obs_index = obs_distribution(generator);

					bool is_existing = false;
					for (int i_index = 0; i_index < (int)test_input_types.size(); i_index++) {
						if (test_input_types[i_index] == INPUT_TYPE_HISTORY
								&& test_input_node_contexts[i_index] == node
								&& test_input_obs_indexes[i_index] == obs_index) {
							is_existing = true;
							break;
						}
					}
					if (!is_existing) {
						test_input_types.push_back(INPUT_TYPE_HISTORY);
						test_input_locations.push_back(vector<double>());
						test_input_node_contexts.push_back(node);
						test_input_obs_indexes.push_back(obs_index);
					}
				} else {
					uniform_int_distribution<int> location_distribution(0, this->world_model_histories[instance_index].size()-1);
					vector<double> world_location = next(this->world_model_histories[instance_index].begin(), location_distribution(generator))->first;

					if (global_distribution(generator) == 0) {
						vector<double> relative_location = problem_type->world_to_relative(
							world_location, this->starting_location_histories[instance_index]);

						bool is_existing = false;
						for (int i_index = 0; i_index < (int)test_input_types.size(); i_index++) {
							if (test_input_types[i_index] == INPUT_TYPE_GLOBAL
									&& test_input_locations[i_index] == relative_location) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							test_input_types.push_back(INPUT_TYPE_GLOBAL);
							test_input_locations.push_back(relative_location);
							test_input_node_contexts.push_back(NULL);
							test_input_obs_indexes.push_back(-1);
						}
					} else {
						vector<double> relative_location = problem_type->world_to_relative(
							world_location, this->local_location_histories[instance_index]);

						bool is_existing = false;
						for (int i_index = 0; i_index < (int)test_input_types.size(); i_index++) {
							if (test_input_types[i_index] == INPUT_TYPE_LOCAL
									&& test_input_locations[i_index] == relative_location) {
								is_existing = true;
								break;
							}
						}
						if (!is_existing) {
							test_input_types.push_back(INPUT_TYPE_LOCAL);
							test_input_locations.push_back(relative_location);
							test_input_node_contexts.push_back(NULL);
							test_input_obs_indexes.push_back(-1);
						}
					}
				}
			}

			Network* test_network;
			if (this->network == NULL) {
				test_network = new Network();
			} else {
				test_network = new Network(this->network);
			}
			test_network->increment((int)test_input_types.size());

			vector<vector<double>> test_input_vals = input_vals;
			for (int d_index = 0; d_index < num_instances; d_index++) {
				test_input_vals[d_index].reserve((int)test_input_types.size());
				for (int t_index = (int)this->input_types.size(); t_index < (int)test_input_types.size(); t_index++) {
					switch (test_input_types[t_index]) {
					case INPUT_TYPE_GLOBAL:
						{
							vector<double> location = problem_type->relative_to_world(
								this->starting_location_histories[d_index],
								test_input_locations[t_index]);

							map<vector<double>, double>::iterator it = this->world_model_histories[d_index].find(location);
							if (it == this->world_model_histories[d_index].end()) {
								test_input_vals[d_index].push_back(0.0);
							} else {
								test_input_vals[d_index].push_back(it->second);
							}
						}
						break;
					case INPUT_TYPE_LOCAL:
						{
							vector<double> location = problem_type->relative_to_world(
								this->local_location_histories[d_index],
								test_input_locations[t_index]);

							map<vector<double>, double>::iterator it = this->world_model_histories[d_index].find(location);
							if (it == this->world_model_histories[d_index].end()) {
								test_input_vals[d_index].push_back(0.0);
							} else {
								test_input_vals[d_index].push_back(it->second);
							}
						}
						break;
					case INPUT_TYPE_HISTORY:
						{
							map<AbstractNode*, pair<vector<double>,vector<double>>>::iterator it
								= this->node_histories[d_index].find(test_input_node_contexts[t_index]);
							if (it == this->node_histories[d_index].end()) {
								test_input_vals[d_index].push_back(0.0);
							} else {
								test_input_vals[d_index].push_back(it->second.second[test_input_obs_indexes[t_index]]);
							}
						}
						break;
					}
				}
			}

			train_network(test_input_vals,
						  this->target_val_histories,
						  test_network);

			double test_average_misguess;
			double test_misguess_standard_deviation;
			measure_network(test_input_vals,
							this->target_val_histories,
							test_network,
							test_average_misguess,
							test_misguess_standard_deviation);

			bool is_select = false;
			if (this->network == NULL) {
				is_select = true;
			} else {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%3 == 0) {
				#else
				double improvement = average_misguess - test_average_misguess;
				double standard_deviation = min(misguess_standard_deviation, test_misguess_standard_deviation);
				double t_score = improvement / (standard_deviation / sqrt(num_instances * TEST_SAMPLES_PERCENTAGE));

				if (t_score > 1.645) {
				#endif /* MDEBUG */
					is_select = true;
				}
			}

			if (is_select) {
				cout << "s" << endl;

				int original_input_size = (int)this->input_types.size();
				int test_input_size = (int)test_input_types.size();

				#if defined(MDEBUG) && MDEBUG
				#else
				average_misguess = test_average_misguess;
				misguess_standard_deviation = test_misguess_standard_deviation;
				#endif /* MDEBUG */

				this->input_types = test_input_types;
				this->input_locations = test_input_locations;
				this->input_node_contexts = test_input_node_contexts;
				this->input_obs_indexes = test_input_obs_indexes;

				if (this->network != NULL) {
					delete this->network;
				}
				this->network = test_network;

				input_vals = test_input_vals;

				for (int i_index = test_input_size-1; i_index >= original_input_size; i_index--) {
					vector<int> remove_input_types = this->input_types;
					vector<vector<double>> remove_input_locations = this->input_locations;
					vector<AbstractNode*> remove_input_node_contexts = this->input_node_contexts;
					vector<int> remove_input_obs_indexes = this->input_obs_indexes;

					remove_input_types.erase(remove_input_types.begin() + i_index);
					remove_input_locations.erase(remove_input_locations.begin() + i_index);
					remove_input_node_contexts.erase(remove_input_node_contexts.begin() + i_index);
					remove_input_obs_indexes.erase(remove_input_obs_indexes.begin() + i_index);

					Network* remove_network = new Network(this->network);
					remove_network->remove_input(i_index);

					vector<vector<double>> remove_input_vals = input_vals;
					for (int d_index = 0; d_index < num_instances; d_index++) {
						remove_input_vals[d_index].erase(remove_input_vals[d_index].begin() + i_index);
					}

					optimize_network(remove_input_vals,
									 this->target_val_histories,
									 remove_network);

					double remove_average_misguess;
					double remove_misguess_standard_deviation;
					measure_network(remove_input_vals,
									this->target_val_histories,
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
						this->input_types = remove_input_types;
						this->input_locations = remove_input_locations;
						this->input_node_contexts = remove_input_node_contexts;
						this->input_obs_indexes = remove_input_obs_indexes;

						delete this->network;
						this->network = remove_network;

						input_vals = remove_input_vals;
					} else {
						delete remove_network;
					}
				}

				#if defined(MDEBUG) && MDEBUG
				#else
				train_index = 0;
				#endif /* MDEBUG */
			} else {
				delete test_network;

				train_index++;
			}

			cout << "i" << endl;
		}

		this->starting_location_histories.clear();
		this->local_location_histories.clear();
		this->world_model_histories.clear();
		this->node_histories.clear();
		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
