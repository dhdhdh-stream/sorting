#include "pattern.h"

#include "network.h"
#include "solution_helpers.h"

using namespace std;

Pattern::Pattern() {
	this->keypoint_network = NULL;
	this->predict_network = NULL;
}

Pattern::~Pattern() {
	if (this->keypoint_network != NULL) {
		delete this->keypoint_network;
	}

	if (this->predict_network != NULL) {
		delete this->predict_network;
	}
}

void Pattern::activate(bool& has_match,
					   double& predicted,
					   ScopeHistory* scope_history) {
	vector<double> keypoint_vals(this->keypoints.size());
	vector<bool> keypoint_is_on(this->keypoints.size());
	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   this->keypoints[k_index],
						   0,
						   val,
						   is_on);
		keypoint_vals[k_index] = val;
		keypoint_is_on[k_index] = is_on;
	}
	this->keypoint_network->activate(keypoint_vals,
									 keypoint_is_on);
	if (this->keypoint_network->output->acti_vals[0] < 0.0) {
		has_match = false;
	} else {
		has_match = true;

		vector<double> input_vals(this->inputs.size());
		vector<bool> input_is_on(this->inputs.size());
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->inputs[i_index],
							   0,
							   val,
							   is_on);
			input_vals[i_index] = val;
			input_is_on[i_index] = is_on;
		}
		this->predict_network->activate(input_vals,
										input_is_on);
		predicted = this->predict_network->output->acti_vals[0];
	}
}
