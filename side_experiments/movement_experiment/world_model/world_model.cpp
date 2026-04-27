#include "world_model.h"

#include <iostream>

#include "globals.h"
#include "network.h"

using namespace std;

const int WORLD_SIZE = 10;

const double WEIGHT_DISCOUNT = 0.7;

WorldModel::WorldModel() {
	this->location_networks = vector<Network*>(5);
	for (int i_index = 0; i_index < 5; i_index++) {
		this->location_networks[i_index] = new Network(12);
	}

	this->state_networks = vector<Network*>(5);
	for (int i_index = 0; i_index < 5; i_index++) {
		this->state_networks[i_index] = new Network(10);
	}
}

WorldModelInstance::WorldModelInstance(WorldModel* world_model) {
	this->world_model = world_model;

	this->states = vector<double>(WORLD_SIZE, 4.5);
	this->location = 0;
}

void WorldModelInstance::train_init(vector<double>& obs) {
	vector<double> state_input;
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		state_input.push_back(this->states[index]);
	}
	state_input.insert(state_input.end(), obs.begin(), obs.end());

	for (int i_index = 0; i_index < 5; i_index++) {
		Network* network = this->world_model->state_networks[i_index];
		network->activate(state_input);

		int index = this->location + i_index-2;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		this->states[index] = network->output->acti_vals[0];
	}

	this->state_input_histories.push_back(state_input);
	this->state_location_history.push_back(this->location);

	// vector<double> predict_identity_input;
	// for (int i_index = -2; i_index <= 2; i_index++) {
	// 	int index = this->location + i_index;
	// 	if (index < 0) {
	// 		index += WORLD_SIZE;
	// 	}
	// 	if (index >= WORLD_SIZE) {
	// 		index -= WORLD_SIZE;
	// 	}
	// 	predict_identity_input.push_back(this->states[index]);
	// }

	// this->predict_identity_target_histories.push_back(obs);

	// this->predict_identity_input_histories.push_back(predict_identity_input);

	// this->predict_identity_location_history.push_back(this->location);
}

void WorldModelInstance::train_step(int action,
									vector<double>& obs,
									bool add_uncertainty) {
	this->action_history.push_back(action);

	vector<double> location_input;
	for (int a_index = 0; a_index < 2; a_index++) {
		if (action == a_index) {
			location_input.push_back(1.0);
		} else {
			location_input.push_back(0.0);
		}
	}
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		location_input.push_back(this->states[index]);
	}
	location_input.insert(location_input.end(), obs.begin(), obs.end());
	this->location_input_histories.push_back(location_input);

	/**
	 * - same issue as damage in solution
	 *   - cannot make representative decisions with damage
	 *   - but cannot experiment without damage
	 */
	uniform_int_distribution<int> uncertainty_distribution(0, 9);
	if (add_uncertainty
			&& uncertainty_distribution(generator) == 0) {
		uniform_int_distribution<int> location_distribution(0, 4);
		int random_location = location_distribution(generator);
		this->location += random_location-2;

		this->rel_location_history.push_back(random_location);
	} else {
		double min_misguess;
		int best_location;
		{
			Network* network = this->world_model->location_networks[0];
			network->activate(location_input);
			min_misguess = network->output->acti_vals[0];
			best_location = 0;
		}
		for (int i_index = 1; i_index < 5; i_index++) {
			Network* network = this->world_model->location_networks[i_index];
			network->activate(location_input);
			if (network->output->acti_vals[0] < min_misguess) {
				min_misguess = network->output->acti_vals[0];
				best_location = i_index;
			}
		}
		this->location += best_location-2;

		this->rel_location_history.push_back(best_location);
	}
	if (this->location < 0) {
		this->location += WORLD_SIZE;
	}
	if (this->location >= WORLD_SIZE) {
		this->location -= WORLD_SIZE;
	}

	vector<double> predict_input;
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		predict_input.push_back(this->states[index]);
	}

	this->predict_target_histories.push_back(obs);

	this->predict_input_histories.push_back(predict_input);

	this->predict_location_history.push_back(this->location);

	vector<double> state_input;
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		state_input.push_back(this->states[index]);
	}
	state_input.insert(state_input.end(), obs.begin(), obs.end());

	for (int i_index = 0; i_index < 5; i_index++) {
		Network* network = this->world_model->state_networks[i_index];
		network->activate(state_input);

		int index = this->location + i_index-2;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		this->states[index] = network->output->acti_vals[0];
	}

	this->state_input_histories.push_back(state_input);
	this->state_location_history.push_back(this->location);

	// vector<double> predict_identity_input;
	// for (int i_index = -2; i_index <= 2; i_index++) {
	// 	int index = this->location + i_index;
	// 	if (index < 0) {
	// 		index += WORLD_SIZE;
	// 	}
	// 	if (index >= WORLD_SIZE) {
	// 		index -= WORLD_SIZE;
	// 	}
	// 	predict_identity_input.push_back(this->states[index]);
	// }

	// this->predict_identity_target_histories.push_back(obs);

	// this->predict_identity_input_histories.push_back(predict_identity_input);

	// this->predict_identity_location_history.push_back(this->location);
}

void WorldModelInstance::train_backprop() {
	vector<double> errors(WORLD_SIZE, 4.5);

	{
		// for (int i_index = 0; i_index < 5; i_index++) {
		// 	double predicted = this->predict_identity_input_histories.back()[i_index];

		// 	double error = this->predict_identity_target_histories.back()[i_index] - predicted;

		// 	int index = this->predict_identity_location_history.back() + i_index-2;
		// 	if (index < 0) {
		// 		index += WORLD_SIZE;
		// 	}
		// 	if (index >= WORLD_SIZE) {
		// 		index -= WORLD_SIZE;
		// 	}

		// 	errors[index] += error;
		// }

		// this->predict_identity_target_histories.pop_back();
		// this->predict_identity_input_histories.pop_back();
		// this->predict_identity_location_history.pop_back();

		// for (int i_index = 0; i_index < 5; i_index++) {
		// 	int index = this->state_location_history.back() + i_index-2;
		// 	if (index < 0) {
		// 		index += WORLD_SIZE;
		// 	}
		// 	if (index >= WORLD_SIZE) {
		// 		index -= WORLD_SIZE;
		// 	}

		// 	Network* network = this->world_model->state_networks[i_index];
		// 	network->activate(this->state_input_histories.back());

		// 	double error = errors[index];
		// 	errors[index] = 0.0;

		// 	network->backprop(error);

		// 	for (int ii_index = 0; ii_index < 5; ii_index++) {
		// 		int index = this->state_location_history.back() + ii_index-2;
		// 		if (index < 0) {
		// 			index += WORLD_SIZE;
		// 		}
		// 		if (index >= WORLD_SIZE) {
		// 			index -= WORLD_SIZE;
		// 		}

		// 		errors[index] += network->input->errors[ii_index];
		// 		network->input->errors[ii_index] = 0.0;
		// 	}
		// }

		this->state_input_histories.pop_back();
		this->state_location_history.pop_back();
	}

	double curr_weighted_misguess = 0.0;
	while (true) {
		double curr_sum_misguess = 0.0;
		for (int i_index = 0; i_index < 5; i_index++) {
			double predicted = this->predict_input_histories.back()[i_index];

			curr_sum_misguess += (this->predict_target_histories.back()[i_index] - predicted)
				* (this->predict_target_histories.back()[i_index] - predicted);
			double error = this->predict_target_histories.back()[i_index] - predicted;

			int index = this->predict_location_history.back() + i_index-2;
			if (index < 0) {
				index += WORLD_SIZE;
			}
			if (index >= WORLD_SIZE) {
				index -= WORLD_SIZE;
			}

			errors[index] += error;
		}

		// curr_weighted_misguess = (1.0 - WEIGHT_DISCOUNT) * curr_sum_misguess + WEIGHT_DISCOUNT * curr_weighted_misguess;
		curr_weighted_misguess = curr_sum_misguess;

		this->predict_target_histories.pop_back();
		this->predict_input_histories.pop_back();
		this->predict_location_history.pop_back();

		{
			Network* network = this->world_model->location_networks[this->rel_location_history.back()];
			network->activate(this->location_input_histories.back());

			double error = curr_weighted_misguess - network->output->acti_vals[0];
			network->backprop(error);
		}

		this->action_history.pop_back();
		this->location_input_histories.pop_back();
		this->rel_location_history.pop_back();

		// for (int i_index = 0; i_index < 5; i_index++) {
		// 	double predicted = this->predict_identity_input_histories.back()[i_index];

		// 	double error = this->predict_identity_target_histories.back()[i_index] - predicted;

		// 	int index = this->predict_identity_location_history.back() + i_index-2;
		// 	if (index < 0) {
		// 		index += WORLD_SIZE;
		// 	}
		// 	if (index >= WORLD_SIZE) {
		// 		index -= WORLD_SIZE;
		// 	}

		// 	errors[index] += error;
		// }

		// this->predict_identity_target_histories.pop_back();
		// this->predict_identity_input_histories.pop_back();
		// this->predict_identity_location_history.pop_back();

		for (int i_index = 0; i_index < 5; i_index++) {
			int index = this->state_location_history.back() + i_index-2;
			if (index < 0) {
				index += WORLD_SIZE;
			}
			if (index >= WORLD_SIZE) {
				index -= WORLD_SIZE;
			}

			Network* network = this->world_model->state_networks[i_index];
			network->activate(this->state_input_histories.back());

			double error = errors[index];
			errors[index] = 0.0;

			network->backprop(error);

			for (int ii_index = 0; ii_index < 5; ii_index++) {
				int index = this->state_location_history.back() + ii_index-2;
				if (index < 0) {
					index += WORLD_SIZE;
				}
				if (index >= WORLD_SIZE) {
					index -= WORLD_SIZE;
				}

				errors[index] += network->input->errors[ii_index];
				network->input->errors[ii_index] = 0.0;
			}
		}

		this->state_input_histories.pop_back();
		this->state_location_history.pop_back();
		if (this->state_input_histories.size() == 0) {
			break;
		}			
	}
}

void WorldModelInstance::debug_init(vector<double>& obs) {
	cout << "debug_init" << endl;

	vector<double> state_input;
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		state_input.push_back(this->states[index]);
	}
	state_input.insert(state_input.end(), obs.begin(), obs.end());

	for (int i_index = 0; i_index < 5; i_index++) {
		Network* network = this->world_model->state_networks[i_index];
		network->activate(state_input);

		int index = this->location + i_index-2;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		this->states[index] = network->output->acti_vals[0];
	}
}

void WorldModelInstance::debug_step(int action,
									vector<double>& obs) {
	vector<double> location_input;
	for (int a_index = 0; a_index < 2; a_index++) {
		if (action == a_index) {
			location_input.push_back(1.0);
		} else {
			location_input.push_back(0.0);
		}
	}
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		location_input.push_back(this->states[index]);
	}
	location_input.insert(location_input.end(), obs.begin(), obs.end());

	double min_misguess;
	int best_location;
	{
		Network* network = this->world_model->location_networks[0];
		network->activate(location_input);
		min_misguess = network->output->acti_vals[0];
		best_location = 0;

		cout << "0: " << network->output->acti_vals[0] << endl;
	}
	for (int i_index = 1; i_index < 5; i_index++) {
		Network* network = this->world_model->location_networks[i_index];
		network->activate(location_input);
		if (network->output->acti_vals[0] < min_misguess) {
			min_misguess = network->output->acti_vals[0];
			best_location = i_index;
		}

		cout << i_index << ": " << network->output->acti_vals[0] << endl;
	}
	this->location += best_location-2;

	cout << "action: " << action << endl;
	cout << "best_location: " << best_location << endl;

	if (this->location < 0) {
		this->location += WORLD_SIZE;
	}
	if (this->location >= WORLD_SIZE) {
		this->location -= WORLD_SIZE;
	}

	cout << "this->location: " << this->location << endl;

	cout << "predicted:";
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		cout << " " << this->states[index];
	}
	cout << endl;

	cout << "obs:";
	for (int i_index = 0; i_index < 5; i_index++) {
		cout << " " << obs[i_index];
	}
	cout << endl;

	vector<double> state_input;
	for (int i_index = -2; i_index <= 2; i_index++) {
		int index = this->location + i_index;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		state_input.push_back(this->states[index]);
	}
	state_input.insert(state_input.end(), obs.begin(), obs.end());

	for (int i_index = 0; i_index < 5; i_index++) {
		Network* network = this->world_model->state_networks[i_index];
		network->activate(state_input);

		int index = this->location + i_index-2;
		if (index < 0) {
			index += WORLD_SIZE;
		}
		if (index >= WORLD_SIZE) {
			index -= WORLD_SIZE;
		}
		this->states[index] = network->output->acti_vals[0];
	}

	cout << endl;
}
