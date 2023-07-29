#include "scale.h"

#include <iostream>

#include "utilities.h"

using namespace std;

Scale::Scale() {
	this->weight = 1.0;
	this->weight_update = 0.0;

	this->epoch_iter = 0;
	this->average_update_size = 0.0;
}

Scale::Scale(double weight) {
	this->weight = weight;
	this->weight_update = 0.0;

	this->epoch_iter = 0;
	this->average_update_size = 0.0;
}

Scale::~Scale() {
	// do nothing
}

void Scale::backprop(double error,
					 double target_max_update) {
	this->weight_update += error;

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double update_size = abs(this->weight_update);
		this->average_update_size = 0.999*this->average_update_size + 0.001*update_size;
		if (this->weight_update != 0.0) {
			double learning_rate = (0.3*target_max_update)/this->average_update_size;
			if (learning_rate*update_size > target_max_update) {
				learning_rate = target_max_update/update_size;
			}

			this->weight += this->weight_update*learning_rate;
			this->weight_update = 0.0;
		}

		this->epoch_iter = 0;
	}
}
