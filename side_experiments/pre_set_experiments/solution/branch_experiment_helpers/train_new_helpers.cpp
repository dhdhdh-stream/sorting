#include "branch_experiment.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const int NETWORK_NUM_INPUTS = 10;

bool BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		history->instance_count++;

		this->scope_histories.push_back(new ScopeHistory(scope_history));

		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
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
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_history;

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
		double new_average_score = sum_scores / num_instances;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			this->target_val_histories[d_index] -= new_average_score;
		}

		{
			default_random_engine generator_copy = generator;
			shuffle(this->scope_histories.begin(), this->scope_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);
		}

		uniform_int_distribution<int> instance_distribution(0, num_instances-1);
		for (int i_index = 0; i_index < NETWORK_NUM_INPUTS; i_index++) {
			vector<Scope*> scope_context;
			vector<int> node_context;
			int node_count = 0;
			pair<pair<vector<Scope*>,vector<int>>,int> new_input;
			gather_possible_helper(this->scope_histories[instance_distribution(generator)],
								   scope_context,
								   node_context,
								   node_count,
								   new_input);

			bool is_existing = false;
			for (int ii_index = 0; ii_index < (int)this->inputs.size(); ii_index++) {
				if (new_input == this->inputs[ii_index]) {
					is_existing = true;
					break;
				}
			}
			if (!is_existing) {
				this->inputs.push_back(new_input);
			}
		}

		this->network = new Network((int)this->inputs.size());

		vector<vector<double>> input_vals(num_instances);
		for (int d_index = 0; d_index < num_instances; d_index++) {
			input_vals[d_index] = vector<double>(this->inputs.size(), 0.0);
			for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
				fetch_input_helper(this->scope_histories[d_index],
								   this->inputs[i_index],
								   0,
								   input_vals[d_index][i_index]);
			}
		}

		train_network(input_vals,
					  this->target_val_histories,
					  this->network);

		double average_misguess;
		double misguess_standard_deviation;
		measure_network(input_vals,
						this->target_val_histories,
						this->network,
						average_misguess,
						misguess_standard_deviation);

		for (int i_index = (int)this->inputs.size()-1; i_index >= 0; i_index--) {
			vector<pair<pair<vector<Scope*>,vector<int>>,int>> remove_inputs = this->inputs;
			remove_inputs.erase(remove_inputs.begin() + i_index);

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
				this->inputs = remove_inputs;

				delete this->network;
				this->network = remove_network;

				input_vals = remove_input_vals;
			} else {
				delete remove_network;
			}
		}

		for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
			delete this->scope_histories[h_index];
		}
		this->scope_histories.clear();
		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
