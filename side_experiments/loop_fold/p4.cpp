#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// Network* full_init_network = new Network(1, 10, 2);
	// Network* full_network = new Network(5, 100, 2);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	if (epoch_index%100 == 0) {
	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		double score_state = 0.0;
	// 		double loop_state = 0.0;

	// 		// behind the loop
	// 		int second_val_on = -1+rand()%2*2;
	// 		cout << "second_val_on: " << second_val_on << endl;

	// 		vector<double> init_input{(double)second_val_on};
	// 		full_init_network->activate(init_input);
	// 		score_state = full_init_network->output->acti_vals[0];
	// 		loop_state = full_init_network->output->acti_vals[1];
	// 		cout << "starting state: " << score_state << endl;

	// 		int sum = 0;
	// 		for (int i = 0; i < rand()%6; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			cout << i << " first_value: " << first_value << endl;
	// 			cout << i << " second_value: " << second_value << endl;

	// 			sum += first_value;
	// 			if (second_val_on == 1) {
	// 				sum += second_value;
	// 			}

	// 			vector<double> input;
	// 			input.push_back(score_state);
	// 			input.push_back(loop_state);
	// 			input.push_back(second_val_on);
	// 			input.push_back(first_value);
	// 			input.push_back(second_value);
	// 			full_network->activate(input, network_historys);
	// 			score_state = full_network->output->acti_vals[0];
	// 			loop_state = full_network->output->acti_vals[1];
	// 			cout << i << " score_state: " << score_state << endl;
	// 		}

	// 		double score_state_error = sum - score_state;
	// 		sum_error += abs(score_state_error);
	// 		double loop_state_error = 0.0;

	// 		cout << "sum: " << sum << endl;
	// 		cout << "score_state_error: " << score_state_error << endl;

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			vector<double> errors{score_state_error, loop_state_error};
	// 			if (epoch_index < 4000) {
	// 				full_network->backprop(errors, 0.01);
	// 			} else {
	// 				full_network->backprop(errors, 0.001);
	// 			}
	// 			score_state_error = full_network->input->errors[0];
	// 			full_network->input->errors[0] = 0.0;
	// 			loop_state_error = full_network->input->errors[1];
	// 			full_network->input->errors[1] = 0.0;

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		vector<double> errors{score_state_error, loop_state_error};
	// 		if (epoch_index < 4000) {
	// 			full_init_network->backprop(errors, 0.01);
	// 		} else {
	// 			full_init_network->backprop(errors, 0.001);
	// 		}
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		double score_state = 0.0;
	// 		double loop_state = 0.0;

	// 		// behind the loop
	// 		int second_val_on = -1+rand()%2*2;

	// 		vector<double> init_input{(double)second_val_on};
	// 		full_init_network->activate(init_input);
	// 		score_state = full_init_network->output->acti_vals[0];
	// 		loop_state = full_init_network->output->acti_vals[1];

	// 		int sum = 0;
	// 		for (int i = 0; i < rand()%6; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;

	// 			sum += first_value;
	// 			if (second_val_on == 1) {
	// 				sum += second_value;
	// 			}

	// 			vector<double> input;
	// 			input.push_back(score_state);
	// 			input.push_back(loop_state);
	// 			input.push_back(second_val_on);
	// 			input.push_back(first_value);
	// 			input.push_back(second_value);
	// 			full_network->activate(input, network_historys);
	// 			score_state = full_network->output->acti_vals[0];
	// 			loop_state = full_network->output->acti_vals[1];
	// 		}

	// 		double score_state_error = sum - score_state;
	// 		sum_error += abs(score_state_error);
	// 		double loop_state_error = 0.0;

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			vector<double> errors{score_state_error, loop_state_error};
	// 			if (epoch_index < 4000) {
	// 				full_network->backprop(errors, 0.01);
	// 			} else {
	// 				full_network->backprop(errors, 0.001);
	// 			}
	// 			score_state_error = full_network->input->errors[0];
	// 			full_network->input->errors[0] = 0.0;
	// 			loop_state_error = full_network->input->errors[1];
	// 			full_network->input->errors[1] = 0.0;

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		vector<double> errors{score_state_error, loop_state_error};
	// 		if (epoch_index < 40000) {
	// 			full_init_network->backprop(errors, 0.01);
	// 		} else {
	// 			full_init_network->backprop(errors, 0.001);
	// 		}
	// 	}
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/full_init_network.txt");
	// 	full_init_network->save(output_file);
	// 	output_file.close();
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/full_network.txt");
	// 	full_network->save(output_file);
	// 	output_file.close();
	// }

	// Network* front_init_network = new Network(0, 0, 2);
	// Network* front_network = new Network(4, 100, 2);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	if (epoch_index%100 == 0) {
	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		double score_state = 0.0;
	// 		double loop_state = 0.0;

	// 		// behind the loop
	// 		int second_val_on = -1+rand()%2*2;
	// 		cout << "second_val_on: " << second_val_on << endl;

	// 		vector<double> init_input;
	// 		front_init_network->activate(init_input);
	// 		score_state = front_init_network->output->acti_vals[0];
	// 		loop_state = front_init_network->output->acti_vals[1];
	// 		cout << "starting state: " << score_state << endl;

	// 		int sum = 0;
	// 		for (int i = 0; i < rand()%6; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			cout << i << " first_value: " << first_value << endl;
	// 			cout << i << " second_value: " << second_value << endl;

	// 			sum += first_value;
	// 			if (second_val_on == 1) {
	// 				sum += second_value;
	// 			}

	// 			vector<double> input;
	// 			input.push_back(score_state);
	// 			input.push_back(loop_state);
	// 			input.push_back(first_value);
	// 			input.push_back(second_value);
	// 			front_network->activate(input, network_historys);
	// 			score_state = front_network->output->acti_vals[0];
	// 			loop_state = front_network->output->acti_vals[1];
	// 			cout << i << " score_state: " << score_state << endl;
	// 		}

	// 		double score_state_error = sum - score_state;
	// 		sum_error += abs(score_state_error);
	// 		double loop_state_error = 0.0;

	// 		cout << "sum: " << sum << endl;
	// 		cout << "score_state_error: " << score_state_error << endl;

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			vector<double> errors{score_state_error, loop_state_error};
	// 			if (epoch_index < 4000) {
	// 				front_network->backprop(errors, 0.01);
	// 			} else {
	// 				front_network->backprop(errors, 0.001);
	// 			}
	// 			score_state_error = front_network->input->errors[0];
	// 			front_network->input->errors[0] = 0.0;
	// 			loop_state_error = front_network->input->errors[1];
	// 			front_network->input->errors[1] = 0.0;

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		vector<double> errors{score_state_error, loop_state_error};
	// 		if (epoch_index < 4000) {
	// 			front_init_network->backprop(errors, 0.01);
	// 		} else {
	// 			front_init_network->backprop(errors, 0.001);
	// 		}
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		double score_state = 0.0;
	// 		double loop_state = 0.0;

	// 		// behind the loop
	// 		int second_val_on = -1+rand()%2*2;

	// 		vector<double> init_input;
	// 		front_init_network->activate(init_input);
	// 		score_state = front_init_network->output->acti_vals[0];
	// 		loop_state = front_init_network->output->acti_vals[1];

	// 		int sum = 0;
	// 		for (int i = 0; i < rand()%6; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;

	// 			sum += first_value;
	// 			if (second_val_on == 1) {
	// 				sum += second_value;
	// 			}

	// 			vector<double> input;
	// 			input.push_back(score_state);
	// 			input.push_back(loop_state);
	// 			input.push_back(first_value);
	// 			input.push_back(second_value);
	// 			front_network->activate(input, network_historys);
	// 			score_state = front_network->output->acti_vals[0];
	// 			loop_state = front_network->output->acti_vals[1];
	// 		}

	// 		double score_state_error = sum - score_state;
	// 		sum_error += abs(score_state_error);
	// 		double loop_state_error = 0.0;

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			vector<double> errors{score_state_error, loop_state_error};
	// 			if (epoch_index < 4000) {
	// 				front_network->backprop(errors, 0.01);
	// 			} else {
	// 				front_network->backprop(errors, 0.001);
	// 			}
	// 			score_state_error = front_network->input->errors[0];
	// 			front_network->input->errors[0] = 0.0;
	// 			loop_state_error = front_network->input->errors[1];
	// 			front_network->input->errors[1] = 0.0;

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		vector<double> errors{score_state_error, loop_state_error};
	// 		if (epoch_index < 4000) {
	// 			front_init_network->backprop(errors, 0.01);
	// 		} else {
	// 			front_init_network->backprop(errors, 0.001);
	// 		}
	// 	}
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/front_init_network.txt");
	// 	front_init_network->save(output_file);
	// 	output_file.close();
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/front_network.txt");
	// 	front_network->save(output_file);
	// 	output_file.close();
	// }

	ifstream full_init_input_file;
	full_init_input_file.open("saves/full_init_network.txt");
	Network* full_init_network = new Network(full_init_input_file);
	full_init_input_file.close();
	
	ifstream full_input_file;
	full_input_file.open("saves/full_network.txt");
	Network* full_network = new Network(full_input_file);
	full_input_file.close();

	ifstream front_init_input_file;
	front_init_input_file.open("saves/front_init_network.txt");
	Network* front_init_network = new Network(front_init_input_file);
	front_init_input_file.close();
	
	ifstream front_input_file;
	front_input_file.open("saves/front_network.txt");
	Network* front_network = new Network(front_input_file);
	front_input_file.close();

	Network* partial_init_network = new Network(0, 0, 1);
	Network* partial_network = new Network(3, 50, 1);
	Network* partial_combine_network = new Network(2, 20, 1);

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 20000; epoch_index++) {
		if (epoch_index%100 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			vector<AbstractNetworkHistory*> network_historys;
			vector<double> score_diffs;

			double full_score_state = 0.0;
			double full_loop_state = 0.0;

			double front_score_state = 0.0;
			double front_loop_state = 0.0;

			double partial_loop_state = 0.0;

			// behind the loop
			int second_val_on = -1+rand()%2*2;

			vector<double> full_init_input{(double)second_val_on};
			full_init_network->activate(full_init_input);
			full_score_state = full_init_network->output->acti_vals[0];
			full_loop_state = full_init_network->output->acti_vals[1];

			vector<double> front_init_input;
			front_init_network->activate(front_init_input);
			front_score_state = front_init_network->output->acti_vals[0];
			front_loop_state = front_init_network->output->acti_vals[1];

			vector<double> partial_init_input;
			partial_init_network->activate(partial_init_input);
			partial_loop_state = partial_init_network->output->acti_vals[0];

			int sum = 0;
			for (int i = 0; i < rand()%6; i++) {
				int first_value = rand()%2;
				int second_value = rand()%2;

				sum += first_value;
				if (second_val_on == 1) {
					sum += second_value;
				}

				vector<double> full_input;
				full_input.push_back(full_score_state);
				full_input.push_back(full_loop_state);
				full_input.push_back(second_val_on);
				full_input.push_back(first_value);
				full_input.push_back(second_value);
				full_network->activate(full_input);
				full_score_state = full_network->output->acti_vals[0];
				full_loop_state = full_network->output->acti_vals[1];

				vector<double> front_input;
				front_input.push_back(front_score_state);
				front_input.push_back(front_loop_state);
				front_input.push_back(first_value);
				front_input.push_back(second_value);
				front_network->activate(front_input);
				front_score_state = front_network->output->acti_vals[0];
				front_loop_state = front_network->output->acti_vals[1];

				vector<double> partial_input;
				partial_input.push_back(partial_loop_state);
				partial_input.push_back(first_value);
				partial_input.push_back(second_value);
				partial_network->activate(partial_input, network_historys);
				partial_loop_state = partial_network->output->acti_vals[0];
			}

			vector<double> partial_combine_input;
			partial_combine_input.push_back(partial_loop_state);
			partial_combine_input.push_back(second_val_on);
			partial_combine_network->activate(partial_combine_input);
			double diff = partial_combine_network->output->acti_vals[0];

			double partial_error = full_score_state - front_score_state - diff;
			sum_error += abs(partial_error);

			vector<double> partial_combine_error{partial_error};
			if (epoch_index < 4000) {
				partial_combine_network->backprop(partial_combine_error, 0.01);
			} else {
				partial_combine_network->backprop(partial_combine_error, 0.001);
			}
			if (partial_combine_network->epoch_iter == 0) {
				partial_combine_network->output->constants[0] = 0.0;
			}
			double partial_loop_state_error = partial_combine_network->input->errors[0];
			partial_combine_network->input->errors[0] = 0.0;

			while (network_historys.size() > 0) {
				network_historys.back()->reset_weights();

				vector<double> errors{partial_loop_state_error};
				if (epoch_index < 4000) {
					partial_network->backprop(errors, 0.01);
				} else {
					partial_network->backprop(errors, 0.001);
				}
				partial_loop_state_error = partial_network->input->errors[0];
				partial_network->input->errors[0] = 0.0;

				delete network_historys.back();
				network_historys.pop_back();
			}

			vector<double> errors{partial_loop_state_error};
			if (epoch_index < 4000) {
				partial_init_network->backprop(errors, 0.01);
			} else {
				partial_init_network->backprop(errors, 0.001);
			}
		}
	}

	cout << "Done" << endl;
}
