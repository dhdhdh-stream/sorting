#include "eval_experiment.h"

#include <cmath>

#include "globals.h"

#include "action_node.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 2000;
#endif /* MDEBUG */

const double MISGUESS_IMPROVEMENT = 0.2;

void EvalExperiment::measure_activate(Problem* problem) {
	geometric_distribution<int> num_random_distribution(0.2);
	int num_random = num_random_distribution(generator);
	for (int r_index = 0; r_index < num_random; r_index++) {
		problem->perform_action(problem_type->random_action());
	}

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

	bool matches = true;
	for (int f_index = 0; f_index < (int)this->fixed_points.size(); f_index++) {
		double val;
		fetch_input_helper(temp_history,
						   this->fixed_points[f_index],
						   val);

		if (abs(val - this->fixed_point_average_vals[f_index]) > this->fixed_point_standard_deviations[f_index]) {
			matches = false;
		}
	}

	if (matches) {
		vector<double> input_vals(this->score_inputs.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->score_inputs.size(); i_index++) {
			fetch_input_helper(temp_history,
							   this->score_inputs[i_index],
							   input_vals[i_index]);
		}
		this->score_network->activate(input_vals);

		this->predicted_score_histories.push_back(this->score_network->output->acti_vals[0]);
	}
}

void EvalExperiment::measure_backprop(double target_val) {
	this->i_target_val_histories.push_back(target_val);

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
		double sum_misguess_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->i_target_val_histories.size(); h_index++) {
			sum_misguess_variance += (this->i_target_val_histories[h_index] - this->predicted_score_histories[h_index])
				* (this->i_target_val_histories[h_index] - this->predicted_score_histories[h_index]);
		}
		double misguess_standard_deviation = sqrt(sum_misguess_variance / (int)this->i_target_val_histories.size());

		if (misguess_standard_deviation < MISGUESS_IMPROVEMENT * this->score_standard_deviation) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
