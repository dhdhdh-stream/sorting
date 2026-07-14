#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "init_network.h"
#include "obs_network.h"
#include "score_network.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> init_states{0, 1, 2, 3};
	InitNetwork* init_network_1 = new InitNetwork(init_states,
												  4,
												  1);
	double init_1_hidden_1_average_max_update = 0.0;
	double init_1_hidden_2_average_max_update = 0.0;
	double init_1_output_average_max_update = 0.0;
	InitNetwork* init_network_2 = new InitNetwork(init_states,
												  4,
												  1);
	double init_2_hidden_1_average_max_update = 0.0;
	double init_2_hidden_2_average_max_update = 0.0;
	double init_2_output_average_max_update = 0.0;
	InitNetwork* init_network_3 = new InitNetwork(init_states,
												  4,
												  1);
	double init_3_hidden_1_average_max_update = 0.0;
	double init_3_hidden_2_average_max_update = 0.0;
	double init_3_output_average_max_update = 0.0;
	InitNetwork* init_network_4 = new InitNetwork(init_states,
												  4,
												  1);
	double init_4_hidden_1_average_max_update = 0.0;
	double init_4_hidden_2_average_max_update = 0.0;
	double init_4_output_average_max_update = 0.0;
	ScoreNetwork* score_network = new ScoreNetwork(4);
	double score_hidden_1_average_max_update = 0.0;
	double score_hidden_2_average_max_update = 0.0;
	double score_output_average_max_update = 0.0;

	double average_max_update = 0.0;

	uniform_int_distribution<int> input_distribution(-10, 10);
	// uniform_int_distribution<int> activate_distribution(0, 9);
	uniform_int_distribution<int> activate_distribution(1, 1);
	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		double target_val = input_distribution(generator);

		double o_val_1 = target_val;
		double o_val_2 = input_distribution(generator);
		double o_val_3 = input_distribution(generator);
		double o_val_4 = target_val;

		vector<double> state_vals;
		vector<double> new_state_vals(4, 0.0);

		bool on_1 = activate_distribution(generator) != 0;
		if (on_1) {
			vector<double> o_obs_input_vals_1{o_val_1};
			init_network_1->init_activate(state_vals,
										  new_state_vals,
										  o_obs_input_vals_1);
		}
		bool on_2 = activate_distribution(generator) != 0;
		if (on_2) {
			vector<double> o_obs_input_vals_2{o_val_2};
			init_network_2->init_activate(state_vals,
										  new_state_vals,
										  o_obs_input_vals_2);
		}
		bool on_3 = activate_distribution(generator) != 0;
		if (on_3) {
			vector<double> o_obs_input_vals_3{o_val_3};
			init_network_3->init_activate(state_vals,
										  new_state_vals,
										  o_obs_input_vals_3);
		}
		bool on_4 = activate_distribution(generator) != 0;
		if (on_4) {
			vector<double> o_obs_input_vals_4{o_val_4};
			init_network_4->init_activate(state_vals,
										  new_state_vals,
										  o_obs_input_vals_4);
		}
		vector<double> combined_state;
		combined_state.insert(combined_state.end(), state_vals.begin(), state_vals.end());
		combined_state.insert(combined_state.end(), new_state_vals.begin(), new_state_vals.end());
		score_network->activate(combined_state);

		score_network->init_backprop(target_val);

		vector<double> new_state_errors(4, 0.0);

		for (int s_index = 0; s_index < 4; s_index++) {
			new_state_errors[s_index] = score_network->state_input->errors(s_index);
			score_network->state_input->errors(s_index) = 0.0;
		}

		if (on_4) {
			init_network_4->init_backprop(new_state_errors);
		}
		if (on_3) {
			init_network_3->init_backprop(new_state_errors);
		}
		if (on_2) {
			init_network_2->init_backprop(new_state_errors);
		}
		if (on_1) {
			init_network_1->init_backprop(new_state_errors);
		}

		if ((iter_index+1)%20 == 0) {
			init_network_1->init_update(init_1_hidden_1_average_max_update,
										init_1_hidden_2_average_max_update,
										init_1_output_average_max_update);
			init_network_2->init_update(init_2_hidden_1_average_max_update,
										init_2_hidden_2_average_max_update,
										init_2_output_average_max_update);
			init_network_3->init_update(init_3_hidden_1_average_max_update,
										init_3_hidden_2_average_max_update,
										init_3_output_average_max_update);
			init_network_4->init_update(init_4_hidden_1_average_max_update,
										init_4_hidden_2_average_max_update,
										init_4_output_average_max_update);
			score_network->init_update(score_hidden_1_average_max_update,
									   score_hidden_2_average_max_update,
									   score_output_average_max_update);
		}

		if ((iter_index+1)%10000 == 0) {
			cout << iter_index << endl;
			cout << "state_vals:";
			for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
				cout << " " << state_vals[s_index];
			}
			cout << endl;
			cout << "target_val: " << target_val << endl;
			cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
			cout << "average_max_update: " << average_max_update << endl;
			cout << endl;
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		cout << iter_index << endl;

		double target_val = input_distribution(generator);
		cout << "target_val: " << target_val << endl;

		double o_val_1 = target_val;
		double o_val_2 = input_distribution(generator);
		double o_val_3 = input_distribution(generator);
		double o_val_4 = target_val;

		{
			vector<double> state_vals(4, 0.0);

			vector<double> obs_input_vals_1{o_val_1};
			init_network_1->activate(state_vals,
									 obs_input_vals_1);
			vector<double> obs_input_vals_2{o_val_2};
			init_network_2->activate(state_vals,
									 obs_input_vals_2);
			vector<double> obs_input_vals_3{o_val_3};
			init_network_3->activate(state_vals,
									 obs_input_vals_3);
			vector<double> obs_input_vals_4{o_val_4};
			init_network_4->activate(state_vals,
									 obs_input_vals_4);
			score_network->activate(state_vals);

			cout << "both: " << score_network->output->acti_vals(0) << endl;
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		cout << iter_index << endl;

		double target_val = input_distribution(generator);
		cout << "target_val: " << target_val << endl;

		double o_val_1 = target_val;
		double o_val_2 = input_distribution(generator);

		{
			vector<double> state_vals(4, 0.0);

			vector<double> obs_input_vals_1{o_val_1};
			init_network_1->activate(state_vals,
									 obs_input_vals_1);
			vector<double> obs_input_vals_2{o_val_2};
			init_network_2->activate(state_vals,
									 obs_input_vals_2);
			score_network->activate(state_vals);

			cout << "front: " << score_network->output->acti_vals(0) << endl;
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		cout << iter_index << endl;

		double target_val = input_distribution(generator);
		cout << "target_val: " << target_val << endl;

		double o_val_3 = input_distribution(generator);
		double o_val_4 = target_val;

		{
			vector<double> state_vals(4, 0.0);

			vector<double> obs_input_vals_3{o_val_3};
			init_network_3->activate(state_vals,
									 obs_input_vals_3);
			vector<double> obs_input_vals_4{o_val_4};
			init_network_4->activate(state_vals,
									 obs_input_vals_4);
			score_network->activate(state_vals);

			cout << "back: " << score_network->output->acti_vals(0) << endl;
		}
	}

	cout << "Done" << endl;
}
