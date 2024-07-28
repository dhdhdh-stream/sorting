#include "predicted_world.h"

#include "world_model.h"

using namespace std;

PredictedWorld::PredictedWorld() {
	this->possible_models.push_back(new WorldModel());
	this->possible_weights.push_back(1.0);
}

PredictedWorld::~PredictedWorld() {
	for (int p_index = 0; p_index < (int)this->possible_models.size(); p_index++) {
		delete this->possible_models[p_index];
	}
}
