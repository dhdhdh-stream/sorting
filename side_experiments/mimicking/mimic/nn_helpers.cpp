#include "nn_helpers.h"

#include <iostream>

#include "action_network.h"
#include "constants.h"
#include "globals.h"
#include "lstm.h"
#include "sample.h"

using namespace std;

const int TRAIN_ITERS = 1000000;

void train_network(vector<Sample*>& samples,
				   vector<LSTM*>& memory_cells,
				   vector<ActionNetwork*>& action_networks) {
	double sum_errors = 0.0;

	uniform_int_distribution<int> sample_distribution(0, samples.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 100 == 0) {
			cout << iter_index << endl;
		}

		int sample_index = sample_distribution(generator);

		vector<double> state_vals(NUM_STATES, 0.0);

		vector<vector<LSTMHistory*>> memory_cells_histories;
		vector<vector<ActionNetworkHistory*>> action_network_histories;

		{
			vector<LSTMHistory*> curr_memory_cells_histories(NUM_STATES);
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				curr_memory_cells_histories[s_index] = new LSTMHistory();
				memory_cells[s_index]->activate(samples[sample_index]->obs[0],
												0,
												state_vals,
												curr_memory_cells_histories[s_index]);
			}
			memory_cells_histories.push_back(curr_memory_cells_histories);

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_vals[s_index] = memory_cells[s_index]->memory_val;
			}

			vector<double> output_state_vals(NUM_STATES);
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				output_state_vals[s_index] = memory_cells[s_index]->output;
			}

			vector<ActionNetworkHistory*> curr_action_network_histories(action_networks.size());
			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				curr_action_network_histories[a_index] = new ActionNetworkHistory();
				action_networks[a_index]->activate(output_state_vals,
												   curr_action_network_histories[a_index]);
			}
			action_network_histories.push_back(curr_action_network_histories);

			if (iter_index % 100 == 0) {
				double sum_vals = 0.0;
				for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
					sum_vals += action_networks[a_index]->output->acti_vals[0];
				}
				cout << samples[sample_index]->actions[1].move << ": " << action_networks[
					samples[sample_index]->actions[1].move + 1]->output->acti_vals[0] << " / " << sum_vals << endl;
			}

			sum_errors += (1.0 - action_networks[
				samples[sample_index]->actions[1].move + 1]->output->acti_vals[0]);
		}
		for (int step_index = 1; step_index < (int)samples[sample_index]->actions.size()-1; step_index++) {
			vector<LSTMHistory*> curr_memory_cells_histories(NUM_STATES);
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				curr_memory_cells_histories[s_index] = new LSTMHistory();
				memory_cells[s_index]->activate(samples[sample_index]->obs[step_index],
												samples[sample_index]->actions[step_index].move + 1,
												state_vals,
												curr_memory_cells_histories[s_index]);
			}
			memory_cells_histories.push_back(curr_memory_cells_histories);

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_vals[s_index] = memory_cells[s_index]->memory_val;
			}

			vector<double> output_state_vals(NUM_STATES);
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				output_state_vals[s_index] = memory_cells[s_index]->output;
			}

			vector<ActionNetworkHistory*> curr_action_network_histories(action_networks.size());
			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				curr_action_network_histories[a_index] = new ActionNetworkHistory();
				action_networks[a_index]->activate(output_state_vals,
												   curr_action_network_histories[a_index]);
			}
			action_network_histories.push_back(curr_action_network_histories);

			if (iter_index % 100 == 0) {
				double sum_vals = 0.0;
				for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
					sum_vals += action_networks[a_index]->output->acti_vals[0];
				}
				cout << samples[sample_index]->actions[step_index+1].move << ": " << action_networks[
					samples[sample_index]->actions[step_index+1].move + 1]->output->acti_vals[0] << " / " << sum_vals << endl;
			}

			sum_errors += (1.0 - action_networks[
				samples[sample_index]->actions[step_index+1].move + 1]->output->acti_vals[0]);
		}
		{
			vector<LSTMHistory*> curr_memory_cells_histories(NUM_STATES);
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				curr_memory_cells_histories[s_index] = new LSTMHistory();
				memory_cells[s_index]->activate(samples[sample_index]->obs.back(),
												samples[sample_index]->actions.back().move + 1,
												state_vals,
												curr_memory_cells_histories[s_index]);
			}
			memory_cells_histories.push_back(curr_memory_cells_histories);

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_vals[s_index] = memory_cells[s_index]->memory_val;
			}

			vector<double> output_state_vals(NUM_STATES);
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				output_state_vals[s_index] = memory_cells[s_index]->output;
			}

			vector<ActionNetworkHistory*> curr_action_network_histories(action_networks.size());
			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				curr_action_network_histories[a_index] = new ActionNetworkHistory();
				action_networks[a_index]->activate(output_state_vals,
												   curr_action_network_histories[a_index]);
			}
			action_network_histories.push_back(curr_action_network_histories);

			if (iter_index % 100 == 0) {
				double sum_vals = 0.0;
				for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
					sum_vals += action_networks[a_index]->output->acti_vals[0];
				}
				cout << "-1: " << action_networks[0]->output->acti_vals[0] << " / " << sum_vals << endl;
			}

			sum_errors += (1.0 - action_networks[0]->output->acti_vals[0]);
		}

		vector<double> state_errors(NUM_STATES, 0.0);

		{
			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				if (a_index == 0) {
					action_networks[a_index]->backprop(1.0,
													   state_errors,
													   action_network_histories.back()[a_index]);
				} else {
					action_networks[a_index]->backprop(0.0,
													   state_errors,
													   action_network_histories.back()[a_index]);
				}
				delete action_network_histories.back()[a_index];
			}

			vector<double> next_state_errors(NUM_STATES, 0.0);

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				memory_cells[s_index]->backprop(state_errors[s_index],
												next_state_errors,
												memory_cells_histories.back()[s_index]);
				delete memory_cells_histories.back()[s_index];
			}

			state_errors = next_state_errors;
		}
		for (int step_index = (int)samples[sample_index]->actions.size()-2; step_index >= 0; step_index--) {
			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				if (a_index == samples[sample_index]->actions[step_index+1].move + 1) {
					action_networks[a_index]->backprop(1.0,
													   state_errors,
													   action_network_histories[step_index][a_index]);
				} else {
					action_networks[a_index]->backprop(0.0,
													   state_errors,
													   action_network_histories[step_index][a_index]);
				}
				delete action_network_histories[step_index][a_index];
			}

			vector<double> next_state_errors(NUM_STATES, 0.0);

			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				memory_cells[s_index]->backprop(state_errors[s_index],
												next_state_errors,
												memory_cells_histories[step_index][s_index]);
				delete memory_cells_histories[step_index][s_index];
			}

			state_errors = next_state_errors;
		}

		if (iter_index %10 == 0) {
			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				action_networks[a_index]->update_weights();
			}
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				memory_cells[s_index]->update_weights();
			}
		}

		if (iter_index % 100 == 0) {
			cout << "sum_errors: " << sum_errors << endl;
			sum_errors = 0.0;

			cout << endl;
		}

		if ((iter_index+1) % 10000 == 0) {
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				ofstream memory_cell_save_file;
				memory_cell_save_file.open("saves/memory_cell_" + to_string(s_index) + ".txt");
				memory_cells[s_index]->save(memory_cell_save_file);
				memory_cell_save_file.close();
			}

			for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
				ofstream action_network_save_file;
				action_network_save_file.open("saves/action_network_" + to_string(a_index) + ".txt");
				action_networks[a_index]->save(action_network_save_file);
				action_network_save_file.close();
			}
		}
	}
}
