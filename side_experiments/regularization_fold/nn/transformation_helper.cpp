#include "transformation_helper.h"

using namespace std;

const double TRANSFORMATION_TARGET_MAX_UPDATE = 0.005;

TransformationHelper::TransformationHelper() {
	this->transformation = Transformation();

	this->scale_update = 0.0;
	this->weight_update = 0.0;
	this->epoch_iter = 0;
	this->average_scale_update_size = 0.0;
	this->average_weight_update_size = 0.0;
}

void TransformationHelper::backprop(double val_in,
									double target) {
	double val_out = this->transformation.scale*val_in + this->transformation.weight;
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

			this->transformation.scale += this->scale_update*learning_rate;
			this->scale_update = 0.0;
		}

		double weight_update_size = abs(this->weight_update);
		this->average_weight_update_size = 0.999*this->average_weight_update_size + 0.001*weight_update_size;
		if (this->weight_update != 0.0) {
			double learning_rate = (0.3*TRANSFORMATION_TARGET_MAX_UPDATE)/this->average_weight_update_size;
			if (learning_rate*weight_update_size > TRANSFORMATION_TARGET_MAX_UPDATE) {
				learning_rate = TRANSFORMATION_TARGET_MAX_UPDATE/weight_update_size;
			}

			this->transformation.weight += this->weight_update*learning_rate;
			this->weight_update = 0.0;
		}

		this->epoch_iter = 0;
	}
}
