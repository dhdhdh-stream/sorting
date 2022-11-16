#include "branch.h"

#include <iostream>
#include <limits>

using namespace std;



void Branch::activate(vector<double>& input_state_vals,
					  vector<double>& s_input_vals,
					  vector<double>& output_state_vals,
					  vector<vector<double>>& flat_vals,
					  double& predicted_score,
					  double& scale_factor) {
	double best_score = numeric_limits<double>::lowest();
	double best_index = -1;
	for (int b_index = 0; b_index < (int)this->branches.size(); b_index++) {
		this->score_networks[b_index]->activate(input_state_vals,
												s_input_vals);
		double curr_predicted_score = previous_scale_mods[b_index]*predicted_score
									  + previous_average_mods[b_index]
									  + scale_factor*this->score_networks[b_index]->output->acti_vals[0];
		if (curr_predicted_score > best_score) {
			best_score = curr_predicted_score;
			best_index = b_index;
		}
	}

	predicted_score = curr_predicted_score;

	vector<double> scope_state_vals;
	if (this->compress_sizes[best_index] > 0) {
		if (this->active_compress[best_index]) {
			this->compress_networks[best_index]->activate_small(input_state_vals,
																s_input_vals);
			int compress_new_size = (int)input_state_vals.size() - this->compress_sizes[best_index];
			scope_state_vals.reserve(compress_new_size);
			for (int s_index = 0; s_index < compress_new_size; s_index++) {
				scope_state_vals.push_back(this->compress_networks[best_index]->output->acti_vals[s_index]);
			}
		} else {
			scope_state_vals = vector<double>(input_state_vals.begin(),
				input_state_vals.end()-this->compress_sizes[best_index]);
		}
	} else {
		scope_state_vals = input_state_vals;
	}

	this->branches[best_index]->activate(flat_vals,
										 scope_state_vals,
										 s_input_vals,
										 output_state_vals,
										 predicted_score,
										 scale_factor);

	predicted_score += this->ending_averages_mods[best_index];

	scale_factor *= this->ending_scale_modes[best_index];
}
