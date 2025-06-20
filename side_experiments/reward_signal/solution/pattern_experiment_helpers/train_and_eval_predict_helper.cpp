#include "pattern_experiment.h"

#include "network.h"
#include "nn_helpers.h"
#include "pattern.h"
#include "solution_helpers.h"

using namespace std;

double PatternExperiment::train_and_eval_predict_helper(Pattern* potential_pattern) {
	vector<vector<double>> input_vals(this->explore_scope_histories.size());
	vector<vector<bool>> input_is_on(this->explore_scope_histories.size());
	for (int h_index = 0; h_index < (int)this->explore_scope_histories.size(); h_index++) {
		vector<double> curr_vals(potential_pattern->inputs.size());
		vector<bool> curr_is_on(potential_pattern->inputs.size());
		for (int i_index = 0; i_index < (int)potential_pattern->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(this->explore_scope_histories[h_index],
							   potential_pattern->inputs[i_index],
							   0,
							   val,
							   is_on);
			curr_vals[i_index] = val;
			curr_is_on[i_index] = is_on;
		}
		input_vals[h_index] = curr_vals;
		input_is_on[h_index] = curr_is_on;
	}

	potential_pattern->predict_network = new Network((int)potential_pattern->inputs.size(),
													 input_vals,
													 input_is_on);

	train_network(input_vals,
				  input_is_on,
				  this->explore_target_vals,
				  potential_pattern->predict_network);

	double average_misguess;
	double misguess_standard_deviation;
	measure_network(input_vals,
					input_is_on,
					this->explore_target_vals,
					potential_pattern->predict_network,
					average_misguess,
					misguess_standard_deviation);

	return average_misguess;
}
