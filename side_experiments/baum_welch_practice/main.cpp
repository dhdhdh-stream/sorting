#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<double>> true_transition{
		{0.7, 0.3},
		{0.8, 0.2}
	};

	vector<vector<double>> true_emission{
		{1.0, 0.0},
		{0.2, 0.8}
	};

	vector<double> true_starting{1.0, 0.0};

	vector<int> obs;
	{
		int curr_state = 0;
		uniform_real_distribution<double> distribution(0.0, 1.0);
		for (int i = 0; i < 20; i++) {
			double obs_random_val = distribution(generator);
			if (obs_random_val > true_emission[curr_state][0]) {
				obs.push_back(1);
			} else {
				obs.push_back(0);
			}

			double transition_random_val = distribution(generator);
			if (transition_random_val > true_transition[curr_state][0]) {
				curr_state = 1;
			} else {
				curr_state = 0;
			}

			// cout << i << ": " << curr_state << endl;
		}
	}

	for (int o_index = 0; o_index < 20; o_index++) {
		cout << o_index << ": " << obs[o_index] << endl;
	}

	// viterbi
	// {
	// 	vector<vector<double>> forward_likelihoods;
	// 	vector<vector<vector<double>>> forward_sub_likelihoods;

	// 	forward_likelihoods.push_back(true_starting);

	// 	for (int o_index = 0; o_index < 20; o_index++) {
	// 		double likelihood_0 = forward_likelihoods.back()[0] * true_emission[0][obs[o_index]];
	// 		double likelihood_1 = forward_likelihoods.back()[1] * true_emission[1][obs[o_index]];

	// 		double likelihood_0_to_0 = likelihood_0 * true_transition[0][0];
	// 		double likelihood_0_to_1 = likelihood_0 * true_transition[0][1];
	// 		double likelihood_1_to_0 = likelihood_1 * true_transition[1][0];
	// 		double likelihood_1_to_1 = likelihood_1 * true_transition[1][1];

	// 		double sum_likelihood = likelihood_0_to_0 + likelihood_0_to_1
	// 			+ likelihood_1_to_0 + likelihood_1_to_1;

	// 		forward_likelihoods.push_back({
	// 			(likelihood_0_to_0 + likelihood_1_to_0) / sum_likelihood,
	// 			(likelihood_0_to_1 + likelihood_1_to_1) / sum_likelihood});

	// 		forward_sub_likelihoods.push_back({
	// 			{likelihood_0_to_0, likelihood_0_to_1},
	// 			{likelihood_1_to_0, likelihood_1_to_1}
	// 		});
	// 	}

	// 	vector<int> max_sequence;

	// 	if (forward_likelihoods.back()[0] > forward_likelihoods.back()[1]) {
	// 		max_sequence.insert(max_sequence.begin(), 0);
	// 	} else {
	// 		max_sequence.insert(max_sequence.begin(), 1);
	// 	}

	// 	for (int o_index = 18; o_index >= 0; o_index--) {
	// 		if (forward_sub_likelihoods[o_index][0][max_sequence[0]]
	// 				> forward_sub_likelihoods[o_index][1][max_sequence[0]]) {
	// 			max_sequence.insert(max_sequence.begin(), 0);
	// 		} else {
	// 			max_sequence.insert(max_sequence.begin(), 1);
	// 		}
	// 	}

	// 	for (int s_index = 0; s_index < (int)max_sequence.size(); s_index++) {
	// 		cout << s_index << ": " << max_sequence[s_index] << endl;
	// 	}
	// }

	// state likelihoods
	// {
	// 	vector<vector<double>> forward_likelihoods;

	// 	forward_likelihoods.push_back(true_starting);

	// 	for (int o_index = 0; o_index < 20; o_index++) {
	// 		double likelihood_0 = forward_likelihoods.back()[0] * true_emission[0][obs[o_index]];
	// 		double likelihood_1 = forward_likelihoods.back()[1] * true_emission[1][obs[o_index]];

	// 		double likelihood_0_to_0 = likelihood_0 * true_transition[0][0];
	// 		double likelihood_0_to_1 = likelihood_0 * true_transition[0][1];
	// 		double likelihood_1_to_0 = likelihood_1 * true_transition[1][0];
	// 		double likelihood_1_to_1 = likelihood_1 * true_transition[1][1];

	// 		double sum_likelihood = likelihood_0_to_0 + likelihood_0_to_1
	// 			+ likelihood_1_to_0 + likelihood_1_to_1;

	// 		forward_likelihoods.push_back({
	// 			(likelihood_0_to_0 + likelihood_1_to_0) / sum_likelihood,
	// 			(likelihood_0_to_1 + likelihood_1_to_1) / sum_likelihood});
	// 	}

	// 	vector<vector<double>> backward_likelihoods;

	// 	backward_likelihoods.insert(backward_likelihoods.begin(), {1.0, 1.0});

	// 	for (int o_index = 19; o_index >= 0; o_index--) {
	// 		double likelihood_0_from_0 = backward_likelihoods[0][0] * true_emission[0][obs[o_index]] * true_transition[0][0];
	// 		double likelihood_0_from_1 = backward_likelihoods[0][0] * true_emission[1][obs[o_index]] * true_transition[1][0];
	// 		double likelihood_1_from_0 = backward_likelihoods[0][1] * true_emission[0][obs[o_index]] * true_transition[0][1];
	// 		double likelihood_1_from_1 = backward_likelihoods[0][1] * true_emission[1][obs[o_index]] * true_transition[1][1];

	// 		double sum_likelihood = likelihood_0_from_0 + likelihood_0_from_1
	// 			+ likelihood_1_from_0 + likelihood_1_from_1;

	// 		backward_likelihoods.insert(backward_likelihoods.begin(), {
	// 			(likelihood_0_from_0 + likelihood_1_from_0) / sum_likelihood,
	// 			(likelihood_0_from_1 + likelihood_1_from_1) / sum_likelihood
	// 		});
	// 	}

	// 	vector<vector<double>> state_probabilities;
	// 	for (int o_index = 0; o_index < 21; o_index++) {
	// 		double likelihood_0 = forward_likelihoods[o_index][0] * backward_likelihoods[o_index][0];
	// 		double likelihood_1 = forward_likelihoods[o_index][1] * backward_likelihoods[o_index][1];

	// 		// cout << o_index << endl;
	// 		// cout << "0 forward: " << forward_likelihoods[o_index][0] << endl;
	// 		// cout << "0 backward: " << backward_likelihoods[o_index][0] << endl;
	// 		// cout << "1 forward: " << forward_likelihoods[o_index][1] << endl;
	// 		// cout << "1 backward: " << backward_likelihoods[o_index][1] << endl;

	// 		double sum_likelihood = likelihood_0 + likelihood_1;

	// 		state_probabilities.push_back({
	// 			likelihood_0 / sum_likelihood,
	// 			likelihood_1 / sum_likelihood
	// 		});
	// 	}

	// 	for (int o_index = 0; o_index < 21; o_index++) {
	// 		cout << o_index << endl;
	// 		cout << "0: " << state_probabilities[o_index][0] << endl;
	// 		cout << "1: " << state_probabilities[o_index][1] << endl;
	// 	}
	// }

	vector<vector<double>> curr_transition{
		{0.5, 0.5},
		{0.3, 0.7}
	};

	vector<vector<double>> curr_emission{
		{0.3, 0.7},
		{0.8, 0.2}
	};

	vector<double> curr_starting{0.2, 0.8};

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		vector<vector<double>> forward_likelihoods;

		forward_likelihoods.push_back(curr_starting);

		for (int o_index = 0; o_index < 20; o_index++) {
			double likelihood_0 = forward_likelihoods.back()[0] * curr_emission[0][obs[o_index]];
			double likelihood_1 = forward_likelihoods.back()[1] * curr_emission[1][obs[o_index]];

			double likelihood_0_to_0 = likelihood_0 * curr_transition[0][0];
			double likelihood_0_to_1 = likelihood_0 * curr_transition[0][1];
			double likelihood_1_to_0 = likelihood_1 * curr_transition[1][0];
			double likelihood_1_to_1 = likelihood_1 * curr_transition[1][1];

			double sum_likelihood = likelihood_0_to_0 + likelihood_0_to_1
				+ likelihood_1_to_0 + likelihood_1_to_1;

			forward_likelihoods.push_back({
				(likelihood_0_to_0 + likelihood_1_to_0) / sum_likelihood,
				(likelihood_0_to_1 + likelihood_1_to_1) / sum_likelihood});
		}

		vector<vector<double>> backward_likelihoods;

		backward_likelihoods.insert(backward_likelihoods.begin(), {1.0, 1.0});

		for (int o_index = 19; o_index >= 0; o_index--) {
			double likelihood_0_from_0 = backward_likelihoods[0][0] * curr_emission[0][obs[o_index]] * curr_transition[0][0];
			double likelihood_0_from_1 = backward_likelihoods[0][0] * curr_emission[1][obs[o_index]] * curr_transition[1][0];
			double likelihood_1_from_0 = backward_likelihoods[0][1] * curr_emission[0][obs[o_index]] * curr_transition[0][1];
			double likelihood_1_from_1 = backward_likelihoods[0][1] * curr_emission[1][obs[o_index]] * curr_transition[1][1];

			double sum_likelihood = likelihood_0_from_0 + likelihood_0_from_1
				+ likelihood_1_from_0 + likelihood_1_from_1;

			backward_likelihoods.insert(backward_likelihoods.begin(), {
				(likelihood_0_from_0 + likelihood_1_from_0) / sum_likelihood,
				(likelihood_0_from_1 + likelihood_1_from_1) / sum_likelihood
			});
		}

		vector<vector<double>> state_probabilities;
		for (int o_index = 0; o_index < 21; o_index++) {
			double likelihood_0 = forward_likelihoods[o_index][0] * backward_likelihoods[o_index][0];
			double likelihood_1 = forward_likelihoods[o_index][1] * backward_likelihoods[o_index][1];

			double sum_likelihood = likelihood_0 + likelihood_1;

			state_probabilities.push_back({
				likelihood_0 / sum_likelihood,
				likelihood_1 / sum_likelihood
			});
		}

		{
			double sum_0_after_0 = 0.0;
			double sum_1_after_0 = 0.0;
			double sum_0_after_1 = 0.0;
			double sum_1_after_1 = 0.0;
			for (int o_index = 0; o_index < 20; o_index++) {
				sum_0_after_0 += state_probabilities[o_index][0] * state_probabilities[o_index+1][0];
				sum_1_after_0 += state_probabilities[o_index][0] * state_probabilities[o_index+1][1];
				sum_0_after_1 += state_probabilities[o_index][1] * state_probabilities[o_index+1][0];
				sum_1_after_1 += state_probabilities[o_index][1] * state_probabilities[o_index+1][1];
			}
			double sum_0_counts = sum_0_after_0 + sum_1_after_0;
			double sum_1_counts = sum_0_after_1 + sum_1_after_1;
			curr_transition = {
				{sum_0_after_0 / sum_0_counts, sum_1_after_0 / sum_0_counts},
				{sum_0_after_1 / sum_1_counts, sum_1_after_1 / sum_1_counts}
			};
		}

		{
			double sum_0_obs_0_counts = 0.0;
			double sum_0_obs_1_counts = 0.0;
			double sum_1_obs_0_counts = 0.0;
			double sum_1_obs_1_counts = 0.0;
			for (int o_index = 0; o_index < 20; o_index++) {
				if (obs[o_index] == 0) {
					sum_0_obs_0_counts += state_probabilities[o_index][0];
					sum_1_obs_0_counts += state_probabilities[o_index][1];
				} else {
					sum_0_obs_1_counts += state_probabilities[o_index][0];
					sum_1_obs_1_counts += state_probabilities[o_index][1];
				}
			}
			double sum_0_counts = sum_0_obs_0_counts + sum_0_obs_1_counts;
			double sum_1_counts = sum_1_obs_0_counts + sum_1_obs_1_counts;
			curr_emission = {
				{sum_0_obs_0_counts / sum_0_counts, sum_0_obs_1_counts / sum_0_counts},
				{sum_1_obs_0_counts / sum_1_counts, sum_1_obs_1_counts / sum_1_counts}
			};
		}

		curr_starting = state_probabilities[0];

		cout << "curr_transition[0][0]: " << curr_transition[0][0] << endl;
		cout << "curr_transition[0][1]: " << curr_transition[0][1] << endl;
		cout << "curr_transition[1][0]: " << curr_transition[1][0] << endl;
		cout << "curr_transition[1][1]: " << curr_transition[1][1] << endl;
		cout << endl;

		cout << "curr_emission[0][0]: " << curr_emission[0][0] << endl;
		cout << "curr_emission[0][1]: " << curr_emission[0][1] << endl;
		cout << "curr_emission[1][0]: " << curr_emission[1][0] << endl;
		cout << "curr_emission[1][1]: " << curr_emission[1][1] << endl;
		cout << endl;

		cout << "curr_starting[0]: " << curr_starting[0] << endl;
		cout << "curr_starting[1]: " << curr_starting[1] << endl;
		cout << endl;
	}

	cout << "Done" << endl;
}
