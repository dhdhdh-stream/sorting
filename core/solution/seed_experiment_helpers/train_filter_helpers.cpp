#include "seed_experiment.h"

using namespace std;

const int TRAIN_FILTER_ITERS = 3;

void SeedExperiment::train_filter_backprop(double target_val,
										   SeedExperimentOverallHistory* history) {
	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (history->has_target) {
		if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
			this->i_is_higher_histories.push_back(true);
		} else {
			this->i_is_higher_histories.push_back(false);
		}

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
			if (this->curr_filter->network == NULL) {
				test_network = new Network(num_new_input_indexes);
			} else {
				test_network = new Network(this->curr_filter->network);

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

			overshoot_train_network(network_inputs,
									this->i_is_higher_histories,
									test_network_input_scope_contexts,
									test_network_input_node_contexts,
									test_network);

			double average_misguess;
			double misguess_variance;
			measure_network(network_inputs,
							this->i_is_higher_histories,
							test_network,
							average_misguess,
							misguess_variance);

			if (this->curr_filter->network == NULL) {
				this->curr_filter->input_scope_contexts.push_back(test_network_input_scope_contexts);
				this->curr_filter->input_node_contexts.push_back(test_network_input_node_contexts);
				vector<int> new_input_indexes;
				for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
					new_input_indexes.push_back(t_index);
				}
				this->curr_filter->network_input_indexes.push_back(new_input_indexes);

				this->curr_filter->network = test_network;

				this->curr_filter->average_misguess = average_misguess;
				this->curr_filter->misguess_variance = misguess_variance;
			} else {
				double improvement = this->curr_filter->average_misguess - average_misguess;
				double standard_deviation = min(sqrt(this->curr_filter->misguess_variance), sqrt(misguess_variance));
				double t_score = improvement / (standard_deviation / sqrt(solution->curr_num_datapoints * TEST_SAMPLES_PERCENTAGE));
				if (t_score > 2.326) {
					vector<int> new_input_indexes;
					for (int t_index = 0; t_index < (int)test_network_input_scope_contexts.size(); t_index++) {
						int index = -1;
						for (int i_index = 0; i_index < (int)this->curr_filter->input_scope_contexts.size(); i_index++) {
							if (test_network_input_scope_contexts[t_index] == this->curr_filter->input_scope_contexts[i_index]
									&& test_network_input_node_contexts[t_index] == this->curr_filter->input_node_contexts[i_index]) {
								index = i_index;
								break;
							}
						}
						if (index == -1) {
							this->curr_filter->input_scope_contexts.push_back(test_network_input_scope_contexts[t_index]);
							this->curr_filter->input_node_contexts.push_back(test_network_input_node_contexts[t_index]);

							index = this->curr_filter->input_scope_contexts.size()-1;
						}
						new_input_indexes.push_back(index);
					}
					this->curr_filter->network_input_indexes.push_back(new_input_indexes);

					delete this->curr_filter->network;
					this->curr_filter->network = test_network;

					this->curr_filter->average_misguess = average_misguess;
					this->curr_filter->misguess_variance = misguess_variance;
				} else {
					delete test_network;
				}
			}

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_is_higher_histories.clear();

			this->state_iter++;
			if (this->state_iter >= TRAIN_FILTER_ITERS) {
				this->i_is_seed_histories.reserve(solution->curr_num_datapoints);
				this->i_is_higher_histories.reserve(solution->curr_num_datapoints);

				this->state = SEED_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_is_higher_histories.reserve(solution->curr_num_datapoints);
			}
		}
	}
}
