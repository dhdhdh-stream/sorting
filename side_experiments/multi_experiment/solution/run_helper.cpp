#include "run_helper.h"

#include "abstract_experiment.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_actions = 0;

	this->num_experiments_seen = 0;

	this->early_exit = false;
}

RunHelper::~RunHelper() {
	for (int h_index = 0; h_index < (int)this->experiment_histories.size(); h_index++) {
		delete this->experiment_histories[h_index];
	}
}
