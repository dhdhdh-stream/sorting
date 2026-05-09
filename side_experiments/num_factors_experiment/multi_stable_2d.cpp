/**
 * - can't expect everything to just automatically shift into place
 * 
 * - maybe find something that matters, and set it to 1.0
 *   - train final network
 *     - then retrain to adjust for context
 *   - for something that matters, initially, capture to +1 or -1
 *     - but how to transition to context?
 *       - maybe after final network set, retrain?
 * 
 * - but what if XOR factors?
 *   - need contrast between 2 different things
 *     - not findable 1-by-1
 * - maybe always try with a set of possible factors
 * 
 * - even with single sequence, context changes things
 *   - but now want to track context plus actions
 * 
 * - when exploring, break into 2 parts:
 *   - commit: actions which to perform no matter what
 *   - return: constantly calculate way back, taking into account context
 *     - and constantly updating context
 * 
 * - so it makes sense to measure things in terms of actions
 *   - but even actions don't always contribute 1.0 to actions
 *     - so there are other factors involved?
 *       - how to capture those
 * 
 * - just because sequences are dependent doesn't mean breaks should happen
 *   - could depend on context
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

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		vector<int> curr_sequence;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
		}
		sequences.push_back(curr_sequence);

		int sum_distance_one = 0;
		int sum_distance_two = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			case 0:
				sum_distance_one--;
				break;
			case 1:
				sum_distance_one++;
				break;
			case 2:
				sum_distance_two--;
				break;
			case 3:
				sum_distance_two++;
				break;
			}
		}

		if (sum_distance_one >= 1
				&& sum_distance_two >= 0) {
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

	FinalNetwork* score_network_one = new FinalNetwork(6);
	FinalNetwork* score_network_two = new FinalNetwork(6);

	/**
	 * - direct score
	 */
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double average_diff = (target_vals[sequence_index] - average_score)
			/ (double)sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(6, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			double error_one = average_diff - score_network_one->output->acti_vals[0];
			score_network_one->backprop(error_one);

			score_network_two->activate(inputs);
			double error_two = average_diff - score_network_two->output->acti_vals[0];
			score_network_two->backprop(error_two);
		}

		// temp
		if (iter_index % 20 == 0) {
			score_network_one->update();
			score_network_two->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			vector<double> inputs(6, 0.0);
			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			cout << "score_network_one->output->acti_vals[0]: " << score_network_one->output->acti_vals[0] << endl;
			sum_scores_one += score_network_one->output->acti_vals[0];

			score_network_two->activate(inputs);
			cout << "score_network_two->output->acti_vals[0]: " << score_network_two->output->acti_vals[0] << endl;
			sum_scores_two += score_network_two->output->acti_vals[0];
		}

		cout << "sum_scores_one: " << sum_scores_one << endl;
		cout << "sum_scores_two: " << sum_scores_two << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	FinalNetwork* final_network = new FinalNetwork(2);

	/**
	 * - final
	 */
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(6, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			sum_scores_one += score_network_one->output->acti_vals[0];

			score_network_two->activate(inputs);
			sum_scores_two += score_network_two->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores_one, sum_scores_two};
		final_network->activate(final_inputs);

		double error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(error);

		final_network->input->errors[0] = 0.0;
		final_network->input->errors[1] = 0.0;

		// temp
		if (iter_index % 20 == 0) {
			final_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			vector<double> inputs(6, 0.0);
			int action = sequences[h_index][a_index];
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			sum_scores_one += score_network_one->output->acti_vals[0];

			score_network_two->activate(inputs);
			sum_scores_two += score_network_two->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores_one, sum_scores_two};
		final_network->activate(final_inputs);
		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	/**
	 * - initial context
	 */
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(6, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			sum_scores_one += score_network_one->output->acti_vals[0];

			score_network_two->activate(inputs);
			sum_scores_two += score_network_two->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores_one, sum_scores_two};
		final_network->activate(final_inputs);

		double error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(error);

		double score_error_one = final_network->input->errors[0];
		final_network->input->errors[0] = 0.0;
		double score_error_two = final_network->input->errors[1];
		final_network->input->errors[1] = 0.0;

		double average_error_one = score_error_one / (double)sequences[sequence_index].size();
		double average_error_two = score_error_two / (double)sequences[sequence_index].size();

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			vector<double> inputs(6, 0.0);
			int action = sequences[sequence_index][a_index];
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			score_network_one->backprop(average_error_one);

			score_network_two->activate(inputs);
			score_network_two->backprop(average_error_two);
		}

		// temp
		if (iter_index % 20 == 0) {
			score_network_one->update();
			score_network_two->update();
			final_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 10; h_index++) {
		cout << h_index << endl;

		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			vector<double> inputs(6, 0.0);
			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;
			inputs[action] += 1.0;

			score_network_one->activate(inputs);
			cout << "score_network_one->output->acti_vals[0]: " << score_network_one->output->acti_vals[0] << endl;
			sum_scores_one += score_network_one->output->acti_vals[0];

			score_network_two->activate(inputs);
			cout << "score_network_two->output->acti_vals[0]: " << score_network_two->output->acti_vals[0] << endl;
			sum_scores_two += score_network_two->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores_one, sum_scores_two};
		final_network->activate(final_inputs);
		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	/**
	 * - with context
	 */
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		{
			double sum_scores_one = average_score;
			double sum_scores_two = average_score;
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				vector<double> inputs(6, 0.0);
				int action = sequences[sequence_index][a_index];
				inputs[action] += 1.0;
				inputs[4] = sum_scores_one;
				inputs[5] = sum_scores_two;

				score_network_one->activate(inputs);
				sum_scores_one += score_network_one->output->acti_vals[0];

				score_network_two->activate(inputs);
				sum_scores_two += score_network_two->output->acti_vals[0];
			}

			vector<double> final_inputs{sum_scores_one, sum_scores_two};
			final_network->activate(final_inputs);
		}

		double error = target_vals[sequence_index] - final_network->output->acti_vals[0];
		final_network->backprop(error);

		double score_error_one = final_network->input->errors[0];
		final_network->input->errors[0] = 0.0;
		double average_error_one = score_error_one / (double)sequences[sequence_index].size();

		double score_error_two = final_network->input->errors[1];
		final_network->input->errors[1] = 0.0;
		double average_error_two = score_error_two / (double)sequences[sequence_index].size();

		{
			double sum_scores_one = average_score;
			double sum_scores_two = average_score;
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				vector<double> inputs(5, 0.0);
				int action = sequences[sequence_index][a_index];
				inputs[action] += 1.0;
				inputs[4] = sum_scores_one;
				inputs[5] = sum_scores_two;

				score_network_one->activate(inputs);
				score_network_one->backprop(average_error_one);
				sum_scores_one += score_network_one->output->acti_vals[0];

				score_network_two->activate(inputs);
				score_network_two->backprop(average_error_two);
				sum_scores_two += score_network_two->output->acti_vals[0];
			}
		}

		// temp
		if (iter_index % 20 == 0) {
			score_network_one->update();
			score_network_two->update();
			final_network->update();
		}
	}

	// temp
	for (int h_index = 0; h_index < 40; h_index++) {
		cout << h_index << endl;

		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			vector<double> inputs(5, 0.0);
			int action = sequences[h_index][a_index];
			cout << "action: " << action << endl;
			inputs[action] += 1.0;
			inputs[4] = sum_scores_one;
			inputs[5] = sum_scores_two;

			score_network_one->activate(inputs);
			cout << "score_network_one->output->acti_vals[0]: " << score_network_one->output->acti_vals[0] << endl;
			sum_scores_one += score_network_one->output->acti_vals[0];
			cout << "sum_scores_one: " << sum_scores_one << endl;

			score_network_two->activate(inputs);
			cout << "score_network_two->output->acti_vals[0]: " << score_network_two->output->acti_vals[0] << endl;
			sum_scores_two += score_network_two->output->acti_vals[0];
			cout << "sum_scores_two: " << sum_scores_two << endl;
		}

		vector<double> final_inputs{sum_scores_one, sum_scores_two};
		final_network->activate(final_inputs);
		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;

		cout << "target_vals[h_index]: " << target_vals[h_index] << endl;

		cout << endl;
	}

	// temp
	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)sequences.size(); h_index++) {
		double sum_scores_one = average_score;
		double sum_scores_two = average_score;
		for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
			vector<double> inputs(5, 0.0);
			int action = sequences[h_index][a_index];
			inputs[action] += 1.0;
			inputs[4] = sum_scores_one;
			inputs[5] = sum_scores_two;

			score_network_one->activate(inputs);
			sum_scores_one += score_network_one->output->acti_vals[0];

			score_network_two->activate(inputs);
			sum_scores_two += score_network_two->output->acti_vals[0];
		}

		vector<double> final_inputs{sum_scores_one, sum_scores_two};
		final_network->activate(final_inputs);

		sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
			* (target_vals[h_index] - final_network->output->acti_vals[0]);
	}
	double average_misguess = sum_misguess / (double)sequences.size();
	cout << "average_misguess: " << average_misguess << endl;

	cout << "Done" << endl;
}
