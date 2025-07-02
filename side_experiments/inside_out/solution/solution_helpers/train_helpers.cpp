#include "solution_helpers.h"

#include <cmath>
#include <iostream>

using namespace std;

void analyze_input(Input& input,
				   vector<ScopeHistory*>& scope_histories,
				   InputData& input_data) {
	vector<double> vals;
	int num_is_on = 0;
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_histories[h_index],
						   input,
						   0,
						   val,
						   is_on);
		if (is_on) {
			vals.push_back(val);
			num_is_on++;
		}
	}

	input_data.hit_percent = (double)num_is_on / (double)scope_histories.size();
	if (input_data.hit_percent >= MIN_CONSIDER_HIT_PERCENT) {
		double sum_vals = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_vals += vals[v_index];
		}
		input_data.average = sum_vals / (double)vals.size();

		double sum_variance = 0.0;
		for (int v_index = 0; v_index < (int)vals.size(); v_index++) {
			sum_variance += (input_data.average - vals[v_index]) * (input_data.average - vals[v_index]);
		}
		input_data.standard_deviation = sqrt(sum_variance / (double)vals.size());
	}
}

bool is_unique(vector<vector<double>>& input_vals,
			   vector<double>& existing_averages,
			   vector<double>& existing_standard_deviations,
			   vector<double>& potential_input_vals,
			   double& potential_average,
			   double& potential_standard_deviation,
			   double max_pcc) {
	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)potential_input_vals.size(); h_index++) {
		sum_vals += potential_input_vals[h_index];
	}
	potential_average = sum_vals / (double)potential_input_vals.size();

	double sum_variances = 0.0;
	for (int h_index = 0; h_index < (int)potential_input_vals.size(); h_index++) {
		sum_variances += (potential_input_vals[h_index] - potential_average)
			* (potential_input_vals[h_index] - potential_average);
	}
	potential_standard_deviation = sqrt(sum_variances / (double)potential_input_vals.size());

	for (int f_index = 0; f_index < (int)input_vals.size(); f_index++) {
		double sum_covariance = 0.0;
		for (int h_index = 0; h_index < (int)potential_input_vals.size(); h_index++) {
			sum_covariance += (potential_input_vals[h_index] - potential_average)
				* (input_vals[f_index][h_index] - existing_averages[f_index]);
		}
		double covariance = sum_covariance / (double)potential_input_vals.size();

		double pcc = covariance / potential_standard_deviation / existing_standard_deviations[f_index];
		if (abs(pcc) > max_pcc) {
			return false;
		}
	}

	return true;
}
