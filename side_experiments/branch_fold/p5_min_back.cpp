/**
 * Note: back nodes cannot help solve this problem
 * 
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
 * 1 - 3: 1 which is 1st branch val
 * 2 - 2: 1 which is 2nd branch val
 * 3 - 2: 1 which is shared val
 * 4 - 2: 1 which is shared score modifier
 * 5 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

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

	vector<int> second_branch_choice_flat_sizes;
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(3);
	second_branch_choice_flat_sizes.push_back(2);
	second_branch_choice_flat_sizes.push_back(2);

	ifstream second_branch_choice_input_file;
	second_branch_choice_input_file.open("saves/second_branch_choice_fold_network.txt");
	FoldNetwork* second_branch_choice_flat_network = new FoldNetwork(second_branch_choice_input_file);
	second_branch_choice_input_file.close();

	vector<Node*> nodes;
	for (int i = 0; i < 15; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_14.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	ifstream second_branch_front_input_file;
	second_branch_front_input_file.open("saves/second_branch_front_network.txt");
	Network* second_branch_front_network = new Network(second_branch_front_input_file);
	second_branch_front_input_file.close();

	// int first_branch_count = 0;
	// double first_branch_average = 0.0;
	// int second_branch_count = 0;
	// double second_branch_average = 0.0;
	// for (int iter_index = 0; iter_index < 100000; iter_index++) {
	// 	vector<vector<double>> state_vals;
	// 	double predicted_score = 0.0;

	// 	int flat_input_counter = 0;

	// 	int choice_val = 0;
	// 	int shared_val = 0;
	// 	int first_val = 0;
	// 	int second_val = 0;
	// 	double first_score_modifier = 0.0;
	// 	double second_score_modifier = 0.0;
	// 	double shared_score_modifier = 0.0;

	// 	vector<vector<double>> flat_vals;
	// 	flat_vals.reserve(8);

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[0][0] = rand()%2*2-1;
	// 	flat_vals[0][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[1][0] = rand()%2*2-1;
	// 	if (flat_vals[1][0] == 1.0) {
	// 		shared_val++;
	// 	}
	// 	flat_vals[1][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[2][0] = rand()%2*2-1;
	// 	first_score_modifier += flat_vals[2][0];
	// 	flat_vals[2][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[3][0] = rand()%2*2-1;
	// 	second_score_modifier += flat_vals[3][0];
	// 	flat_vals[3][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[4][0] = rand()%2*2-1;
	// 	if (flat_vals[4][0] == 1.0) {
	// 		choice_val++;
	// 	}
	// 	flat_vals[4][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[5][0] = rand()%2*2-1;
	// 	if (flat_vals[5][0] == 1.0) {
	// 		first_val++;
	// 	}
	// 	flat_vals[5][1] = rand()%2*2-1;
	// 	if (flat_vals[5][1] == 1.0) {
	// 		second_val++;
	// 	}
	// 	flat_vals[5][2] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[6][0] = rand()%2*2-1;
	// 	if (flat_vals[6][0] == 1.0) {
	// 		choice_val++;
	// 	}
	// 	flat_vals[6][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[7][0] = rand()%2*2-1;
	// 	flat_vals[7][1] = rand()%2*2-1;
	// 	nodes[flat_input_counter]->activate(state_vals,
	// 										flat_vals[flat_input_counter],
	// 										predicted_score);
	// 	flat_input_counter++;

	// 	second_branch_choice_flat_network->activate(flat_vals);

	// 	// cout << "1st branch prediction: " << predicted_score << endl;
	// 	// cout << "2nd branch prediction: " << second_branch_choice_flat_network->output->acti_vals[0] << endl;

	// 	double final_val;
	// 	if (predicted_score > second_branch_choice_flat_network->output->acti_vals[0]) {
	// 		// 1st branch
	// 		double flat_vals_8_0 = rand()%2*2-1;
	// 		if (flat_vals_8_0 == 1.0) {
	// 			first_val++;
	// 		}

	// 		double flat_vals_10_0 = rand()%2*2-1;
	// 		if (flat_vals_10_0 == 1.0) {
	// 			first_val++;
	// 		}

	// 		double flat_vals_12_0 = rand()%2*2-1;
	// 		if (flat_vals_12_0 == 1.0) {
	// 			shared_val++;
	// 		}

	// 		double flat_vals_13_0 = rand()%2*2-1;
	// 		shared_score_modifier += flat_vals_13_0;

	// 		if (choice_val%2 == 0) {
	// 			final_val = (shared_val+first_val)%2 + first_score_modifier + shared_score_modifier;
	// 		} else {
	// 			final_val = -5.0;
	// 		}

	// 		first_branch_count++;
	// 		first_branch_average += final_val;
	// 	} else {
	// 		// 2nd branch
	// 		double flat_vals_8_0 = rand()%2*2-1;
	// 		if (flat_vals_8_0 == 1.0) {
	// 			second_val++;
	// 		}

	// 		double flat_vals_10_0 = rand()%2*2-1;
	// 		if (flat_vals_10_0 == 1.0) {
	// 			second_val++;
	// 		}

	// 		double flat_vals_13_0 = rand()%2*2-1;
	// 		if (flat_vals_13_0 == 1.0) {
	// 			second_val++;
	// 		}

	// 		double flat_vals_14_0 = rand()%2*2-1;
	// 		if (flat_vals_14_0 == 1.0) {
	// 			shared_val++;
	// 		}

	// 		double flat_vals_15_0 = rand()%2*2-1;
	// 		shared_score_modifier += flat_vals_15_0;

	// 		if (choice_val%2 == 0) {
	// 			final_val = -5.0;
	// 		} else {
	// 			final_val = (shared_val+second_val)%2 + second_score_modifier + shared_score_modifier;
	// 		}

	// 		second_branch_count++;
	// 		second_branch_average += final_val;
	// 	}

	// 	// cout << "final_val: " << final_val << endl;
	// }

	// cout << "first branch average: " << first_branch_average/first_branch_count << endl;
	// cout << "second branch average: " << second_branch_average/second_branch_count << endl;

	// double average_score = 0.5;

	int front_input_size = 2+2+2+2+2+3+2+2
						   +2+2+3;

	int fix_network_output_size = 1;	// equal to output size
	Network* fix_network = new Network(front_input_size,
									   100,
									   fix_network_output_size);

	// TODO: one more possibility is that 0 state is needed except for knowledge that branch was chosen

	int sup_front_network_output_size = 2;
	Network* sup_front_network = new Network(front_input_size,
											 100,
											 sup_front_network_output_size);

	int sup_back_network_input_size = 2
									  +2+3+2+2+2+2;
	int sup_back_network_output_size = 1;
	Network* sup_back_network = new Network(sup_back_network_input_size,
											100,
											sup_back_network_output_size);

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

		vector<double> front_inputs;
		front_inputs.reserve(front_input_size);
		vector<double> sup_back_network_inputs;
		sup_back_network_inputs.reserve(sup_back_network_input_size);

		double pre_0_0 = rand()%2*2-1;
		front_inputs.push_back(pre_0_0);
		double pre_0_1 = rand()%2*2-1;
		front_inputs.push_back(pre_0_1);

		double pre_1_0 = rand()%2*2-1;
		if (pre_1_0 == 1.0) {
			shared_val++;
		}
		front_inputs.push_back(pre_1_0);
		double pre_1_1 = rand()%2*2-1;
		front_inputs.push_back(pre_1_1);

		double pre_2_0 = rand()%2*2-1;
		front_inputs.push_back(pre_2_0);
		double pre_2_1 = rand()%2*2-1;
		front_inputs.push_back(pre_2_1);

		double pre_3_0 = rand()%2*2-1;
		second_score_modifier += pre_3_0;
		front_inputs.push_back(pre_3_0);
		double pre_3_1 = rand()%2*2-1;
		front_inputs.push_back(pre_3_1);

		double pre_4_0 = rand()%2*2-1;
		if (pre_4_0 == 1.0) {
			choice_val++;
		}
		front_inputs.push_back(pre_4_0);
		double pre_4_1 = rand()%2*2-1;
		front_inputs.push_back(pre_4_1);

		double pre_5_0 = rand()%2*2-1;
		front_inputs.push_back(pre_5_0);
		double pre_5_1 = rand()%2*2-1;
		if (pre_5_1 == 1.0) {
			second_val++;
		}
		front_inputs.push_back(pre_5_1);
		double pre_5_2 = rand()%2*2-1;
		front_inputs.push_back(pre_5_2);

		double pre_6_0;
		if (choice_val == 1) {
			pre_6_0 = -1.0;
		} else {
			pre_6_0 = 1.0;
			choice_val++;
		}
		front_inputs.push_back(pre_6_0);
		double pre_6_1 = rand()%2*2-1;
		front_inputs.push_back(pre_6_1);

		double pre_7_0 = rand()%2*2-1;
		front_inputs.push_back(pre_7_0);
		double pre_7_1 = rand()%2*2-1;
		front_inputs.push_back(pre_7_1);

		double new_0_0 = rand()%2*2-1;
		if (new_0_0 == 1.0) {
			second_val++;
		}
		front_inputs.push_back(new_0_0);
		double new_0_1 = rand()%2*2-1;
		front_inputs.push_back(new_0_1);

		double new_1_0 = rand()%2*2-1;
		front_inputs.push_back(new_1_0);
		double new_1_1 = rand()%2*2-1;
		front_inputs.push_back(new_1_1);

		double new_2_0 = rand()%2*2-1;
		if (new_2_0 == 1.0) {
			second_val++;
		}
		front_inputs.push_back(new_2_0);
		double new_2_1 = rand()%2*2-1;
		front_inputs.push_back(new_2_1);
		double new_2_2 = rand()%2*2-1;
		front_inputs.push_back(new_2_2);

		second_branch_front_network->activate(front_inputs);
		double predicted_score = second_branch_front_network->output->acti_vals[0];

		if (iter_index%10000 == 0) {
			cout << "initial predicted_score: " << predicted_score << endl;
		}

		fix_network->activate(front_inputs);
		vector<vector<double>> state_vals;
		state_vals.push_back(vector<double>{fix_network->output->acti_vals[0]});

		if (iter_index%10000 == 0) {
			cout << "starting state: " << fix_network->output->acti_vals[0] << endl;
		}

		sup_front_network->activate(front_inputs);
		sup_back_network_inputs.push_back(sup_front_network->output->acti_vals[0]);
		sup_back_network_inputs.push_back(sup_front_network->output->acti_vals[1]);

		double post_0_0 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_0_0);
		double post_0_1 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_0_1);
		{
			vector<double> node_input{post_0_0, post_0_1};
			nodes[9]->activate(state_vals,
							   node_input,
							   predicted_score);
		}

		double post_1_0 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_1_0);
		double post_1_1 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_1_1);
		double post_1_2 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_1_2);
		{
			vector<double> node_input{post_1_0, post_1_1, post_1_2};
			nodes[10]->activate(state_vals,
								node_input,
								predicted_score);
		}

		double post_2_0 = rand()%2*2-1;
		if (post_2_0 == 1.0) {
			second_val++;
		}
		sup_back_network_inputs.push_back(post_2_0);
		double post_2_1 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_2_1);
		{
			vector<double> node_input{post_2_0, post_2_1};
			nodes[11]->activate(state_vals,
								node_input,
								predicted_score);
		}

		double post_3_0 = rand()%2*2-1;
		if (post_3_0 == 1.0) {
			shared_val++;
		}
		sup_back_network_inputs.push_back(post_3_0);
		double post_3_1 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_3_1);
		{
			vector<double> node_input{post_3_0, post_3_1};
			nodes[12]->activate(state_vals,
								node_input,
								predicted_score);
		}

		double post_4_0 = rand()%2*2-1;
		shared_score_modifier += post_4_0;
		sup_back_network_inputs.push_back(post_4_0);
		double post_4_1 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_4_1);
		{
			vector<double> node_input{post_4_0, post_4_1};
			nodes[13]->activate(state_vals,
								node_input,
								predicted_score);
		}

		double post_5_0 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_5_0);
		double post_5_1 = rand()%2*2-1;
		sup_back_network_inputs.push_back(post_5_1);
		{
			vector<double> node_input{post_5_0, post_5_1};
			nodes[14]->activate(state_vals,
								node_input,
								predicted_score);
		}

		double final_val;
		if (choice_val%2 == 0) {
			final_val = -5.0;
		} else {
			final_val = (shared_val+second_val)%2 + second_score_modifier + shared_score_modifier;
		}

		sup_back_network->activate(sup_back_network_inputs);
		double combined_score = predicted_score + sup_back_network->output->acti_vals[0];

		if (iter_index%10000 == 0) {
			cout << "final_val: " << final_val << endl;
			cout << "predicted_score: " << predicted_score << endl;
			cout << "sup_back_network->output->acti_vals[0]: " << sup_back_network->output->acti_vals[0] << endl;
		}

		double error = final_val - combined_score;
		sum_error += abs(error);

		vector<double> sup_back_error{error};
		if (iter_index < 800000) {
			sup_back_network->backprop(sup_back_error, 0.01);
		} else {
			sup_back_network->backprop(sup_back_error, 0.001);
		}

		vector<double> sup_front_error;
		sup_front_error.push_back(sup_back_network->input->errors[0]);
		sup_back_network->input->errors[0] = 0.0;
		sup_front_error.push_back(sup_back_network->input->errors[1]);
		sup_back_network->input->errors[1] = 0.0;
		if (iter_index < 800000) {
			sup_front_network->backprop(sup_front_error, 0.01);
		} else {
			sup_front_network->backprop(sup_front_error, 0.001);
		}

		vector<vector<double>> state_errors;
		for (int n_index = 14; n_index >= 9; n_index--) {
			nodes[n_index]->backprop_errors_with_no_weight_change(
				state_errors,
				predicted_score,
				final_val);
		}

		if (iter_index%10000 == 0) {
			cout << "state error: " << state_errors[0][0] << endl;
		}

		vector<double> fix_error{state_errors[0][0]};
		if (iter_index < 800000) {
			fix_network->backprop(fix_error, 0.01);
		} else {
			fix_network->backprop(fix_error, 0.001);
		}
	}

	delete second_branch_choice_flat_network;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}

	cout << "Done" << endl;
}