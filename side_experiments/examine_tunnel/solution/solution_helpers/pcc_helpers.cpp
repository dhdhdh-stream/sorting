#include "solution_helpers.h"

#include <cmath>

using namespace std;

double calc_pcc(vector<double>& val_1s,
				vector<double>& val_2s) {
	double val_1s_val_sum = 0.0;
	for (int i_index = 0; i_index < (int)val_1s.size(); i_index++) {
		val_1s_val_sum += val_1s[i_index];
	}
	double val_1s_val_average = val_1s_val_sum / (double)val_1s.size();

	double val_1s_variance_sum = 0.0;
	for (int i_index = 0; i_index < (int)val_1s.size(); i_index++) {
		val_1s_variance_sum += (val_1s[i_index] - val_1s_val_average) * (val_1s[i_index] - val_1s_val_average);
	}
	double val_1s_standard_deviation = sqrt(val_1s_variance_sum / (double)val_1s.size());

	double val_2s_val_sum = 0.0;
	for (int i_index = 0; i_index < (int)val_2s.size(); i_index++) {
		val_2s_val_sum += val_2s[i_index];
	}
	double val_2s_val_average = val_2s_val_sum / (double)val_2s.size();

	double val_2s_variance_sum = 0.0;
	for (int i_index = 0; i_index < (int)val_2s.size(); i_index++) {
		val_2s_variance_sum += (val_2s[i_index] - val_2s_val_average) * (val_2s[i_index] - val_2s_val_average);
	}
	double val_2s_standard_deviation = sqrt(val_2s_variance_sum / (double)val_2s.size());

	double sum_covariance = 0.0;
	for (int i_index = 0; i_index < (int)val_1s.size(); i_index++) {
		sum_covariance += (val_1s[i_index] - val_1s_val_average)
			* (val_2s[i_index] - val_2s_val_average);
	}
	double covariance_average = sum_covariance / (double)val_1s.size();

	double pcc = covariance_average / val_1s_standard_deviation / val_2s_standard_deviation;

	return pcc;
}
