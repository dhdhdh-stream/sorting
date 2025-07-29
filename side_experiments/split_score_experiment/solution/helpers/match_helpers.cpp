#include "helpers.h"

#include <cmath>

#include "network.h"

using namespace std;

double calc_miss_average_guess(vector<vector<double>>& vals,
							   vector<double>& target_vals,
							   vector<Network*>& match_networks) {
	double sum_target_vals = 0.0;
	int sum_count = 0;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		bool has_match = false;
		for (int m_index = 0; m_index < (int)match_networks.size(); m_index++) {
			match_networks[m_index]->activate(vals[h_index]);
			if (match_networks[m_index]->output->acti_vals[0] > 0.0) {
				has_match = true;
				break;
			}
		}

		if (!has_match) {
			sum_target_vals += target_vals[h_index];
			sum_count++;
		}
	}

	if (sum_count > 0) {
		return sum_target_vals / (double)sum_count;
	} else {
		return 0.0;
	}
}

double calc_signal(vector<double>& vals,
				   vector<Network*>& match_networks,
				   vector<Network*>& signal_networks,
				   double miss_average_guess) {
	for (int m_index = 0; m_index < (int)match_networks.size(); m_index++) {
		bool is_match;
		if (match_networks[m_index] == NULL) {
			is_match = true;
		} else {
			match_networks[m_index]->activate(vals);
			if (match_networks[m_index]->output->acti_vals[0] > 0.0) {
				is_match = true;
			} else {
				is_match = false;
			}
		}

		if (is_match) {
			signal_networks[m_index]->activate(vals);
			return signal_networks[m_index]->output->acti_vals[0];
		}
	}

	return miss_average_guess;
}

void eval_signal(vector<vector<double>>& vals,
				 vector<double>& target_vals,
				 vector<Network*>& match_networks,
				 vector<Network*>& signal_networks,
				 double miss_average_guess,
				 double& new_misguess,
				 double& new_misguess_standard_deviation) {
	vector<double> signals(vals.size());
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		signals[h_index] = calc_signal(vals[h_index],
									   match_networks,
									   signal_networks,
									   miss_average_guess);
	}

	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		sum_misguess += (target_vals[h_index] - signals[h_index])
			* (target_vals[h_index] - signals[h_index]);
	}
	new_misguess = sum_misguess / (double)vals.size();

	double sum_misguess_variance = 0.0;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		double curr_misguess = (target_vals[h_index] - signals[h_index])
			* (target_vals[h_index] - signals[h_index]);
		sum_misguess_variance += (curr_misguess - new_misguess)
			* (curr_misguess - new_misguess);
	}
	new_misguess_standard_deviation = sqrt(sum_misguess_variance / (double)vals.size());
}
