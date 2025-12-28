#include "solution_helpers.h"

#include <cmath>
/**
 * good tunnel:
 * - 1 iters:
 *   t_score: 5.31231
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.637703
 *   this->starting_true_standard_deviation: 19.9939
 *   this->starting_val_average: -0.187549
 *   this->starting_val_standard_deviation: 1.3997
 *   this->current_true_average: 0.326205
 *   this->current_true_standard_deviation: 19.2857
 *   this->current_val_average: 0.0472931
 *   this->current_val_standard_deviation: 1.39795
 * - 5 iters:
 *   t_score: 22.6733
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.637703
 *   this->starting_true_standard_deviation: 19.9939
 *   this->starting_val_average: -0.187549
 *   this->starting_val_standard_deviation: 1.3997
 *   this->current_true_average: 1.48375
 *   this->current_true_standard_deviation: 19.1106
 *   this->current_val_average: 1.20465
 *   this->current_val_standard_deviation: 1.94171
 *   this->num_tries: 98
 *   this->num_train_fail: 17
 *   this->num_measure_fail: 51
 *   this->num_success: 30
 *   pcc: 0.991036
 * bad tunnel:
 * - 1 iters:
 *   t_score: 0.87063
 *   this->obs_indexes: 3
 *   this->starting_true_average: 0.899782
 *   this->starting_true_standard_deviation: 19.5195
 *   this->starting_val_average: -1.40236
 *   this->starting_val_standard_deviation: 6.40947
 *   this->current_true_average: 0.605357
 *   this->current_true_standard_deviation: 18.9842
 *   this->current_val_average: -1.22711
 *   this->current_val_standard_deviation: 6.36523
 *   this->num_tries: 91
 *   this->num_train_fail: 75
 *   this->num_measure_fail: 10
 *   this->num_success: 6
 * - 5 iters:
 *   t_score: 0.2988
 *   this->obs_indexes: 3
 *   this->starting_true_average: 0.899782
 *   this->starting_true_standard_deviation: 19.5195
 *   this->starting_val_average: -1.40236
 *   this->starting_val_standard_deviation: 6.40947
 *   this->current_true_average: 0.94555
 *   this->current_true_standard_deviation: 18.5589
 *   this->current_val_average: -1.34174
 *   this->current_val_standard_deviation: 6.41535
 *   this->num_tries: 322
 *   this->num_train_fail: 234
 *   this->num_measure_fail: 58
 *   this->num_success: 30
 *   pcc: 0.994647
 * diminishing returns:
 * - 1 iters:
 *   t_score: 2.25223
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 1.90564
 *   this->current_true_standard_deviation: 19.4833
 *   this->current_val_average: -1.07945
 *   this->current_val_standard_deviation: 2.47991
 *   this->num_tries: 20
 *   this->num_train_fail: 4
 *   this->num_measure_fail: 10
 *   this->num_success: 6
 * - 2 iters:
 *   t_score: 6.17168
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 0.702249
 *   this->current_true_standard_deviation: 19.1533
 *   this->current_val_average: -0.733192
 *   this->current_val_standard_deviation: 2.67916
 *   this->num_tries: 55
 *   this->num_train_fail: 7
 *   this->num_measure_fail: 36
 *   this->num_success: 12
 * - 3 iters:
 *   t_score: 8.55538
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 0.387708
 *   this->current_true_standard_deviation: 19.0437
 *   this->current_val_average: -0.560987
 *   this->current_val_standard_deviation: 2.56921
 *   this->num_tries: 125
 *   this->num_train_fail: 20
 *   this->num_measure_fail: 87
 *   this->num_success: 18
 * - 5 iters:
 *   t_score: 24.2482
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 0.467921
 *   this->current_true_standard_deviation: 19.4945
 *   this->current_val_average: 0.702412
 *   this->current_val_standard_deviation: 2.55412
 *   this->num_tries: 182
 *   this->num_train_fail: 28
 *   this->num_measure_fail: 124
 *   this->num_success: 30
 *   pcc: 0.912222
 * - 10 iters:
 *   t_score: 37.6261
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 1.1931
 *   this->current_true_standard_deviation: 19.1642
 *   this->current_val_average: 1.95677
 *   this->current_val_standard_deviation: 2.70023
 *   this->num_tries: 311
 *   this->num_train_fail: 53
 *   this->num_measure_fail: 198
 *   this->num_success: 60
 *   pcc: 0.750118
 * - 15 iters:
 *   t_score: 61.6899
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 2.37077
 *   this->current_true_standard_deviation: 19.2633
 *   this->current_val_average: 5.43055
 *   this->current_val_standard_deviation: 3.42762
 *   this->num_tries: 446
 *   this->num_train_fail: 90
 *   this->num_measure_fail: 266
 *   this->num_success: 90
 *   pcc: 0.793701
 * - 20 iters:
 *   t_score: 56.9419
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 2.4929
 *   this->current_true_standard_deviation: 19.0403
 *   this->current_val_average: 11.3929
 *   this->current_val_standard_deviation: 7.02464
 *   this->num_tries: 559
 *   this->num_train_fail: 107
 *   this->num_measure_fail: 332
 *   this->num_success: 120
 *   pcc: 0.755284
 * - 25 iters:
 *   t_score: 72.199
 *   this->obs_indexes: 2
 *   this->starting_true_average: 0.592943
 *   this->starting_true_standard_deviation: 19.061
 *   this->starting_val_average: -1.25607
 *   this->starting_val_standard_deviation: 2.06807
 *   this->current_true_average: 2.75487
 *   this->current_true_standard_deviation: 19.3425
 *   this->current_val_average: 14.2253
 *   this->current_val_standard_deviation: 6.78074
 *   this->num_tries: 707
 *   this->num_train_fail: 139
 *   this->num_measure_fail: 418
 *   this->num_success: 150
 *   pcc: 0.837552
 * max:
 * - 1 iters:
 *   t_score: 5.60649
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 1.77265
 *   this->current_true_standard_deviation: 19.4358
 *   this->current_val_average: 0.88045
 *   this->current_val_standard_deviation: 1.00518
 *   this->num_tries: 29
 *   this->num_train_fail: 7
 *   this->num_measure_fail: 16
 *   this->num_success: 6
 * - 5 iters:
 *   t_score: 12.5725
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 2.69947
 *   this->current_true_standard_deviation: 19.1685
 *   this->current_val_average: 1.21963
 *   this->current_val_standard_deviation: 1.30135
 *   this->num_tries: 113
 *   this->num_train_fail: 21
 *   this->num_measure_fail: 62
 *   this->num_success: 30
 *   pcc: 0.996917
 * - 10 iters:
 *   t_score: 16.8916
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 3.67241
 *   this->current_true_standard_deviation: 19.0536
 *   this->current_val_average: 1.3668
 *   this->current_val_standard_deviation: 1.24413
 *   this->num_tries: 306
 *   this->num_train_fail: 73
 *   this->num_measure_fail: 173
 *   this->num_success: 60
 *   pcc: 0.992268
 * - 15 iters:
 *   t_score: 37.5735
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 2.75209
 *   this->current_true_standard_deviation: 19.6312
 *   this->current_val_average: 2.11268
 *   this->current_val_standard_deviation: 1.18706
 *   this->num_tries: 469
 *   this->num_train_fail: 123
 *   this->num_measure_fail: 256
 *   this->num_success: 90
 *   pcc: 0.979611
 * - 20 iters:
 *   t_score: 43.2631
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 4.21565
 *   this->current_true_standard_deviation: 19.2583
 *   this->current_val_average: 3.44704
 *   this->current_val_standard_deviation: 2.00629
 *   this->num_tries: 658
 *   this->num_train_fail: 182
 *   this->num_measure_fail: 356
 *   this->num_success: 120
 *   pcc: 0.987772
 * - 25 iters:
 *   t_score: 38.8613
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 4.40548
 *   this->current_true_standard_deviation: 19.4077
 *   this->current_val_average: 4.49989
 *   this->current_val_standard_deviation: 3.09028
 *   this->num_tries: 857
 *   this->num_train_fail: 233
 *   this->num_measure_fail: 474
 *   this->num_success: 150
 *   pcc: 0.99091
 * - 30 iters:
 *   t_score: 45.1169
 *   this->obs_indexes: 2
 *   this->starting_true_average: 2.32113
 *   this->starting_true_standard_deviation: 19.3357
 *   this->starting_val_average: 0.702239
 *   this->starting_val_standard_deviation: 0.843315
 *   this->current_true_average: 3.54074
 *   this->current_true_standard_deviation: 19.4912
 *   this->current_val_average: 5.32768
 *   this->current_val_standard_deviation: 3.24201
 *   this->num_tries: 997
 *   this->num_train_fail: 273
 *   this->num_measure_fail: 544
 *   this->num_success: 180
 *   pcc: 0.992287
 * reverse:
 * - 1 iters:
 *   t_score: 7.93143
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 1.92573
 *   this->current_true_standard_deviation: 19.1447
 *   this->current_val_average: 0.711503
 *   this->current_val_standard_deviation: 1.41437
 *   this->num_tries: 21
 *   this->num_train_fail: 5
 *   this->num_measure_fail: 10
 *   this->num_success: 6
 * - 5 iters:
 *   t_score: 23.0625
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 2.53149
 *   this->current_true_standard_deviation: 19.609
 *   this->current_val_average: 1.41317
 *   this->current_val_standard_deviation: 1.44852
 *   this->num_tries: 141
 *   this->num_train_fail: 31
 *   this->num_measure_fail: 80
 *   this->num_success: 30
 *   pcc: 0.983842
 * - 10 iters:
 *   t_score: 50.8591
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 3.23849
 *   this->current_true_standard_deviation: 18.8123
 *   this->current_val_average: 3.19639
 *   this->current_val_standard_deviation: 1.7656
 *   this->num_tries: 294
 *   this->num_train_fail: 84
 *   this->num_measure_fail: 150
 *   this->num_success: 60
 *   pcc: 0.981603
 * - 15 iters:
 *   t_score: 58.4857
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 3.72181
 *   this->current_true_standard_deviation: 18.9749
 *   this->current_val_average: 3.63903
 *   this->current_val_standard_deviation: 1.7747
 *   this->num_tries: 472
 *   this->num_train_fail: 149
 *   this->num_measure_fail: 233
 *   this->num_success: 90
 *   pcc: 0.987359
 * - 20 iters:
 *   t_score: 59.4285
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 2.92042
 *   this->current_true_standard_deviation: 19.0326
 *   this->current_val_average: 4.26792
 *   this->current_val_standard_deviation: 2.08118
 *   this->num_tries: 685
 *   this->num_train_fail: 235
 *   this->num_measure_fail: 330
 *   this->num_success: 120
 *   pcc: 0.961874
 * - 25 iters:
 *   t_score: 55.9009
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 4.22034
 *   this->current_true_standard_deviation: 20.0196
 *   this->current_val_average: 4.27501
 *   this->current_val_standard_deviation: 2.21653
 *   this->num_tries: 881
 *   this->num_train_fail: 310
 *   this->num_measure_fail: 421
 *   this->num_success: 150
 *   pcc: 0.966224
 * - 65 iters:
 *   t_score: 65.906
 *   this->obs_indexes: 2
 *   this->starting_true_average: 1.15527
 *   this->starting_true_standard_deviation: 18.9901
 *   this->starting_val_average: 0.356759
 *   this->starting_val_standard_deviation: 1.29062
 *   this->current_true_average: 2.5684
 *   this->current_true_standard_deviation: 19.1875
 *   this->current_val_average: 5.25026
 *   this->current_val_standard_deviation: 2.34798
 *   this->num_tries: 3604
 *   this->num_train_fail: 1764
 *   this->num_measure_fail: 1450
 *   this->num_success: 390
 *   pcc: 0.922952
 */

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
