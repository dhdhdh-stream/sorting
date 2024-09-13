#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

RunHelper::RunHelper() {
	this->world_model = NULL;

	this->exceeded_limit = false;

	this->num_analyze = 0;
	this->num_actions = 0;
}

RunHelper::~RunHelper() {
	if (this->world_model != NULL) {
		delete this->world_model;
	}

	for (int h_index = 0; h_index < (int)this->experiment_histories.size(); h_index++) {
		delete this->experiment_histories[h_index];
	}
}
