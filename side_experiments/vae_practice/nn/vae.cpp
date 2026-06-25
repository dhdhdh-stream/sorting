#include "vae.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

VAE::VAE(int num_inputs,
		 int num_states,
		 int num_outputs) {
	this->encoder_input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->encoder_input->acti_vals.push_back(0.0);
		this->encoder_input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->encoder_input);
	this->hidden_1->update_structure();

	this->means = new Layer(LINEAR_LAYER);
	for (int s_index = 0; s_index < num_states; s_index++) {
		this->means->acti_vals.push_back(0.0);
		this->means->errors.push_back(0.0);
	}
	this->means->input_layers.push_back(this->hidden_1);
	this->means->update_structure();

	this->log_vars = new Layer(LINEAR_LAYER);
	for (int s_index = 0; s_index < num_states; s_index++) {
		this->log_vars->acti_vals.push_back(0.0);
		this->log_vars->errors.push_back(0.0);
	}
	this->log_vars->input_layers.push_back(this->hidden_1);
	this->log_vars->update_structure();

	this->rand_vals = vector<double>(num_states);

	this->decoder_input = new Layer(LINEAR_LAYER);
	for (int s_index = 0; s_index < num_states; s_index++) {
		this->decoder_input->acti_vals.push_back(0.0);
		this->decoder_input->errors.push_back(0.0);
	}

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->decoder_input);
	this->hidden_2->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	for (int o_index = 0; o_index < num_outputs; o_index++) {
		this->output->acti_vals.push_back(0.0);
		this->output->errors.push_back(0.0);
	}
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

VAE::~VAE() {

}

void VAE::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->encoder_input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->means->activate();
	this->log_vars->activate();

	normal_distribution<double> distribution(0, 1);
	for (int s_index = 0; s_index < (int)this->means->acti_vals.size(); s_index++) {
		this->rand_vals[s_index] = distribution(generator);

		double standard_deviation = exp(0.5 * this->log_vars->acti_vals[s_index]);
		this->decoder_input->acti_vals[s_index] = this->means->acti_vals[s_index]
			+ this->rand_vals[s_index] * standard_deviation;
	}
	this->hidden_2->activate();
	this->output->activate();
}

void VAE::backprop(vector<double>& errors,
				   double kl_factor) {
	for (int o_index = 0; o_index < (int)errors.size(); o_index++) {
		this->output->errors[o_index] = errors[o_index];
	}
	this->output->backprop();
	this->hidden_2->backprop();

	for (int s_index = 0; s_index < (int)this->means->acti_vals.size(); s_index++) {
		this->means->errors[s_index] += this->decoder_input->errors[s_index];
		this->means->errors[s_index] -= kl_factor * this->means->acti_vals[s_index];

		this->log_vars->errors[s_index] +=
			this->rand_vals[s_index]
			* 0.5 * exp(0.5 * this->log_vars->acti_vals[s_index])
			* this->decoder_input->errors[s_index];
		this->log_vars->errors[s_index] -=
			kl_factor * 0.5 * (exp(this->log_vars->acti_vals[s_index]) - 1.0);

		this->decoder_input->errors[s_index] = 0.0;
	}
	this->means->backprop();
	this->log_vars->backprop();
	this->hidden_1->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		this->hidden_1->get_max_update(max_update);
		this->means->get_max_update(max_update);
		this->log_vars->get_max_update(max_update);
		this->hidden_2->get_max_update(max_update);
		this->output->get_max_update(max_update);
		this->average_max_update = 0.999*this->average_max_update+0.001*max_update;
		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}
			this->hidden_1->update_weights(learning_rate);
			this->means->update_weights(learning_rate);
			this->log_vars->update_weights(learning_rate);
			this->hidden_2->update_weights(learning_rate);
			this->output->update_weights(learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void VAE::init_backprop(vector<double>& errors,
						double kl_factor,
						double& hidden_1_average_max_update,
						double& means_average_max_update,
						double& log_vars_average_max_update,
						double& hidden_2_average_max_update,
						double& output_average_max_update) {
	for (int o_index = 0; o_index < (int)errors.size(); o_index++) {
		this->output->errors[o_index] = errors[o_index];
	}
	this->output->backprop();
	this->hidden_2->backprop();

	for (int s_index = 0; s_index < (int)this->means->acti_vals.size(); s_index++) {
		this->means->errors[s_index] += this->decoder_input->errors[s_index];
		this->means->errors[s_index] -= kl_factor * this->means->acti_vals[s_index];

		this->log_vars->errors[s_index] +=
			this->rand_vals[s_index]
			* 0.5 * exp(0.5 * this->log_vars->acti_vals[s_index])
			* this->decoder_input->errors[s_index];
		this->log_vars->errors[s_index] -=
			kl_factor * 0.5 * (exp(this->log_vars->acti_vals[s_index]) - 1.0);

		this->decoder_input->errors[s_index] = 0.0;
	}
	this->means->backprop();
	this->log_vars->backprop();
	this->hidden_1->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == NETWORK_EPOCH_SIZE) {
		double hidden_1_max_update = 0.0;
		this->hidden_1->get_max_update(hidden_1_max_update);
		hidden_1_average_max_update = 0.999*hidden_1_average_max_update+0.001*hidden_1_max_update;
		if (hidden_1_max_update > 0.0) {
			double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_1_average_max_update;
			if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
			}
			this->hidden_1->update_weights(hidden_1_learning_rate);
		}

		double means_max_update = 0.0;
		this->means->get_max_update(means_max_update);
		means_average_max_update = 0.999*means_average_max_update+0.001*means_max_update;
		if (means_max_update > 0.0) {
			double means_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/means_average_max_update;
			if (means_learning_rate*means_max_update > NETWORK_TARGET_MAX_UPDATE) {
				means_learning_rate = NETWORK_TARGET_MAX_UPDATE/means_max_update;
			}
			this->means->update_weights(means_learning_rate);
		}

		double log_vars_max_update = 0.0;
		this->log_vars->get_max_update(log_vars_max_update);
		log_vars_average_max_update = 0.999*log_vars_average_max_update+0.001*log_vars_max_update;
		if (log_vars_max_update > 0.0) {
			double log_vars_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/log_vars_average_max_update;
			if (log_vars_learning_rate*log_vars_max_update > NETWORK_TARGET_MAX_UPDATE) {
				log_vars_learning_rate = NETWORK_TARGET_MAX_UPDATE/log_vars_max_update;
			}
			this->log_vars->update_weights(log_vars_learning_rate);
		}

		double hidden_2_max_update = 0.0;
		this->hidden_2->get_max_update(hidden_2_max_update);
		hidden_2_average_max_update = 0.999*hidden_2_average_max_update+0.001*hidden_2_max_update;
		if (hidden_2_max_update > 0.0) {
			double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_2_average_max_update;
			if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
			}
			this->hidden_2->update_weights(hidden_2_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		output_average_max_update = 0.999*output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/output_average_max_update;
			if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
				output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}
