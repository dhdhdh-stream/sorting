#include "keypoint.h"

using namespace std;

void Keypoint::measure_activate(double target_val,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	bool has_dependencies = true;
	vector<double> input_vals(this->inputs.size());
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		bool hit;
		fetch_input_helper(scope_history,
						   this->inputs[i_index],
						   0,
						   hit,
						   input_vals[i_index]);
		if (!hit) {
			has_dependencies = false;
			break;
		}
	}

	if (has_dependencies) {
		this->num_hit++;

		this->network->activate(input_vals);
		double predicted_val = this->network->output->acti_vals[0];

		double curr_misguess = (target_val - predicted_val) * (target_val - predicted_val);
		this->sum_misguess += curr_misguess;
	} else {
		this->num_miss++;
	}
}
