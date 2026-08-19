#include "predict_network.h"

#include "constants.h"
#include "globals.h"

using namespace std;

const int DENOISE_NUM_LAYERS = 4;
const vector<double> NOISE_MULTIPLIERS{
	0.05, 0.2, 0.35, 0.5
};

PredictNetwork::PredictNetwork(int num_states) {
	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->dist_hidden_1 = new Layer(LEAKY_LAYER);
	this->dist_hidden_1->acti_vals.resize(16);
	this->dist_hidden_1->errors.resize(16);
	this->dist_hidden_1->errors.setConstant(0.0);
	this->dist_hidden_1->input_layers.push_back(this->state_input);
	this->dist_hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->dist_hidden_2 = new Layer(LEAKY_LAYER);
	this->dist_hidden_2->acti_vals.resize(8);
	this->dist_hidden_2->errors.resize(8);
	this->dist_hidden_2->errors.setConstant(0.0);
	this->dist_hidden_2->input_layers.push_back(this->state_input);
	this->dist_hidden_2->input_layers.push_back(this->dist_hidden_1);
	this->dist_hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->mean_output = new Layer(LINEAR_LAYER);
	this->mean_output->acti_vals.resize(num_states);
	this->mean_output->errors.resize(num_states);
	this->mean_output->errors.setConstant(0.0);
	this->mean_output->input_layers.push_back(this->dist_hidden_1);
	this->mean_output->input_layers.push_back(this->dist_hidden_2);
	this->mean_output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->deviation_output = new Layer(LINEAR_LAYER);
	this->deviation_output->acti_vals.resize(num_states);
	this->deviation_output->errors.resize(num_states);
	this->deviation_output->errors.setConstant(0.0);
	this->deviation_output->input_layers.push_back(this->dist_hidden_1);
	this->deviation_output->input_layers.push_back(this->dist_hidden_2);
	this->deviation_output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->seed_input = new Layer(LINEAR_LAYER);
	this->seed_input->acti_vals.resize(num_states);
	this->seed_input->errors.resize(num_states);
	this->seed_input->errors.setConstant(0.0);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		Layer* hidden_1 = new Layer(LEAKY_LAYER);
		hidden_1->acti_vals.resize(16);
		hidden_1->errors.resize(16);
		hidden_1->errors.setConstant(0.0);
		hidden_1->input_layers.push_back(this->state_input);
		hidden_1->input_layers.push_back(this->seed_input);
		hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_hidden_1s.push_back(hidden_1);

		Layer* hidden_2 = new Layer(LEAKY_LAYER);
		hidden_2->acti_vals.resize(8);
		hidden_2->errors.resize(8);
		hidden_2->errors.setConstant(0.0);
		hidden_2->input_layers.push_back(this->state_input);
		hidden_2->input_layers.push_back(this->seed_input);
		hidden_2->input_layers.push_back(hidden_1);
		hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_hidden_2s.push_back(hidden_2);

		Layer* output = new Layer(LINEAR_LAYER);
		output->acti_vals.resize(num_states);
		output->errors.resize(num_states);
		output->errors.setConstant(0.0);
		output->input_layers.push_back(hidden_1);
		output->input_layers.push_back(hidden_2);
		output->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_outputs.push_back(output);
	}

	this->epoch_iter = 0;
}

PredictNetwork::PredictNetwork(PredictNetwork* original) {
	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(original->state_input->acti_vals.size());
	this->state_input->errors.resize(original->state_input->errors.size());
	this->state_input->errors.setConstant(0.0);

	this->dist_hidden_1 = new Layer(LEAKY_LAYER);
	this->dist_hidden_1->acti_vals.resize(original->dist_hidden_1->acti_vals.size());
	this->dist_hidden_1->errors.resize(original->dist_hidden_1->errors.size());
	this->dist_hidden_1->errors.setConstant(0.0);
	this->dist_hidden_1->input_layers.push_back(this->state_input);
	this->dist_hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
	this->dist_hidden_1->copy_weights_from(original->dist_hidden_1);

	this->dist_hidden_2 = new Layer(LEAKY_LAYER);
	this->dist_hidden_2->acti_vals.resize(original->dist_hidden_2->acti_vals.size());
	this->dist_hidden_2->errors.resize(original->dist_hidden_2->errors.size());
	this->dist_hidden_2->errors.setConstant(0.0);
	this->dist_hidden_2->input_layers.push_back(this->state_input);
	this->dist_hidden_2->input_layers.push_back(this->dist_hidden_1);
	this->dist_hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
	this->dist_hidden_2->copy_weights_from(original->dist_hidden_2);

	this->mean_output = new Layer(LINEAR_LAYER);
	this->mean_output->acti_vals.resize(original->mean_output->acti_vals.size());
	this->mean_output->errors.resize(original->mean_output->errors.size());
	this->mean_output->errors.setConstant(0.0);
	this->mean_output->input_layers.push_back(this->dist_hidden_1);
	this->mean_output->input_layers.push_back(this->dist_hidden_2);
	this->mean_output->update_structure(NETWORK_INIT_MULTIPLIER);
	this->mean_output->copy_weights_from(original->mean_output);

	this->deviation_output = new Layer(LINEAR_LAYER);
	this->deviation_output->acti_vals.resize(original->deviation_output->acti_vals.size());
	this->deviation_output->errors.resize(original->deviation_output->errors.size());
	this->deviation_output->errors.setConstant(0.0);
	this->deviation_output->input_layers.push_back(this->dist_hidden_1);
	this->deviation_output->input_layers.push_back(this->dist_hidden_2);
	this->deviation_output->update_structure(NETWORK_INIT_MULTIPLIER);
	this->deviation_output->copy_weights_from(original->deviation_output);

	this->seed_input = new Layer(LINEAR_LAYER);
	this->seed_input->acti_vals.resize(original->seed_input->acti_vals.size());
	this->seed_input->errors.resize(original->seed_input->errors.size());
	this->seed_input->errors.setConstant(0.0);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		Layer* hidden_1 = new Layer(LEAKY_LAYER);
		hidden_1->acti_vals.resize(original->noise_hidden_1s[layer_index]->acti_vals.size());
		hidden_1->errors.resize(original->noise_hidden_1s[layer_index]->errors.size());
		hidden_1->errors.setConstant(0.0);
		hidden_1->input_layers.push_back(this->state_input);
		hidden_1->input_layers.push_back(this->seed_input);
		hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_hidden_1s.push_back(hidden_1);
		hidden_1->copy_weights_from(original->noise_hidden_1s[layer_index]);

		Layer* hidden_2 = new Layer(LEAKY_LAYER);
		hidden_2->acti_vals.resize(original->noise_hidden_2s[layer_index]->acti_vals.size());
		hidden_2->errors.resize(original->noise_hidden_2s[layer_index]->errors.size());
		hidden_2->errors.setConstant(0.0);
		hidden_2->input_layers.push_back(this->state_input);
		hidden_2->input_layers.push_back(this->seed_input);
		hidden_2->input_layers.push_back(hidden_1);
		hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_hidden_2s.push_back(hidden_2);
		hidden_2->copy_weights_from(original->noise_hidden_2s[layer_index]);

		Layer* output = new Layer(LINEAR_LAYER);
		output->acti_vals.resize(original->noise_outputs[layer_index]->acti_vals.size());
		output->errors.resize(original->noise_outputs[layer_index]->errors.size());
		output->errors.setConstant(0.0);
		output->input_layers.push_back(hidden_1);
		output->input_layers.push_back(hidden_2);
		output->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_outputs.push_back(output);
		output->copy_weights_from(original->noise_outputs[layer_index]);
	}

	this->epoch_iter = 0;
}

PredictNetwork::PredictNetwork(std::ifstream& input_file) {
	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->dist_hidden_1 = new Layer(LEAKY_LAYER);
	string dist_hidden_1_size_line;
	getline(input_file, dist_hidden_1_size_line);
	int dist_hidden_1_size = stoi(dist_hidden_1_size_line);
	this->dist_hidden_1->acti_vals.resize(dist_hidden_1_size);
	this->dist_hidden_1->errors.resize(dist_hidden_1_size);
	this->dist_hidden_1->errors.setConstant(0.0);
	this->dist_hidden_1->input_layers.push_back(this->state_input);
	this->dist_hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->dist_hidden_2 = new Layer(LEAKY_LAYER);
	string dist_hidden_2_size_line;
	getline(input_file, dist_hidden_2_size_line);
	int dist_hidden_2_size = stoi(dist_hidden_2_size_line);
	this->dist_hidden_2->acti_vals.resize(dist_hidden_2_size);
	this->dist_hidden_2->errors.resize(dist_hidden_2_size);
	this->dist_hidden_2->errors.setConstant(0.0);
	this->dist_hidden_2->input_layers.push_back(this->state_input);
	this->dist_hidden_2->input_layers.push_back(this->dist_hidden_1);
	this->dist_hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->mean_output = new Layer(LINEAR_LAYER);
	this->mean_output->acti_vals.resize(num_states);
	this->mean_output->errors.resize(num_states);
	this->mean_output->errors.setConstant(0.0);
	this->mean_output->input_layers.push_back(this->dist_hidden_1);
	this->mean_output->input_layers.push_back(this->dist_hidden_2);
	this->mean_output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->deviation_output = new Layer(LINEAR_LAYER);
	this->deviation_output->acti_vals.resize(num_states);
	this->deviation_output->errors.resize(num_states);
	this->deviation_output->errors.setConstant(0.0);
	this->deviation_output->input_layers.push_back(this->dist_hidden_1);
	this->deviation_output->input_layers.push_back(this->dist_hidden_2);
	this->deviation_output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->seed_input = new Layer(LINEAR_LAYER);
	this->seed_input->acti_vals.resize(num_states);
	this->seed_input->errors.resize(num_states);
	this->seed_input->errors.setConstant(0.0);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		Layer* hidden_1 = new Layer(LEAKY_LAYER);
		string hidden_1_size_line;
		getline(input_file, hidden_1_size_line);
		int hidden_1_size = stoi(hidden_1_size_line);
		hidden_1->acti_vals.resize(hidden_1_size);
		hidden_1->errors.resize(hidden_1_size);
		hidden_1->errors.setConstant(0.0);
		hidden_1->input_layers.push_back(this->state_input);
		hidden_1->input_layers.push_back(this->seed_input);
		hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_hidden_1s.push_back(hidden_1);

		Layer* hidden_2 = new Layer(LEAKY_LAYER);
		string hidden_2_size_line;
		getline(input_file, hidden_2_size_line);
		int hidden_2_size = stoi(hidden_2_size_line);
		hidden_2->acti_vals.resize(hidden_2_size);
		hidden_2->errors.resize(hidden_2_size);
		hidden_2->errors.setConstant(0.0);
		hidden_2->input_layers.push_back(this->state_input);
		hidden_2->input_layers.push_back(this->seed_input);
		hidden_2->input_layers.push_back(hidden_1);
		hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_hidden_2s.push_back(hidden_2);

		Layer* output = new Layer(LINEAR_LAYER);
		output->acti_vals.resize(num_states);
		output->errors.resize(num_states);
		output->errors.setConstant(0.0);
		output->input_layers.push_back(hidden_1);
		output->input_layers.push_back(hidden_2);
		output->update_structure(NETWORK_INIT_MULTIPLIER);
		this->noise_outputs.push_back(output);
	}

	this->dist_hidden_1->load_weights_from(input_file);
	this->dist_hidden_2->load_weights_from(input_file);
	this->mean_output->load_weights_from(input_file);
	this->deviation_output->load_weights_from(input_file);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		this->noise_hidden_1s[layer_index]->load_weights_from(input_file);
		this->noise_hidden_2s[layer_index]->load_weights_from(input_file);
		this->noise_outputs[layer_index]->load_weights_from(input_file);
	}

	this->epoch_iter = 0;
}

PredictNetwork::~PredictNetwork() {
	delete this->state_input;

	delete this->dist_hidden_1;
	delete this->dist_hidden_2;
	delete this->mean_output;
	delete this->deviation_output;

	delete this->seed_input;

	for (int l_index = 0; l_index < (int)this->noise_hidden_1s.size(); l_index++) {
		delete this->noise_hidden_1s[l_index];
	}

	for (int l_index = 0; l_index < (int)this->noise_hidden_2s.size(); l_index++) {
		delete this->noise_hidden_2s[l_index];
	}

	for (int l_index = 0; l_index < (int)this->noise_outputs.size(); l_index++) {
		delete this->noise_outputs[l_index];
	}
}

void PredictNetwork::activate(Eigen::VectorXf& state_vals) {
	this->state_input->acti_vals = state_vals;

	this->dist_hidden_1->activate();
	this->dist_hidden_2->activate();
	this->mean_output->activate();
	this->deviation_output->activate();

	normal_distribution<double> distribution(0, 1);
	for (int s_index = 0; s_index < this->seed_input->acti_vals.size(); s_index++) {
		this->seed_input->acti_vals(s_index) = this->mean_output->acti_vals(s_index)
			+ distribution(generator) * this->deviation_output->acti_vals(s_index);
	}

	for (int layer_index = DENOISE_NUM_LAYERS-1; layer_index >= 0; layer_index--) {
		this->noise_hidden_1s[layer_index]->activate();
		this->noise_hidden_2s[layer_index]->activate();
		this->noise_outputs[layer_index]->activate();

		this->seed_input->acti_vals -= this->noise_outputs[layer_index]->acti_vals;
	}

	state_vals += this->seed_input->acti_vals;
}

void PredictNetwork::backprop(Eigen::VectorXf& starting_state_vals,
							  Eigen::VectorXf& state_diff_vals) {
	this->state_input->acti_vals = starting_state_vals;

	this->dist_hidden_1->activate();
	this->dist_hidden_2->activate();
	this->mean_output->activate();
	this->deviation_output->activate();

	this->mean_output->errors = state_diff_vals - this->mean_output->acti_vals;
	this->deviation_output->errors = this->mean_output->errors.cwiseAbs() - this->deviation_output->acti_vals;

	this->mean_output->backprop();
	this->deviation_output->backprop();
	this->dist_hidden_2->backprop();
	this->dist_hidden_1->backprop();

	normal_distribution<double> distribution(0, 1);
	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		Eigen::VectorXf noise;
		noise.resize(this->seed_input->acti_vals.size());
		for (int s_index = 0; s_index < this->seed_input->acti_vals.size(); s_index++) {
			noise(s_index) = NOISE_MULTIPLIERS[layer_index] * this->deviation_output->acti_vals(s_index) * distribution(generator);
		}

		this->seed_input->acti_vals = state_diff_vals + noise;

		this->noise_hidden_1s[layer_index]->activate();
		this->noise_hidden_2s[layer_index]->activate();
		this->noise_outputs[layer_index]->activate();

		this->noise_outputs[layer_index]->errors = noise - this->noise_outputs[layer_index]->acti_vals;

		this->noise_outputs[layer_index]->backprop();
		this->noise_hidden_2s[layer_index]->backprop();
		this->noise_hidden_1s[layer_index]->backprop();
	}

	this->epoch_iter++;
	if (this->epoch_iter == UPDATE_EPOCH_SIZE) {
		this->dist_hidden_1->update(1, SCORE_LEARNING_RATE);
		this->dist_hidden_2->update(1, SCORE_LEARNING_RATE);
		this->mean_output->update(1, SCORE_LEARNING_RATE);
		this->deviation_output->update(1, SCORE_LEARNING_RATE);

		for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
			this->noise_outputs[layer_index]->update(1, SCORE_LEARNING_RATE);
			this->noise_hidden_2s[layer_index]->update(1, SCORE_LEARNING_RATE);
			this->noise_hidden_1s[layer_index]->update(1, SCORE_LEARNING_RATE);
		}

		this->epoch_iter = 0;
	}
}

void PredictNetwork::clear_momentum() {
	this->dist_hidden_1->clear_momentum();
	this->dist_hidden_2->clear_momentum();
	this->mean_output->clear_momentum();
	this->deviation_output->clear_momentum();

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		this->noise_hidden_1s[layer_index]->clear_momentum();
		this->noise_hidden_2s[layer_index]->clear_momentum();
		this->noise_outputs[layer_index]->clear_momentum();
	}

	this->epoch_iter = 0;
}

void PredictNetwork::add_states(int new_num_states) {
	this->state_input->acti_vals.resize(new_num_states);
	this->state_input->errors.resize(new_num_states);
	this->state_input->errors.setConstant(0.0);

	this->mean_output->acti_vals.resize(new_num_states);
	this->mean_output->errors.resize(new_num_states);
	this->mean_output->errors.setConstant(0.0);

	this->deviation_output->acti_vals.resize(new_num_states);
	this->deviation_output->errors.resize(new_num_states);
	this->deviation_output->errors.setConstant(0.0);

	this->seed_input->acti_vals.resize(new_num_states);
	this->seed_input->errors.resize(new_num_states);
	this->seed_input->errors.setConstant(0.0);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		this->noise_outputs[layer_index]->acti_vals.resize(new_num_states);
		this->noise_outputs[layer_index]->errors.resize(new_num_states);
		this->noise_outputs[layer_index]->errors.setConstant(0.0);
	}

	this->dist_hidden_1->update_structure(0.0);
	this->dist_hidden_2->update_structure(0.0);
	this->mean_output->update_structure(0.0);
	this->deviation_output->update_structure(0.0);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		this->noise_hidden_1s[layer_index]->update_structure(0.0);
		this->noise_hidden_2s[layer_index]->update_structure(0.0);
		this->noise_outputs[layer_index]->update_structure(0.0);
	}
}

void PredictNetwork::save(std::ofstream& output_file) {
	output_file << this->state_input->acti_vals.size() << endl;

	output_file << this->dist_hidden_1->acti_vals.size() << endl;
	output_file << this->dist_hidden_2->acti_vals.size() << endl;

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		output_file << this->noise_hidden_1s[layer_index]->acti_vals.size() << endl;
		output_file << this->noise_hidden_2s[layer_index]->acti_vals.size() << endl;
	}

	this->dist_hidden_1->save_weights(output_file);
	this->dist_hidden_2->save_weights(output_file);
	this->mean_output->save_weights(output_file);
	this->deviation_output->save_weights(output_file);

	for (int layer_index = 0; layer_index < DENOISE_NUM_LAYERS; layer_index++) {
		this->noise_hidden_1s[layer_index]->save_weights(output_file);
		this->noise_hidden_2s[layer_index]->save_weights(output_file);
		this->noise_outputs[layer_index]->save_weights(output_file);
	}
}
