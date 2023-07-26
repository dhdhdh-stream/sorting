#include "transformation.h"

#include <iostream>

using namespace std;

const double TRANSFORMATION_TARGET_MAX_UPDATE = 0.005;

Transformation::Transformation() {
	this->scale = 0.0;
	this->weight = 0.0;

	this->scale_update = 0.0;
	this->weight_update = 0.0;
	this->epoch_iter = 0;
	this->average_scale_update_size = 0.0;
	this->average_weight_update_size = 0.0;
}

Transformation::Transformation(Transformation& reverse) {
	this->scale = 1.0/reverse.scale;
	this->weight = -reverse.weight/reverse.scale;
}

Transformation::Transformation(ifstream& save_file) {
	string scale_line;
	getline(input_file, scale_line);
	this->scale = stod(scale_line);

	string weight_line;
	getline(input_file, weight_line);
	this->weight = stod(weight_line);
}

void Transformation::backprop(double val_in,
							  double target) {
	double val_out = this->scale*val_in + this->weight;
	double error = target - val_out;
	this->scale_update += val_in*error;
	this->weight_update += error;

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double scale_update_size = abs(this->scale_update);
		this->average_scale_update_size = 0.999*this->average_scale_update_size + 0.001*scale_update_size;
		if (this->scale_update != 0.0) {
			double learning_rate = (0.3*TRANSFORMATION_TARGET_MAX_UPDATE)/this->average_scale_update_size;
			if (learning_rate*scale_update_size > TRANSFORMATION_TARGET_MAX_UPDATE) {
				learning_rate = TRANSFORMATION_TARGET_MAX_UPDATE/scale_update_size;
			}

			this->scale += this->scale_update*learning_rate;
			this->scale_update = 0.0;
		}

		double weight_update_size = abs(this->weight_update);
		this->average_weight_update_size = 0.999*this->average_weight_update_size + 0.001*weight_update_size;
		if (this->weight_update != 0.0) {
			double learning_rate = (0.3*TRANSFORMATION_TARGET_MAX_UPDATE)/this->average_weight_update_size;
			if (learning_rate*weight_update_size > TRANSFORMATION_TARGET_MAX_UPDATE) {
				learning_rate = TRANSFORMATION_TARGET_MAX_UPDATE/weight_update_size;
			}

			this->weight += this->weight_update*learning_rate;
			this->weight_update = 0.0;
		}

		this->epoch_iter = 0;
	}
}

double Transformation::forward(double val_in) {
	return this->scale*val_in + this->weight;
}

double Transformation::backward(double val_in) {
	return (val_in-this->weight)/this->scale;
}

double Transformation::backprop_backward(double error_in) {
	return this->scale*error_in;
}

double Transformation::backprop_forward(double error_in) {
	return error_in/this->scale;
}

void Transformation::save(ofstream& save_file) {
	save_file << this->scale << endl;
	save_file << this->weight < endl;
}
