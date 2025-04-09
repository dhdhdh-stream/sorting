#include "eval_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "nn_helpers.h"
#include "obs_node.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

const double FIXED_POINT_GATE = 0.1;

void EvalExperiment::train_activate(Problem* problem) {
	ScopeHistory* temp_history = new ScopeHistory(this->scope_context);

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		switch (this->nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->nodes[n_index];
				action_node->new_activate(problem);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->nodes[n_index];
				obs_node->new_activate(problem,
									   temp_history);
			}
			break;
		}
	}

	vector<double> input_vals(this->inputs.size());
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		fetch_input_helper(temp_history,
						   this->inputs[i_index],
						   input_vals[i_index]);
	}
	this->input_histories.push_back(input_vals);

	delete temp_history;
}

void EvalExperiment::train_backprop(double target_val) {
	this->i_target_val_histories.push_back(target_val);

	this->state_iter++;
	if (this->state_iter >= TRAIN_NUM_DATAPOINTS) {
		double sum_score_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
			sum_score_vals += this->i_target_val_histories[h_index];
		}
		this->score_average_val = sum_score_vals / (int)this->i_target_val_histories.size();

		double sum_score_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
			sum_score_variance += (this->i_target_val_histories[h_index] - this->score_average_val)
				* (this->i_target_val_histories[h_index] - this->score_average_val);
		}
		this->score_standard_deviation = sqrt(sum_score_variance / (int)this->i_target_val_histories.size());
		if (this->score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		double sum_obs_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->input_histories.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->input_histories[h_index].size(); i_index++) {
				sum_obs_vals += this->input_histories[h_index][i_index];
			}
		}
		this->obs_average_val = sum_obs_vals / (int)this->input_histories.size() / (int)this->inputs.size();

		cout << "this->obs_average_val: " << this->obs_average_val << endl;

		double sum_obs_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->input_histories.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->input_histories[h_index].size(); i_index++) {
				sum_obs_variance += (this->input_histories[h_index][i_index] - this->obs_average_val)
					* (this->input_histories[h_index][i_index] - this->obs_average_val);
			}
		}
		this->obs_standard_deviation = sqrt(sum_obs_variance / (int)this->input_histories.size() / (int)this->inputs.size());
		if (this->obs_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->obs_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		cout << "this->obs_standard_deviation: " << this->obs_standard_deviation << endl;

		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->input_histories.size(); h_index++) {
				sum_vals += this->input_histories[h_index][i_index];
			}
			double average_val = sum_vals / (int)this->input_histories.size();

			double sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->input_histories.size(); h_index++) {
				sum_variance += (this->input_histories[h_index][i_index] - average_val)
					* (this->input_histories[h_index][i_index] - average_val);
			}
			double standard_deviation = sqrt(sum_variance / (int)this->input_histories.size());
			if (standard_deviation < MIN_STANDARD_DEVIATION) {
				standard_deviation = MIN_STANDARD_DEVIATION;
			}

			cout << "standard_deviation: " << standard_deviation << endl;

			if (standard_deviation < FIXED_POINT_GATE * this->obs_standard_deviation) {
				this->fixed_points.push_back(this->inputs[i_index]);
				this->fixed_point_average_vals.push_back(average_val);
				this->fixed_point_standard_deviations.push_back(standard_deviation);
			}
		}

		{
			default_random_engine generator_copy = generator;
			shuffle(this->input_histories.begin(), this->input_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->i_target_val_histories.begin(), this->i_target_val_histories.end(), generator_copy);
		}

		int num_instances = (int)this->i_target_val_histories.size();

		this->score_inputs = this->inputs;
		this->score_network = new Network((int)this->score_inputs.size());

		train_network(this->input_histories,
					  this->i_target_val_histories,
					  this->score_network);

		double average_misguess;
		double misguess_standard_deviation;
		measure_network(this->input_histories,
						this->i_target_val_histories,
						this->score_network,
						average_misguess,
						misguess_standard_deviation);

		for (int i_index = (int)this->score_inputs.size()-1; i_index >= 0; i_index--) {
			vector<Input> remove_inputs = this->score_inputs;
			remove_inputs.erase(remove_inputs.begin() + i_index);

			Network* remove_network = new Network(this->score_network);
			remove_network->remove_input(i_index);

			vector<vector<double>> remove_input_vals = this->input_histories;
			for (int d_index = 0; d_index < num_instances; d_index++) {
				remove_input_vals[d_index].erase(remove_input_vals[d_index].begin() + i_index);
			}

			optimize_network(remove_input_vals,
							 this->i_target_val_histories,
							 remove_network);

			double remove_average_misguess;
			double remove_misguess_standard_deviation;
			measure_network(remove_input_vals,
							this->i_target_val_histories,
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
				this->score_inputs = remove_inputs;

				delete this->score_network;
				this->score_network = remove_network;

				this->input_histories = remove_input_vals;
			} else {
				delete remove_network;
			}
		}

		this->input_histories.clear();
		this->i_target_val_histories.clear();

		this->state = EVAL_EXPERIMENT_MEASURE;
		this->state_iter = 0;
	}
}
