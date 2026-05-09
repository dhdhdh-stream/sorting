/**
 * - error gradient more reliable with sigmoid instead of RELU?
 *   - maybe specifically for multi-phase?
 * 
 * - maybe RELU not good for anything besides single shot
 *   - need inputs not to be killed
 * 
 * - maybe sigmoid specifically good for non-linear
 *   - the summing kind of already handles linear
 * 
 * - slower to converge though
 * 
 * - none of this seems super reliable though?
 *   - especially if requirements super sharp
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "final_network.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_SAMPLES = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> sequences;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}
		sequences.push_back(curr_sequence);

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			// case 0:
			// 	sum_distance--;
			// 	break;
			case 1:
				sum_distance++;
				break;
			}
		}
		// if (sum_distance >= 1) {
		if (sum_distance == 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	double sum_score = 0.0;
	for (int h_index = 0; h_index < (int)target_vals.size(); h_index++) {
		sum_score += target_vals[h_index];
	}
	double average_score = sum_score / (double)target_vals.size();

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	// Network* score_network = new Network(5);
	FinalNetwork* score_network = new FinalNetwork(5);

	/**
	 * - direct score
	 */
	// for (int iter_index = 0; iter_index < 150000; iter_index++) {
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double average_diff = (target_vals[sequence_index] - average_score)
			/ (double)sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(5, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network->activate(inputs);

			double error = average_diff - score_network->output->acti_vals[0];
			score_network->backprop(error);
		}

		// temp
		if (iter_index % 20 == 0) {
			score_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_scores = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			vector<double> inputs(5, 0.0);
			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;
			inputs[action] += 1.0;

			score_network->activate(inputs);
			cout << "score_network->output->acti_vals[0]: " << score_network->output->acti_vals[0] << endl;

			sum_scores += score_network->output->acti_vals[0];
		}

		cout << "sum_scores: " << sum_scores << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	FinalNetwork* final_network = new FinalNetwork(1);

	/**
	 * - final
	 */
	// for (int iter_index = 0; iter_index < 150000; iter_index++) {
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_scores = average_score;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(5, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network->activate(inputs);

			sum_scores += score_network->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores};
		final_network->activate(final_inputs);

		double error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(error);

		final_network->input->errors[0] = 0.0;

		// temp
		if (iter_index % 20 == 0) {
			final_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_scores = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			vector<double> inputs(5, 0.0);
			int action = sequences[h_index][a_index];
			inputs[action] += 1.0;

			score_network->activate(inputs);

			sum_scores += score_network->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores};
		final_network->activate(final_inputs);
		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	/**
	 * - initial context
	 */
	// for (int iter_index = 0; iter_index < 150000; iter_index++) {
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_scores = average_score;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(5, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network->activate(inputs);

			sum_scores += score_network->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores};
		final_network->activate(final_inputs);

		double error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(error);

		double score_error = final_network->input->errors[0];
		final_network->input->errors[0] = 0.0;

		double average_error = score_error / (double)sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(5, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network->activate(inputs);

			score_network->backprop(average_error);
		}

		// temp
		if (iter_index % 20 == 0) {
			score_network->update();
			final_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_scores = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			vector<double> inputs(5, 0.0);
			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;
			inputs[action] += 1.0;

			score_network->activate(inputs);
			cout << "score_network->output->acti_vals[0]: " << score_network->output->acti_vals[0] << endl;

			sum_scores += score_network->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores};
		final_network->activate(final_inputs);
		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	/**
	 * - with context
	 */
	// for (int iter_index = 0; iter_index < 150000; iter_index++) {
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		{
			double sum_scores = average_score;
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				vector<double> inputs(5, 0.0);
				int action = sequences[sequence_index][a_index];
				inputs[action] += 1.0;
				inputs.back() = sum_scores;

				score_network->activate(inputs);

				sum_scores += score_network->output->acti_vals[0];
			}

			vector<double> final_inputs{sum_scores};
			final_network->activate(final_inputs);
		}

		double error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(error);

		double score_error = final_network->input->errors[0];
		final_network->input->errors[0] = 0.0;

		double average_error = score_error / (double)sequences[sequence_index].size();

		{
			double sum_scores = average_score;
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				vector<double> inputs(5, 0.0);
				int action = sequences[sequence_index][a_index];
				inputs[action] += 1.0;
				inputs.back() = sum_scores;

				score_network->activate(inputs);

				score_network->backprop(average_error);

				sum_scores += score_network->output->acti_vals[0];
			}
		}

		// temp
		if (iter_index % 20 == 0) {
			score_network->update();
			final_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 20; h_index++) {
		cout << h_index << endl;

		double sum_scores = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			vector<double> inputs(5, 0.0);
			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;
			inputs[action] += 1.0;
			inputs.back() = sum_scores;

			score_network->activate(inputs);
			cout << "score_network->output->acti_vals[0]: " << score_network->output->acti_vals[0] << endl;

			sum_scores += score_network->output->acti_vals[0];
			cout << "sum_scores: " << sum_scores << endl;
		}

		vector<double> final_inputs{sum_scores};
		final_network->activate(final_inputs);
		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
