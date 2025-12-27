#include "solution_helpers.h"

#include <cmath>
#include <iostream>

#include "globals.h"
#include "network.h"
#include "tunnel.h"

using namespace std;

void measure_tunnel_val(vector<vector<double>>& obs_histories,
						Tunnel* tunnel,
						double& val_average,
						double& val_standard_deviation) {
	vector<double> vals(obs_histories.size());
	for (int h_index = 0; h_index < (int)obs_histories.size(); h_index++) {
		vector<double> inputs(tunnel->obs_indexes.size());
		for (int o_index = 0; o_index < (int)tunnel->obs_indexes.size(); o_index++) {
			inputs[o_index] = obs_histories[h_index][tunnel->obs_indexes[o_index]];
		}

		double similarity;
		if (tunnel->is_pattern) {
			tunnel->similarity_network->activate(inputs);
			similarity = tunnel->similarity_network->output->acti_vals[0];
			if (similarity > 1.0) {
				similarity = 1.0;
			} else if (similarity < 0.0) {
				similarity = 0.0;
			}
		} else {
			similarity = 1.0;
		}

		tunnel->signal_network->activate(inputs);
		double signal = tunnel->signal_network->output->acti_vals[0];

		vals[h_index] = similarity * signal;
	}

	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		sum_vals += vals[h_index];
	}
	val_average = sum_vals / (double)vals.size();

	double sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)vals.size(); h_index++) {
		sum_variance += (vals[h_index] - val_average) * (vals[h_index] - val_average);
	}
	val_standard_deviation = sqrt(sum_variance / (double)vals.size());
}

void retrain_and_compare(vector<vector<double>>& obs_histories,
						 vector<double>& target_val_histories,
						 Tunnel* tunnel) {
	vector<vector<double>> inputs(obs_histories.size());
	for (int h_index = 0; h_index < (int)obs_histories.size(); h_index++) {
		inputs[h_index] = vector<double>(tunnel->obs_indexes.size());
		for (int o_index = 0; o_index < (int)tunnel->obs_indexes.size(); o_index++) {
			inputs[h_index][o_index] = obs_histories[h_index][tunnel->obs_indexes[o_index]];
		}
	}

	vector<double> original_vals(inputs.size());
	for (int h_index = 0; h_index < (int)inputs.size(); h_index++) {
		tunnel->signal_network->activate(inputs[h_index]);
		original_vals[h_index] = tunnel->signal_network->output->acti_vals[0];
	}

	double original_sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)original_vals.size(); h_index++) {
		original_sum_vals += original_vals[h_index];
	}
	double original_val_average = original_sum_vals / (double)original_vals.size();

	double original_sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)original_vals.size(); h_index++) {
		original_sum_variance += (original_vals[h_index] - original_val_average) * (original_vals[h_index] - original_val_average);
	}
	double original_val_standard_deviation = sqrt(original_sum_variance / (double)original_vals.size());

	Network* new_network = new Network(tunnel->obs_indexes.size(),
									   NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> sample_distribution(0, inputs.size()-1);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 30; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
	#endif /* MDEBUG */
		int index = sample_distribution(generator);

		new_network->activate(inputs[index]);

		double error = target_val_histories[index] - new_network->output->acti_vals[0];

		new_network->backprop(error);
	}

	vector<double> new_vals(inputs.size());
	for (int h_index = 0; h_index < (int)inputs.size(); h_index++) {
		new_network->activate(inputs[h_index]);
		new_vals[h_index] = new_network->output->acti_vals[0];
	}

	double new_sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)new_vals.size(); h_index++) {
		new_sum_vals += new_vals[h_index];
	}
	double new_val_average = new_sum_vals / (double)new_vals.size();

	double new_sum_variance = 0.0;
	for (int h_index = 0; h_index < (int)new_vals.size(); h_index++) {
		new_sum_variance += (new_vals[h_index] - new_val_average) * (new_vals[h_index] - new_val_average);
	}
	double new_val_standard_deviation = sqrt(new_sum_variance / (double)new_vals.size());

	double sum_covariance = 0.0;
	for (int h_index = 0; h_index < (int)original_vals.size(); h_index++) {
		sum_covariance += (original_vals[h_index] - original_val_average) * (new_vals[h_index] - new_val_average);
	}
	double covariance = sum_covariance / (double)original_vals.size();

	double pcc = covariance / original_val_standard_deviation / new_val_standard_deviation;

	cout << "pcc: " << pcc << endl;

	delete new_network;
}
