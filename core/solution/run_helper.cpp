#include "run_helper.h"

#include "abstract_experiment.h"

using namespace std;

RunHelper::RunHelper() {
	this->curr_depth = 0;
	this->max_depth = 0;

	this->num_decisions = 0;
	this->num_actions = 0;

	this->exceeded_limit = false;

	this->num_actions_after_experiment_to_skip = 0;
	this->eval_experiment = false;
}

RunHelper::~RunHelper() {
	for (int h_index = 0; h_index < (int)this->experiment_histories.size(); h_index++) {
		delete this->experiment_histories[h_index];
	}
}
