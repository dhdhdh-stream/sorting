#include "explore_weight.h"

#include <iostream>

using namespace std;

ExploreWeight::ExploreWeight() {
	this->weight = randunit()*0.01;
	this->weight_update = 0.0;

	this->epoch_iter = 0;
	this->average_update_size = 0.0;
}

ExploreWeight::ExploreWeight(double weight) {
	this->weight = weight;
	this->weight_update = 0.0;

	this->epoch_iter = 0;
	this->average_update_size = 0.0;
}

ExploreWeight::~ExploreWeight() {
	// do nothing
}

void ExploreWeight::backprop(double error,
							 double target_max_update) {
	this->weight_update += error;

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double update_size = abs(this->weight_update);
		this->average_update_size = 0.999*this->average_update_size + 0.001*update_size;
		if (this->weight_update != 0.0) {
			double learning_rate = (0.3*target_max_update)/this->average_update;
			if (learning_rate*update_size > target_max_update) {
				learning_rate = target_max_update/update_size;
			}

			this->weight += this->weight_update*learning_rate;
			if (this->weight < 0.0) {
				this->weight = 0.0;
			}
			this->weight_update = 0.0;
		}

		this->epoch_iter = 0;
	}
}
