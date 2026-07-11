// - might even be too good at shifting
//   - current learning rate too aggressive?

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
	ObsNetwork* obs_network_1 = new ObsNetwork(4, 1);
	InitNetwork* init_network_2 = new InitNetwork(init_states,
												  4,
												  1);
	double init_2_hidden_1_average_max_update = 0.0;
	double init_2_hidden_2_average_max_update = 0.0;
	double init_2_output_average_max_update = 0.0;
	ObsNetwork* obs_network_2 = new ObsNetwork(4, 1);
	InitNetwork* init_network_3 = new InitNetwork(init_states,
												  4,
												  1);
	double init_3_hidden_1_average_max_update = 0.0;
	double init_3_hidden_2_average_max_update = 0.0;
	double init_3_output_average_max_update = 0.0;
	ObsNetwork* obs_network_3 = new ObsNetwork(4, 1);
	InitNetwork* init_network_4 = new InitNetwork(init_states,
												  4,
												  1);
	double init_4_hidden_1_average_max_update = 0.0;
	double init_4_hidden_2_average_max_update = 0.0;
	double init_4_output_average_max_update = 0.0;
	ObsNetwork* obs_network_4 = new ObsNetwork(4, 1);
	ScoreNetwork* score_network = new ScoreNetwork(4);
	double score_hidden_1_average_max_update = 0.0;
	double score_hidden_2_average_max_update = 0.0;
	double score_output_average_max_update = 0.0;

	uniform_int_distribution<int> input_distribution(-10, 10);
	uniform_int_distribution<int> non_match_distribution(0, 3);
	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		double target_val = input_distribution(generator);

		double val_1;
		if (non_match_distribution(generator) == 0) {
			val_1 = input_distribution(generator);
		} else {
			val_1 = target_val;
		}
		double val_2 = input_distribution(generator);
		double val_3 = input_distribution(generator);
		double val_4 = input_distribution(generator);

		// double o_val_1 = input_distribution(generator);
		// double o_val_2 = input_distribution(generator);
		// double o_val_3 = input_distribution(generator);
		// double o_val_4 = input_distribution(generator);

		vector<double> state_vals;
		vector<double> new_state_vals(4, 0.0);

		vector<double> obs_input_vals_1{val_1};
		init_network_1->init_activate(state_vals,
									  new_state_vals,
									  obs_input_vals_1);
		vector<double> obs_input_vals_2{val_2};
		init_network_2->init_activate(state_vals,
									  new_state_vals,
									  obs_input_vals_2);
		vector<double> obs_input_vals_3{val_3};
		init_network_3->init_activate(state_vals,
									  new_state_vals,
									  obs_input_vals_3);
		vector<double> obs_input_vals_4{val_4};
		init_network_4->init_activate(state_vals,
									  new_state_vals,
									  obs_input_vals_4);
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

		init_network_4->init_backprop(new_state_errors);
		init_network_3->init_backprop(new_state_errors);
		init_network_2->init_backprop(new_state_errors);
		init_network_1->init_backprop(new_state_errors);

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
			cout << "new_state_vals:";
			for (int s_index = 0; s_index < (int)new_state_vals.size(); s_index++) {
				cout << " " << new_state_vals[s_index];
			}
			cout << endl;
			cout << "target_val: " << target_val << endl;
			cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
			cout << "score_hidden_1_average_max_update: " << score_hidden_1_average_max_update << endl;
			cout << "score_hidden_2_average_max_update: " << score_hidden_2_average_max_update << endl;
			cout << "score_output_average_max_update: " << score_output_average_max_update << endl;
			cout << endl;
		}
	}

	double average_max_update = 0.0;

	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		double target_val = input_distribution(generator);

		double val_1;
		if (non_match_distribution(generator) == 0) {
			val_1 = input_distribution(generator);
		} else {
			val_1 = target_val;
		}
		double val_2 = input_distribution(generator);
		double val_3 = input_distribution(generator);
		double val_4 = input_distribution(generator);

		double o_val_1 = target_val;
		double o_val_2 = input_distribution(generator);
		double o_val_3 = input_distribution(generator);
		double o_val_4 = input_distribution(generator);

		vector<double> state_vals(4, 0.0);

		vector<double> obs_input_vals_1{val_1};
		init_network_1->activate(state_vals,
								 obs_input_vals_1);
		vector<double> o_obs_input_vals_1{o_val_1};
		obs_network_1->activate(state_vals,
								o_obs_input_vals_1);
		vector<double> obs_input_vals_2{val_2};
		init_network_2->activate(state_vals,
								 obs_input_vals_2);
		vector<double> o_obs_input_vals_2{o_val_2};
		obs_network_2->activate(state_vals,
								o_obs_input_vals_2);
		vector<double> obs_input_vals_3{val_3};
		init_network_3->activate(state_vals,
								 obs_input_vals_3);
		vector<double> o_obs_input_vals_3{o_val_3};
		obs_network_3->activate(state_vals,
								o_obs_input_vals_3);
		vector<double> obs_input_vals_4{val_4};
		init_network_4->activate(state_vals,
								 obs_input_vals_4);
		vector<double> o_obs_input_vals_4{o_val_4};
		obs_network_4->activate(state_vals,
								o_obs_input_vals_4);
		score_network->activate(state_vals);

		vector<double> state_errors(4, 0.0);

		score_network->backprop(target_val,
								state_errors);
		obs_network_4->backprop(state_errors);
		init_network_4->backprop(state_errors);
		obs_network_3->backprop(state_errors);
		init_network_3->backprop(state_errors);
		obs_network_2->backprop(state_errors);
		init_network_2->backprop(state_errors);
		obs_network_1->backprop(state_errors);
		init_network_1->backprop(state_errors);

		double max_state_val = 0.0;
		for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
			double state_size = abs(state_vals[s_index]);
			if (state_size > max_state_val) {
				max_state_val = state_size;
			}
		}
		max_state_val = max(1.0, max_state_val);
		double max_state_error = 0.0;
		for (int e_index = 0; e_index < (int)state_errors.size(); e_index++) {
			double error_size = abs(state_errors[e_index]);
			if (error_size > max_state_error) {
				max_state_error = error_size;
			}
		}
		max_state_error = max(abs(target_val), max_state_error);
		double max_update = max_state_val * max_state_error;
		average_max_update = 0.999*average_max_update + 0.001*max_update;
		double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/average_max_update;
		if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
			learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
		}

		init_network_1->update_weights(learning_rate);
		init_network_2->update_weights(learning_rate);
		init_network_3->update_weights(learning_rate);
		init_network_4->update_weights(learning_rate);
		obs_network_1->update_weights(learning_rate);
		obs_network_2->update_weights(learning_rate);
		obs_network_3->update_weights(learning_rate);
		obs_network_4->update_weights(learning_rate);
		score_network->update_weights(learning_rate);

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

	cout << "Done" << endl;
}
