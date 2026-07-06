#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "init_network.h"
#include "negate_network.h"
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
	double init_1_hidden_3_average_max_update = 0.0;
	double init_1_output_average_max_update = 0.0;
	InitNetwork* init_network_2 = new InitNetwork(init_states,
												  4,
												  1);
	double init_2_hidden_1_average_max_update = 0.0;
	double init_2_hidden_2_average_max_update = 0.0;
	double init_2_hidden_3_average_max_update = 0.0;
	double init_2_output_average_max_update = 0.0;
	InitNetwork* init_network_3 = new InitNetwork(init_states,
												  4,
												  1);
	double init_3_hidden_1_average_max_update = 0.0;
	double init_3_hidden_2_average_max_update = 0.0;
	double init_3_hidden_3_average_max_update = 0.0;
	double init_3_output_average_max_update = 0.0;
	InitNetwork* init_network_4 = new InitNetwork(init_states,
												  4,
												  1);
	double init_4_hidden_1_average_max_update = 0.0;
	double init_4_hidden_2_average_max_update = 0.0;
	double init_4_hidden_3_average_max_update = 0.0;
	double init_4_output_average_max_update = 0.0;
	ScoreNetwork* score_network = new ScoreNetwork(4);
	double score_hidden_1_average_max_update = 0.0;
	double score_hidden_2_average_max_update = 0.0;
	double score_hidden_3_average_max_update = 0.0;
	double score_output_average_max_update = 0.0;

	NegateNetwork* negate_network_1 = new NegateNetwork(0);
	NegateNetwork* negate_network_2 = new NegateNetwork(1);
	NegateNetwork* negate_network_3 = new NegateNetwork(2);
	NegateNetwork* negate_network_4 = new NegateNetwork(3);

	uniform_int_distribution<int> input_distribution(-10, 10);

	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);
		double val_3 = input_distribution(generator);
		double val_4 = input_distribution(generator);

		double target_val_1 = val_1 + val_4;

		double val_5 = input_distribution(generator);
		double val_6 = input_distribution(generator);
		double val_7 = input_distribution(generator);
		double val_8 = input_distribution(generator);

		double target_val_2 = val_5 + val_8;

		{
			vector<double> state_vals(4, 0.0);

			vector<double> obs_input_vals_1{val_1};
			init_network_1->activate(state_vals,
									 obs_input_vals_1);
			vector<double> obs_input_vals_2{val_2};
			init_network_2->activate(state_vals,
									 obs_input_vals_2);
			vector<double> obs_input_vals_3{val_3};
			init_network_3->activate(state_vals,
									 obs_input_vals_3);
			vector<double> obs_input_vals_4{val_4};
			init_network_4->activate(state_vals,
									 obs_input_vals_4);
			score_network->activate(state_vals);

			vector<double> state_errors(4, 0.0);

			score_network->backprop(target_val_1,
									state_errors);
			init_network_4->backprop(state_errors);
			init_network_3->backprop(state_errors);
			init_network_2->backprop(state_errors);
			init_network_1->backprop(state_errors);

			if ((iter_index+1)%10000 == 0) {
				cout << iter_index << endl;
				cout << "state_vals:";
				for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
					cout << " " << state_vals[s_index];
				}
				cout << endl;
				cout << "target_val_1: " << target_val_1 << endl;
				cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
				cout << endl;
			}
		}

		{
			vector<double> state_vals(4, 0.0);

			vector<double> obs_input_vals_1{val_5};
			init_network_1->activate(state_vals,
									 obs_input_vals_1);
			vector<double> obs_input_vals_2{val_6};
			init_network_2->activate(state_vals,
									 obs_input_vals_2);
			vector<double> obs_input_vals_3{val_7};
			init_network_3->activate(state_vals,
									 obs_input_vals_3);
			vector<double> obs_input_vals_4{val_8};
			init_network_4->activate(state_vals,
									 obs_input_vals_4);
			score_network->activate(state_vals);

			vector<double> state_errors(4, 0.0);

			score_network->backprop(target_val_2,
									state_errors);
			init_network_4->backprop(state_errors);
			init_network_3->backprop(state_errors);
			init_network_2->backprop(state_errors);
			init_network_1->backprop(state_errors);

			if ((iter_index+1)%10000 == 0) {
				cout << iter_index << endl;
				cout << "state_vals:";
				for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
					cout << " " << state_vals[s_index];
				}
				cout << endl;
				cout << "target_val_2: " << target_val_2 << endl;
				cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
				cout << endl;
			}
		}

		if ((iter_index+1)%20 == 0) {
			init_network_1->init_update(init_1_hidden_1_average_max_update,
										init_1_hidden_2_average_max_update,
										init_1_hidden_3_average_max_update,
										init_1_output_average_max_update);
			init_network_2->init_update(init_2_hidden_1_average_max_update,
										init_2_hidden_2_average_max_update,
										init_2_hidden_3_average_max_update,
										init_2_output_average_max_update);
			init_network_3->init_update(init_3_hidden_1_average_max_update,
										init_3_hidden_2_average_max_update,
										init_3_hidden_3_average_max_update,
										init_3_output_average_max_update);
			init_network_4->init_update(init_4_hidden_1_average_max_update,
										init_4_hidden_2_average_max_update,
										init_4_hidden_3_average_max_update,
										init_4_output_average_max_update);
			score_network->init_update(score_hidden_1_average_max_update,
									   score_hidden_2_average_max_update,
									   score_hidden_3_average_max_update,
									   score_output_average_max_update);
		}
	}

	for (int iter_index = 0; iter_index < 10; iter_index++) {
		cout << iter_index << endl;

		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);
		double val_3 = input_distribution(generator);
		double val_4 = input_distribution(generator);

		double target_val_1 = val_1 + val_4;
		cout << "target_val_1: " << target_val_1 << endl;

		double val_5 = input_distribution(generator);
		double val_6 = input_distribution(generator);
		double val_7 = input_distribution(generator);
		double val_8 = input_distribution(generator);

		double target_val_2 = val_5 + val_8;
		cout << "target_val_2: " << target_val_2 << endl;

		vector<double> state_vals(4, 0.0);

		vector<double> obs_input_vals_1{val_1};
		init_network_1->activate(state_vals,
								 obs_input_vals_1);
		vector<double> obs_input_vals_2{val_2};
		init_network_2->activate(state_vals,
								 obs_input_vals_2);
		vector<double> obs_input_vals_3{val_3};
		init_network_3->activate(state_vals,
								 obs_input_vals_3);
		vector<double> obs_input_vals_4{val_4};
		init_network_4->activate(state_vals,
								 obs_input_vals_4);
		score_network->activate(state_vals);

		cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;

		negate_network_1->activate(state_vals);
		negate_network_2->activate(state_vals);
		negate_network_3->activate(state_vals);
		negate_network_4->activate(state_vals);

		vector<double> obs_input_vals_5{val_5};
		init_network_1->activate(state_vals,
								 obs_input_vals_5);
		vector<double> obs_input_vals_6{val_6};
		init_network_2->activate(state_vals,
								 obs_input_vals_6);
		vector<double> obs_input_vals_7{val_7};
		init_network_3->activate(state_vals,
								 obs_input_vals_7);
		vector<double> obs_input_vals_8{val_8};
		init_network_4->activate(state_vals,
								 obs_input_vals_8);
		score_network->activate(state_vals);

		cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
	}

	double average_max_update = 0.0;

	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		if ((iter_index+1)%10000 == 0) {
			cout << iter_index << endl;
		}

		double val_1 = input_distribution(generator);
		double val_2 = input_distribution(generator);
		double val_3 = input_distribution(generator);
		double val_4 = input_distribution(generator);

		double target_val_1 = val_1 + val_4;

		double val_5 = input_distribution(generator);
		double val_6 = input_distribution(generator);
		double val_7 = input_distribution(generator);
		double val_8 = input_distribution(generator);

		double target_val_2 = val_5 + val_8;

		vector<double> state_vals(4, 0.0);

		vector<double> obs_input_vals_1{val_1};
		init_network_1->activate(state_vals,
								 obs_input_vals_1);
		InitNetworkHistory* init_network_history_1 = new InitNetworkHistory(init_network_1);
		init_network_1->save(init_network_history_1);
		vector<double> obs_input_vals_2{val_2};
		init_network_2->activate(state_vals,
								 obs_input_vals_2);
		InitNetworkHistory* init_network_history_2 = new InitNetworkHistory(init_network_2);
		init_network_2->save(init_network_history_2);
		vector<double> obs_input_vals_3{val_3};
		init_network_3->activate(state_vals,
								 obs_input_vals_3);
		InitNetworkHistory* init_network_history_3 = new InitNetworkHistory(init_network_3);
		init_network_3->save(init_network_history_3);
		vector<double> obs_input_vals_4{val_4};
		init_network_4->activate(state_vals,
								 obs_input_vals_4);
		InitNetworkHistory* init_network_history_4 = new InitNetworkHistory(init_network_4);
		init_network_4->save(init_network_history_4);
		score_network->activate(state_vals);
		ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(score_network);
		score_network->save(score_network_history);

		if ((iter_index+1)%10000 == 0) {
			cout << "target_val_1: " << target_val_1 << endl;
			cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
		}

		negate_network_1->activate(state_vals);
		negate_network_2->activate(state_vals);
		negate_network_3->activate(state_vals);
		negate_network_4->activate(state_vals);

		vector<double> obs_input_vals_5{val_5};
		init_network_1->activate(state_vals,
								 obs_input_vals_5);
		vector<double> obs_input_vals_6{val_6};
		init_network_2->activate(state_vals,
								 obs_input_vals_6);
		vector<double> obs_input_vals_7{val_7};
		init_network_3->activate(state_vals,
								 obs_input_vals_7);
		vector<double> obs_input_vals_8{val_8};
		init_network_4->activate(state_vals,
								 obs_input_vals_8);
		score_network->activate(state_vals);

		if ((iter_index+1)%10000 == 0) {
			cout << "target_val_2: " << target_val_2 << endl;
			cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
		}

		vector<double> state_errors(4, 0.0);

		score_network->backprop(target_val_2,
								state_errors);
		init_network_4->backprop(state_errors);
		init_network_3->backprop(state_errors);
		init_network_2->backprop(state_errors);
		init_network_1->backprop(state_errors);
		negate_network_4->backprop(state_errors);
		negate_network_3->backprop(state_errors);
		negate_network_2->backprop(state_errors);
		negate_network_1->backprop(state_errors);
		score_network->load(score_network_history);
		delete score_network_history;
		score_network->backprop(target_val_1,
								state_errors);
		init_network_4->load(init_network_history_4);
		delete init_network_history_4;
		init_network_4->backprop(state_errors);
		init_network_3->load(init_network_history_3);
		delete init_network_history_3;
		init_network_3->backprop(state_errors);
		init_network_2->load(init_network_history_2);
		delete init_network_history_2;
		init_network_2->backprop(state_errors);
		init_network_1->load(init_network_history_1);
		delete init_network_history_1;
		init_network_1->backprop(state_errors);

		if ((iter_index+1)%20 == 0) {
			double max_update = 0.0;
			init_network_1->get_max_update(max_update);
			init_network_2->get_max_update(max_update);
			init_network_3->get_max_update(max_update);
			init_network_4->get_max_update(max_update);
			negate_network_1->get_max_update(max_update);
			negate_network_2->get_max_update(max_update);
			negate_network_3->get_max_update(max_update);
			negate_network_4->get_max_update(max_update);
			score_network->get_max_update(max_update);
			average_max_update = 0.999*average_max_update + 0.001*max_update;
			if (max_update > 0.0) {
				double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/average_max_update;
				if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
					learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
				}
				init_network_1->update_weights(learning_rate);
				init_network_2->update_weights(learning_rate);
				init_network_3->update_weights(learning_rate);
				init_network_4->update_weights(learning_rate);
				negate_network_1->update_weights(learning_rate);
				negate_network_2->update_weights(learning_rate);
				negate_network_3->update_weights(learning_rate);
				negate_network_4->update_weights(learning_rate);
				score_network->update_weights(learning_rate);
			}
		}

		if ((iter_index+1)%10000 == 0) {
			cout << "negate_network_1->weight: " << negate_network_1->weight << endl;
			cout << "negate_network_2->weight: " << negate_network_2->weight << endl;
			cout << "negate_network_3->weight: " << negate_network_3->weight << endl;
			cout << "negate_network_4->weight: " << negate_network_4->weight << endl;

			cout << endl;
		}
	}

	delete init_network_1;
	delete init_network_2;
	delete init_network_3;
	delete init_network_4;
	delete score_network;
	delete negate_network_1;
	delete negate_network_2;
	delete negate_network_3;
	delete negate_network_4;

	cout << "Done" << endl;
}
