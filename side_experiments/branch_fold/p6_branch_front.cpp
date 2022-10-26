/**
 * 0 - 2: blank
 * 1 - 2: 1 which is shared val
 * 2 - 2: 1 which is 1st branch score modifier
 * 3 - 2: 1 which is 2nd branch score modifier
 * 4 - 2: 1 which is choice
 * 5 - 3: 1 which is 1st branch val, 1 which is 2nd branch val
 * 6 - 2: 1 which is choice
 * 7 - 2: blank
 * - 1st branch:
 *   - 0 - 3: 1 which is val
 * - 2nd branch:
 *   - 0 - 2: 1 which is val
 *   - 1 - 2: blank
 *   - 2 - 3: 1 which is val
 * 0 - 2: blank
 * 1 - 3: 1 which is 1st branch score modifier
 * 2 - 2: 1 which is 2nd branch score modifier
 * 3 - 2: 1 which is shared val
 * 4 - 2: 1 which is shared score modifier
 * 5 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "branch_fold_network.h"
#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	int network_input_size = 2+2+2+2+2+3+2+2
							 +2+2+3;
	Network* front_network = new Network(network_input_size,
										 100,
										 1);

	double sum_error = 0.0;
	for (int iter_index = 1; iter_index < 1000000; iter_index++) {
		if (iter_index%10000 == 0) {
			cout << endl;
			cout << iter_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		int choice_val = 0;
		int shared_val = 0;
		int second_val = 0;
		double second_score_modifier = 0.0;
		double shared_score_modifier = 0.0;

		vector<double> network_inputs;
		network_inputs.reserve(network_input_size);

		double pre_0_0 = rand()%2*2-1;
		network_inputs.push_back(pre_0_0);
		double pre_0_1 = rand()%2*2-1;
		network_inputs.push_back(pre_0_1);

		double pre_1_0 = rand()%2*2-1;
		if (pre_1_0 == 1.0) {
			shared_val++;
		}
		network_inputs.push_back(pre_1_0);
		double pre_1_1 = rand()%2*2-1;
		network_inputs.push_back(pre_1_1);

		double pre_2_0 = rand()%2*2-1;
		network_inputs.push_back(pre_2_0);
		double pre_2_1 = rand()%2*2-1;
		network_inputs.push_back(pre_2_1);

		double pre_3_0 = rand()%2*2-1;
		second_score_modifier += pre_3_0;
		network_inputs.push_back(pre_3_0);
		double pre_3_1 = rand()%2*2-1;
		network_inputs.push_back(pre_3_1);

		double pre_4_0 = rand()%2*2-1;
		if (pre_4_0 == 1.0) {
			choice_val++;
		}
		network_inputs.push_back(pre_4_0);
		double pre_4_1 = rand()%2*2-1;
		network_inputs.push_back(pre_4_1);

		double pre_5_0 = rand()%2*2-1;
		network_inputs.push_back(pre_5_0);
		double pre_5_1 = rand()%2*2-1;
		if (pre_5_1 == 1.0) {
			second_val++;
		}
		network_inputs.push_back(pre_5_1);
		double pre_5_2 = rand()%2*2-1;
		network_inputs.push_back(pre_5_2);

		double pre_6_0;
		if (choice_val == 1) {
			pre_6_0 = -1.0;
		} else {
			pre_6_0 = 1.0;
			choice_val++;
		}
		network_inputs.push_back(pre_6_0);
		double pre_6_1 = rand()%2*2-1;
		network_inputs.push_back(pre_6_1);

		double pre_7_0 = rand()%2*2-1;
		network_inputs.push_back(pre_7_0);
		double pre_7_1 = rand()%2*2-1;
		network_inputs.push_back(pre_7_1);

		double new_0_0 = rand()%2*2-1;
		if (new_0_0 == 1.0) {
			second_val++;
		}
		network_inputs.push_back(new_0_0);
		double new_0_1 = rand()%2*2-1;
		network_inputs.push_back(new_0_1);

		double new_1_0 = rand()%2*2-1;
		network_inputs.push_back(new_1_0);
		double new_1_1 = rand()%2*2-1;
		network_inputs.push_back(new_1_1);

		double new_2_0 = rand()%2*2-1;
		if (new_2_0 == 1.0) {
			second_val++;
		}
		network_inputs.push_back(new_2_0);
		double new_2_1 = rand()%2*2-1;
		network_inputs.push_back(new_2_1);
		double new_2_2 = rand()%2*2-1;
		network_inputs.push_back(new_2_2);

		front_network->activate(network_inputs);

		if (iter_index%10000 == 0) {
			cout << front_network->output->acti_vals[0] << endl;
		}

		double post_2_0 = rand()%2*2-1;
		second_score_modifier += post_2_0;

		double post_3_0 = rand()%2*2-1;
		if (post_3_0 == 1.0) {
			shared_val++;
		}

		double post_4_0 = rand()%2*2-1;
		shared_score_modifier += post_4_0;

		double final_val;
		if (choice_val%2 == 0) {
			final_val = -5.0;
		} else {
			final_val = (shared_val+second_val)%2 + second_score_modifier + shared_score_modifier;
		}

		vector<double> errors;
		errors.push_back(final_val - front_network->output->acti_vals[0]);
		sum_error += abs(errors[0]);

		if (iter_index < 800000) {
			front_network->backprop(errors, 0.01);
		} else {
			front_network->backprop(errors, 0.001);
		}
	}

	ofstream output_file;
	output_file.open("saves/second_branch_front_network.txt");
	front_network->save(output_file);
	output_file.close();

	delete front_network;

	cout << "Done" << endl;
}
